/* Class with main redo log parser
   Copyright (C) 2018-2023 Adam Leszczynski (aleszczynski@bersler.com)

This file is part of OpenLogReplicator.

OpenLogReplicator is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 3, or (at your option)
any later version.

OpenLogReplicator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenLogReplicator; see the file LICENSE;  If not see
<http://www.gnu.org/licenses/>.  */

#include "../builder/Builder.h"
#include "../common/LobCtx.h"
#include "../common/OracleLob.h"
#include "../common/OracleTable.h"
#include "../common/RedoLogException.h"
#include "../common/Timer.h"
#include "../metadata/Metadata.h"
#include "../metadata/Schema.h"
#include "../reader/Reader.h"
#include "OpCode0501.h"
#include "OpCode0502.h"
#include "OpCode0504.h"
#include "OpCode0506.h"
#include "OpCode050B.h"
#include "OpCode0513.h"
#include "OpCode0514.h"
#include "OpCode0A02.h"
#include "OpCode0A08.h"
#include "OpCode0A12.h"
#include "OpCode0B02.h"
#include "OpCode0B03.h"
#include "OpCode0B04.h"
#include "OpCode0B05.h"
#include "OpCode0B06.h"
#include "OpCode0B08.h"
#include "OpCode0B0B.h"
#include "OpCode0B0C.h"
#include "OpCode0B10.h"
#include "OpCode0B16.h"
#include "OpCode1301.h"
#include "OpCode1801.h"
#include "OpCode1A02.h"
#include "OpCode1A06.h"
#include "Parser.h"
#include "Transaction.h"
#include "TransactionBuffer.h"

namespace OpenLogReplicator {
    Parser::Parser(Ctx* newCtx, Builder* newBuilder, Metadata* newMetadata, TransactionBuffer* newTransactionBuffer, int64_t newGroup,
                const std::string& newPath) :
            ctx(newCtx),
            builder(newBuilder),
            metadata(newMetadata),
            transactionBuffer(newTransactionBuffer),
            lastTransaction(nullptr),
            lwnAllocated(0),
            lwnAllocatedMax(0),
            lwnTimestamp(0),
            lwnScn(0),
            lwnCheckpointBlock(0),
            group(newGroup),
            path(newPath),
            sequence(0),
            firstScn(ZERO_SCN),
            nextScn(ZERO_SCN),
            reader(nullptr) {

        memset(reinterpret_cast<void*>(&zero), 0, sizeof(RedoLogRecord));

        lwnChunks[0] = ctx->getMemoryChunk("parser", false);
        auto length = reinterpret_cast<uint64_t*>(lwnChunks[0]);
        *length = sizeof(uint64_t);
        lwnAllocated = 1;
        lwnAllocatedMax = 1;
    }

    Parser::~Parser() {
        while (lwnAllocated > 0) {
            ctx->freeMemoryChunk("parser", lwnChunks[--lwnAllocated], false);
        }
    }

    void Parser::freeLwn() {
        while (lwnAllocated > 1) {
            ctx->freeMemoryChunk("parser", lwnChunks[--lwnAllocated], false);
        }

        auto length = reinterpret_cast<uint64_t*>(lwnChunks[0]);
        *length = sizeof(uint64_t);
    }

    void Parser::analyzeLwn(LwnMember* lwnMember) {
        if (ctx->trace & TRACE_LWN)
            ctx->logTrace(TRACE_LWN, "analyze blk: " + std::to_string(lwnMember->block) + " offset: " +
                          std::to_string(lwnMember->offset) + " scn: " + std::to_string(lwnMember->scn) + " subscn: " +
                          std::to_string(lwnMember->subScn));

        uint8_t* data = reinterpret_cast<uint8_t*>(lwnMember) + sizeof(struct LwnMember);
        RedoLogRecord redoLogRecord[2];
        int64_t vectorCur = -1;
        int64_t vectorPrev = -1;
        if (ctx->trace & TRACE_LWN)
            ctx->logTrace(TRACE_LWN, "analyze length: " + std::to_string(lwnMember->length) + " scn: " + std::to_string(lwnMember->scn) +
                          " subscn: " + std::to_string(lwnMember->subScn));

        uint32_t recordLength = ctx->read32(data);
        uint8_t vld = data[4];
        uint64_t headerLength;

        if (recordLength != lwnMember->length)
            throw RedoLogException(50046, "block: " + std::to_string(lwnMember->block) + ", offset: " +
                                   std::to_string(lwnMember->offset) + ": too small log record, buffer length: " + std::to_string(lwnMember->length) +
                                   ", field length: " + std::to_string(recordLength));

        if ((vld & 0x04) != 0)
            headerLength = 68;
        else
            headerLength = 24;

        if (ctx->dumpRedoLog >= 1) {
            uint16_t thread = 1; // TODO: verify field length/position
            ctx->dumpStream << " \n";

            if (ctx->version < REDO_VERSION_12_1)
                ctx->dumpStream << "REDO RECORD - Thread:" << thread << " RBA: 0x" << std::setfill('0') << std::setw(6) << std::hex << sequence << "." <<
                        std::setfill('0') << std::setw(8) << std::hex << lwnMember->block << "." << std::setfill('0') << std::setw(4) <<
                        std::hex << lwnMember->offset << " LEN: 0x" << std::setfill('0') << std::setw(4) << std::hex << recordLength << " VLD: 0x" <<
                        std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(vld) << '\n';
            else {
                uint32_t conUid = ctx->read32(data + 16);
                ctx->dumpStream << "REDO RECORD - Thread:" << thread << " RBA: 0x" << std::setfill('0') << std::setw(6) << std::hex << sequence <<
                        "." << std::setfill('0') << std::setw(8) << std::hex << lwnMember->block << "." << std::setfill('0') << std::setw(4) <<
                        std::hex << lwnMember->offset << " LEN: 0x" << std::setfill('0') << std::setw(4) << std::hex << recordLength << " VLD: 0x" <<
                        std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(vld) << " CON_UID: " << std::dec << conUid << '\n';
            }

            if (ctx->dumpRawData > 0) {
                std::string header = "## H: [" + std::to_string(lwnMember->block * reader->getBlockSize() + lwnMember->offset) + "] " +
                        std::to_string(headerLength);
                ctx->dumpStream << header;
                if (header.length() < 36)
                    ctx->dumpStream << std::string(36 - header.length(), ' ');

                for (uint64_t j = 0; j < headerLength; ++j)
                    ctx->dumpStream << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(data[j]) << " ";
                ctx->dumpStream << '\n';
            }

            if (headerLength == 68) {
                if (ctx->version < REDO_VERSION_12_2)
                    ctx->dumpStream << "SCN: " << PRINTSCN48(lwnMember->scn) << " SUBSCN:" << std::setfill(' ') << std::setw(3) << std::dec <<
                            lwnMember->subScn << " " << lwnTimestamp << '\n';
                else
                    ctx->dumpStream << "SCN: " << PRINTSCN64(lwnMember->scn) << " SUBSCN:" << std::setfill(' ') << std::setw(3) << std::dec <<
                            lwnMember->subScn << " " << lwnTimestamp << '\n';
                uint16_t lwnNst = ctx->read16(data + 26);
                uint32_t lwnLen = ctx->read32(data + 32);

                if (ctx->version < REDO_VERSION_12_2)
                    ctx->dumpStream << "(LWN RBA: 0x" << std::setfill('0') << std::setw(6) << std::hex << sequence << "." << std::setfill('0') <<
                            std::setw(8) << std::hex << lwnMember->block << "." << std::setfill('0') << std::setw(4) << std::hex <<
                            lwnMember->offset << " LEN: " << std::setfill('0') << std::setw(4) << std::dec << lwnLen << " NST: " <<
                            std::setfill('0') << std::setw(4) << std::dec << lwnNst << " SCN: " << PRINTSCN48(lwnScn) << ")" << '\n';
                else
                    ctx->dumpStream << "(LWN RBA: 0x" << std::setfill('0') << std::setw(6) << std::hex << sequence << "." << std::setfill('0') <<
                            std::setw(8) << std::hex << lwnMember->block << "." << std::setfill('0') << std::setw(4) << std::hex <<
                            lwnMember->offset << " LEN: 0x" << std::setfill('0') << std::setw(8) << std::hex << lwnLen << " NST: 0x" <<
                            std::setfill('0') << std::setw(4) << std::hex << lwnNst << " SCN: " << PRINTSCN64(lwnScn) << ")" << '\n';
            } else {
                if (ctx->version < REDO_VERSION_12_2)
                    ctx->dumpStream << "SCN: " << PRINTSCN48(lwnMember->scn) << " SUBSCN:" << std::setfill(' ') << std::setw(3) << std::dec <<
                            lwnMember->subScn << " " << lwnTimestamp << '\n';
                else
                    ctx->dumpStream << "SCN: " << PRINTSCN64(lwnMember->scn) << " SUBSCN:" << std::setfill(' ') << std::setw(3) << std::dec <<
                            lwnMember->subScn << " " << lwnTimestamp << '\n';
            }
        }

        if (headerLength > recordLength) {
            dumpRedoVector(data, recordLength);
            throw RedoLogException(50046, "block: " + std::to_string(lwnMember->block) + ", offset: " +
                                   std::to_string(lwnMember->offset) + ": too small log record, header length: " + std::to_string(headerLength) +
                                   ", field length: " + std::to_string(recordLength));
        }

        uint64_t offset = headerLength;
        uint64_t vectors = 0;

        while (offset < recordLength) {
            vectorPrev = vectorCur;
            if (vectorPrev == -1)
                vectorCur = 0;
            else
                vectorCur = 1 - vectorPrev;

            memset(reinterpret_cast<void*>(&redoLogRecord[vectorCur]), 0, sizeof(RedoLogRecord));
            redoLogRecord[vectorCur].vectorNo = (++vectors);
            redoLogRecord[vectorCur].cls = ctx->read16(data + offset + 2);
            redoLogRecord[vectorCur].afn = static_cast<typeAfn>(ctx->read32(data + offset + 4) & 0xFFFF);
            redoLogRecord[vectorCur].dba = ctx->read32(data + offset + 8);
            redoLogRecord[vectorCur].scnRecord = ctx->readScn(data + offset + 12);
            redoLogRecord[vectorCur].rbl = 0; // TODO: verify field length/position
            redoLogRecord[vectorCur].seq = data[offset + 20];
            redoLogRecord[vectorCur].typ = data[offset + 21];
            typeUsn usn = (redoLogRecord[vectorCur].cls >= 15) ? (redoLogRecord[vectorCur].cls - 15) / 2 : -1;

            uint64_t fieldOffset;
            if (ctx->version >= REDO_VERSION_12_1) {
                fieldOffset = 32;
                redoLogRecord[vectorCur].flgRecord = ctx->read16(data + offset + 28);
                redoLogRecord[vectorCur].conId = static_cast<typeConId>(ctx->read16(data + offset + 24));
            } else {
                fieldOffset = 24;
                redoLogRecord[vectorCur].flgRecord = 0;
                redoLogRecord[vectorCur].conId = 0;
            }

            if (offset + fieldOffset + 1 >= recordLength) {
                dumpRedoVector(data, recordLength);
                throw RedoLogException(50046, "block: " + std::to_string(lwnMember->block) + ", offset: " +
                                       std::to_string(lwnMember->offset) + ": position of field list (" + std::to_string(offset + fieldOffset + 1) +
                                       ") outside of record, length: " + std::to_string(recordLength));
            }

            uint8_t* fieldList = data + offset + fieldOffset;

            redoLogRecord[vectorCur].opCode = (static_cast<typeOp1>(data[offset + 0]) << 8) | data[offset + 1];
            redoLogRecord[vectorCur].length = fieldOffset + ((ctx->read16(fieldList) + 2) & 0xFFFC);
            redoLogRecord[vectorCur].sequence = sequence;
            redoLogRecord[vectorCur].scn = lwnMember->scn;
            redoLogRecord[vectorCur].subScn = lwnMember->subScn;
            redoLogRecord[vectorCur].usn = usn;
            redoLogRecord[vectorCur].data = data + offset;
            redoLogRecord[vectorCur].dataOffset = lwnMember->block * reader->getBlockSize() + lwnMember->offset + offset;
            redoLogRecord[vectorCur].fieldLengthsDelta = fieldOffset;
            if (redoLogRecord[vectorCur].fieldLengthsDelta + 1 >= recordLength) {
                dumpRedoVector(data, recordLength);
                throw RedoLogException(50046, "block: " + std::to_string(lwnMember->block) + ", offset: " +
                                       std::to_string(lwnMember->offset) + ": field length list (" +
                                       std::to_string(redoLogRecord[vectorCur].fieldLengthsDelta) +
                                       ") outside of record, length: " + std::to_string(recordLength));
            }
            redoLogRecord[vectorCur].fieldCnt = (ctx->read16(redoLogRecord[vectorCur].data + redoLogRecord[vectorCur].fieldLengthsDelta) - 2) / 2;
            redoLogRecord[vectorCur].fieldPos = fieldOffset +
                    ((ctx->read16(redoLogRecord[vectorCur].data + redoLogRecord[vectorCur].fieldLengthsDelta) + 2) & 0xFFFC);
            if (redoLogRecord[vectorCur].fieldPos >= recordLength) {
                dumpRedoVector(data, recordLength);
                throw RedoLogException(50046, "block: " + std::to_string(lwnMember->block) + ", offset: " +
                                       std::to_string(lwnMember->offset) + ": fields (" + std::to_string(redoLogRecord[vectorCur].fieldPos) +
                                       ") outside of record, length: " + std::to_string(recordLength));
            }

            // uint64_t fieldPos = redoLogRecord[vectorCur].fieldPos;
            for (uint64_t i = 1; i <= redoLogRecord[vectorCur].fieldCnt; ++i) {
                redoLogRecord[vectorCur].length += (ctx->read16(fieldList + i * 2) + 3) & 0xFFFC;

                if (offset + redoLogRecord[vectorCur].length > recordLength) {
                    dumpRedoVector(data, recordLength);
                    throw RedoLogException(50046, "block: " + std::to_string(lwnMember->block) + ", offset: " +
                                           std::to_string(lwnMember->offset) + ": position of field list outside of record (" + "i: " +
                                           std::to_string(i) + " c: " + std::to_string(redoLogRecord[vectorCur].fieldCnt) + " " + " o: " +
                                           std::to_string(fieldOffset) + " p: " + std::to_string(offset) + " l: " +
                                           std::to_string(redoLogRecord[vectorCur].length) + " r: " + std::to_string(recordLength) + ")");
                }
            }

            if (redoLogRecord[vectorCur].fieldPos > redoLogRecord[vectorCur].length) {
                dumpRedoVector(data, recordLength);
                throw RedoLogException(50046, "block: " + std::to_string(lwnMember->block) + ", offset: " +
                                       std::to_string(lwnMember->offset) + ": incomplete record, offset: " +
                                       std::to_string(redoLogRecord[vectorCur].fieldPos) + ", length: " +
                                       std::to_string(redoLogRecord[vectorCur].length));
            }

            redoLogRecord[vectorCur].recordObj = 0xFFFFFFFF;
            redoLogRecord[vectorCur].recordDataObj = 0xFFFFFFFF;
            offset += redoLogRecord[vectorCur].length;

            switch (redoLogRecord[vectorCur].opCode) {
                // Undo
                case 0x0501:
                    OpCode0501::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // Begin transaction
                case 0x0502:
                    OpCode0502::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // Commit/rollback transaction
                case 0x0504:
                    OpCode0504::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // Partial rollback
                case 0x0506:
                    OpCode0506::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                case 0x050B:
                    OpCode050B::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // Session information
                case 0x0513:
                    OpCode0513::process(ctx, &redoLogRecord[vectorCur], lastTransaction);
                    break;

                // Session information
                case 0x0514:
                    OpCode0514::process(ctx, &redoLogRecord[vectorCur], lastTransaction);
                    break;

                // REDO: Insert leaf row
                case 0x0A02:
                    if (vectorPrev != -1 && redoLogRecord[vectorPrev].opCode == 0x0501) {
                        redoLogRecord[vectorCur].recordDataObj = redoLogRecord[vectorPrev].dataObj;
                        redoLogRecord[vectorCur].recordObj = redoLogRecord[vectorPrev].obj;
                    }
                    OpCode0A02::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // REDO: Init header
                case 0x0A08:
                    if (vectorPrev != -1 && redoLogRecord[vectorPrev].opCode == 0x0501) {
                        redoLogRecord[vectorCur].recordDataObj = redoLogRecord[vectorPrev].dataObj;
                        redoLogRecord[vectorCur].recordObj = redoLogRecord[vectorPrev].obj;
                    }
                    OpCode0A08::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // REDO: Update key data in row
                case 0x0A12:
                    if (vectorPrev != -1 && redoLogRecord[vectorPrev].opCode == 0x0501) {
                        redoLogRecord[vectorCur].recordDataObj = redoLogRecord[vectorPrev].dataObj;
                        redoLogRecord[vectorCur].recordObj = redoLogRecord[vectorPrev].obj;
                    }
                    OpCode0A12::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // REDO: Insert row piece
                case 0x0B02:
                    if (vectorPrev != -1 && redoLogRecord[vectorPrev].opCode == 0x0501) {
                        redoLogRecord[vectorCur].recordDataObj = redoLogRecord[vectorPrev].dataObj;
                        redoLogRecord[vectorCur].recordObj = redoLogRecord[vectorPrev].obj;
                    }
                    OpCode0B02::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // REDO: Delete row piece
                case 0x0B03:
                    if (vectorPrev != -1 && redoLogRecord[vectorPrev].opCode == 0x0501) {
                        redoLogRecord[vectorCur].recordDataObj = redoLogRecord[vectorPrev].dataObj;
                        redoLogRecord[vectorCur].recordObj = redoLogRecord[vectorPrev].obj;
                    }
                    OpCode0B03::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // REDO: Lock row piece
                case 0x0B04:
                    if (vectorPrev != -1 && redoLogRecord[vectorPrev].opCode == 0x0501) {
                        redoLogRecord[vectorCur].recordDataObj = redoLogRecord[vectorPrev].dataObj;
                        redoLogRecord[vectorCur].recordObj = redoLogRecord[vectorPrev].obj;
                    }
                    OpCode0B04::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // REDO: Update row piece
                case 0x0B05:
                    if (vectorPrev != -1 && redoLogRecord[vectorPrev].opCode == 0x0501) {
                        redoLogRecord[vectorCur].recordDataObj = redoLogRecord[vectorPrev].dataObj;
                        redoLogRecord[vectorCur].recordObj = redoLogRecord[vectorPrev].obj;
                    }
                    OpCode0B05::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // REDO: Overwrite row piece
                case 0x0B06:
                    if (vectorPrev != -1 && redoLogRecord[vectorPrev].opCode == 0x0501) {
                        redoLogRecord[vectorCur].recordDataObj = redoLogRecord[vectorPrev].dataObj;
                        redoLogRecord[vectorCur].recordObj = redoLogRecord[vectorPrev].obj;
                    }
                    OpCode0B06::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // REDO: Change forwarding address
                case 0x0B08:
                    if (vectorPrev != -1 && redoLogRecord[vectorPrev].opCode == 0x0501) {
                        redoLogRecord[vectorCur].recordDataObj = redoLogRecord[vectorPrev].dataObj;
                        redoLogRecord[vectorCur].recordObj = redoLogRecord[vectorPrev].obj;
                    }
                    OpCode0B08::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // REDO: Insert multiple rows
                case 0x0B0B:
                    if (vectorPrev != -1 && redoLogRecord[vectorPrev].opCode == 0x0501) {
                        redoLogRecord[vectorCur].recordDataObj = redoLogRecord[vectorPrev].dataObj;
                        redoLogRecord[vectorCur].recordObj = redoLogRecord[vectorPrev].obj;
                    }
                    OpCode0B0B::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // REDO: Delete multiple rows
                case 0x0B0C:
                    if (vectorPrev != -1 && redoLogRecord[vectorPrev].opCode == 0x0501) {
                        redoLogRecord[vectorCur].recordDataObj = redoLogRecord[vectorPrev].dataObj;
                        redoLogRecord[vectorCur].recordObj = redoLogRecord[vectorPrev].obj;
                    }
                    OpCode0B0C::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // REDO: Supplemental log for update
                case 0x0B10:
                    if (vectorPrev != -1 && redoLogRecord[vectorPrev].opCode == 0x0501) {
                        redoLogRecord[vectorCur].recordDataObj = redoLogRecord[vectorPrev].dataObj;
                        redoLogRecord[vectorCur].recordObj = redoLogRecord[vectorPrev].obj;
                    }
                    OpCode0B10::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // REDO: Logminer support - KDOCMP
                case 0x0B16:
                    if (vectorPrev != -1 && redoLogRecord[vectorPrev].opCode == 0x0501) {
                        redoLogRecord[vectorCur].recordDataObj = redoLogRecord[vectorPrev].dataObj;
                        redoLogRecord[vectorCur].recordObj = redoLogRecord[vectorPrev].obj;
                    }
                    OpCode0B16::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // LOB
                case 0x1301:
                    OpCode1301::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // LOB index 12+ and LOB redo
                case 0x1A02:
                    if (vectorPrev != -1 && redoLogRecord[vectorPrev].opCode == 0x0501) {
                        redoLogRecord[vectorCur].recordDataObj = redoLogRecord[vectorPrev].dataObj;
                        redoLogRecord[vectorCur].recordObj = redoLogRecord[vectorPrev].obj;
                    }
                    OpCode1A02::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                case 0x1A06:
                    OpCode1A06::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                // DDL
                case 0x1801:
                    OpCode1801::process(ctx, &redoLogRecord[vectorCur]);
                    break;

                default:
                    OpCode::process(ctx, &redoLogRecord[vectorCur]);
                    break;
            }

            if (vectorPrev != -1) {
                if (redoLogRecord[vectorPrev].opCode == 0x0501) {
                    // UNDO - index
                    if ((redoLogRecord[vectorCur].opCode & 0xFF00) == 0x0A00 || redoLogRecord[vectorCur].opCode == 0x1A02)
                        appendToTransactionIndex(&redoLogRecord[vectorPrev], &redoLogRecord[vectorCur]);
                    // UNDO - data
                    else if ((redoLogRecord[vectorCur].opCode & 0xFF00) == 0x0B00 || redoLogRecord[vectorCur].opCode == 0x0513 ||
                            redoLogRecord[vectorCur].opCode == 0x0514)
                        appendToTransaction(&redoLogRecord[vectorPrev], &redoLogRecord[vectorCur]);
                    // Single 5.1
                    else if (redoLogRecord[vectorCur].opCode == 0x0501) {
                        appendToTransaction(&redoLogRecord[vectorPrev]);
                        continue;
                    } else if (redoLogRecord[vectorPrev].opc == 0x0B01)
                        ctx->warning(70010, "unknown undo OP: " + std::to_string(redoLogRecord[vectorCur].opCode) + ", opc: " +
                                     std::to_string(redoLogRecord[vectorPrev].opc));

                    vectorCur = -1;
                    continue;
                }

                if ((redoLogRecord[vectorCur].opCode == 0x0506 || redoLogRecord[vectorCur].opCode == 0x050B)) {
                    if ((redoLogRecord[vectorPrev].opCode & 0xFF00) == 0x0B00)
                        appendToTransactionRollback(&redoLogRecord[vectorPrev], &redoLogRecord[vectorCur]);
                    else if (redoLogRecord[vectorCur].opc == 0x0B01)
                        ctx->warning(70011, "unknown rollback OP: " + std::to_string(redoLogRecord[vectorPrev].opCode) + ", opc: " +
                                     std::to_string(redoLogRecord[vectorCur].opc));

                    vectorCur = -1;
                    continue;
                }
            }

            // UNDO - data
            if (redoLogRecord[vectorCur].opCode == 0x0501 && (redoLogRecord[vectorCur].flg & (FLG_MULTIBLOCKUNDOTAIL | FLG_MULTIBLOCKUNDOMID)) != 0) {
                appendToTransaction(&redoLogRecord[vectorCur]);
                vectorCur = -1;
                continue;
            }

            // ROLLBACK - data
            if (redoLogRecord[vectorCur].opCode == 0x0506 || redoLogRecord[vectorCur].opCode == 0x050B) {
                appendToTransactionRollback(&redoLogRecord[vectorCur]);
                vectorCur = -1;
                continue;
            }

            // BEGIN
            if (redoLogRecord[vectorCur].opCode == 0x0502) {
                appendToTransactionBegin(&redoLogRecord[vectorCur]);
                vectorCur = -1;
                continue;
            }

            // COMMIT
            if (redoLogRecord[vectorCur].opCode == 0x0504) {
                appendToTransactionCommit(&redoLogRecord[vectorCur]);
                vectorCur = -1;
                continue;
            }

            // LOB
            if (redoLogRecord[vectorCur].opCode == 0x1301 || redoLogRecord[vectorCur].opCode == 0x1A06) {
                appendToTransactionLob(&redoLogRecord[vectorCur]);
                vectorCur = -1;
                continue;
            }

            // DDL
            if (redoLogRecord[vectorCur].opCode == 0x1801) {
                appendToTransactionDdl(&redoLogRecord[vectorCur]);
                vectorCur = -1;
                continue;
            }
        }

        // UNDO - data
        if (vectorCur != -1 && redoLogRecord[vectorCur].opCode == 0x0501) {
            appendToTransaction(&redoLogRecord[vectorCur]);
        }
    }

    void Parser::appendToTransactionDdl(RedoLogRecord* redoLogRecord1) {
        // Track DDL
        if (!FLAG(REDO_FLAGS_SHOW_DDL))
            return;

        // Skip list
        if (transactionBuffer->skipXidList.find(redoLogRecord1->xid) != transactionBuffer->skipXidList.end())
            return;

        Transaction* transaction = transactionBuffer->findTransaction(redoLogRecord1->xid, redoLogRecord1->conId, true,
                                                                      FLAG(REDO_FLAGS_SHOW_INCOMPLETE_TRANSACTIONS), false);
        if (transaction == nullptr)
            return;
        lastTransaction = transaction;

        OracleTable* table = nullptr;
        {
            std::unique_lock<std::mutex> lckTransaction(metadata->mtxTransaction);
            table = metadata->schema->checkTableDict(redoLogRecord1->obj);
        }

        if (table == nullptr) {
            if (!FLAG(REDO_FLAGS_SCHEMALESS) && !FLAG(REDO_FLAGS_SHOW_DDL)) {
                transaction->log(ctx, "tbl ", redoLogRecord1);
                return;
            }
        } else {
            if ((table->options & OPTIONS_SYSTEM_TABLE) != 0)
                transaction->system = true;
            if ((table->options & OPTIONS_SCHEMA_TABLE) != 0)
                transaction->schema = true;
        }

        // Transaction size limit
        if (ctx->transactionSizeMax > 0 &&
            transaction->size + redoLogRecord1->length + ROW_HEADER_TOTAL >= ctx->transactionSizeMax) {
            transactionBuffer->skipXidList.insert(transaction->xid);
            transactionBuffer->dropTransaction(redoLogRecord1->xid, redoLogRecord1->conId);
            transaction->purge(transactionBuffer);
            if (transaction == lastTransaction)
                lastTransaction = nullptr;
            delete transaction;
            return;
        }

        transaction->add(metadata, transactionBuffer, redoLogRecord1, &zero);
    }

    void Parser::appendToTransactionLob(RedoLogRecord* redoLogRecord1) {
        OracleLob* lob = nullptr;
        {
            std::unique_lock<std::mutex> lckTransaction(metadata->mtxTransaction);
            lob = metadata->schema->checkLobDict(redoLogRecord1->dataObj);
        }

        if (lob == nullptr) {
            if (ctx->trace & TRACE_LOB)
                ctx->logTrace(TRACE_LOB, "skip dataobj: " + std::to_string(redoLogRecord1->dataObj) + " xid: " +
                              redoLogRecord1->xid.toString());
            return;
        }

        redoLogRecord1->lobPageSize = lob->checkLobPageSize(redoLogRecord1->dataObj);

        if (redoLogRecord1->xid.isEmpty()) {
            auto lobIdToXidMapIt = ctx->lobIdToXidMap.find(redoLogRecord1->lobId);
            if (lobIdToXidMapIt == ctx->lobIdToXidMap.end()) {
                transactionBuffer->addOrphanedLob(redoLogRecord1);
                return;
            } else
                redoLogRecord1->xid = lobIdToXidMapIt->second;
        }

        // Skip list
        if (transactionBuffer->skipXidList.find(redoLogRecord1->xid) != transactionBuffer->skipXidList.end())
            return;

        Transaction* transaction = transactionBuffer->findTransaction(redoLogRecord1->xid, redoLogRecord1->conId, true,
                                                                      FLAG(REDO_FLAGS_SHOW_INCOMPLETE_TRANSACTIONS), false);
        if (transaction == nullptr)
            return;
        lastTransaction = transaction;

        if (lob->table != nullptr && (lob->table->options & OPTIONS_SYSTEM_TABLE) != 0)
            transaction->system = true;
        if (lob->table != nullptr && (lob->table->options & OPTIONS_SCHEMA_TABLE) != 0)
            transaction->schema = true;

        if (ctx->trace & TRACE_LOB)
            ctx->logTrace(TRACE_LOB, "id: " + redoLogRecord1->lobId.lower() + " xid: " + transaction->xid.toString() + " obj: " +
                          std::to_string(redoLogRecord1->dataObj) +  " op: " + std::to_string(redoLogRecord1->opCode) + "     dba: " +
                          std::to_string(redoLogRecord1->dba) + " page: " + std::to_string(redoLogRecord1->lobPageNo) + " pg: " +
                          std::to_string(redoLogRecord1->lobPageSize));

        transaction->lobCtx.addLob(ctx, redoLogRecord1->lobId, redoLogRecord1->dba, 0, transactionBuffer->allocateLob(redoLogRecord1),
                                   transaction->xid, redoLogRecord1->dataOffset);
    }

    void Parser::appendToTransaction(RedoLogRecord* redoLogRecord1) {
        // Skip list
        if (redoLogRecord1->xid.getData() != 0 && transactionBuffer->skipXidList.find(redoLogRecord1->xid) != transactionBuffer->skipXidList.end())
            return;

        Transaction* transaction = transactionBuffer->findTransaction(redoLogRecord1->xid, redoLogRecord1->conId, true,
                                                                      FLAG(REDO_FLAGS_SHOW_INCOMPLETE_TRANSACTIONS), false);
        if (transaction == nullptr)
            return;
        lastTransaction = transaction;

        if (redoLogRecord1->opc != 0x0501 && redoLogRecord1->opc != 0x0A16 && redoLogRecord1->opc != 0x0B01) {
            transaction->log(ctx, "opc ", redoLogRecord1);
            return;
        }

        OracleTable* table = nullptr;
        {
            std::unique_lock<std::mutex> lckTransaction(metadata->mtxTransaction);
            table = metadata->schema->checkTableDict(redoLogRecord1->obj);
        }

        if (table == nullptr) {
            if (!FLAG(REDO_FLAGS_SCHEMALESS)) {
                transaction->log(ctx, "tbl ", redoLogRecord1);
                return;
            }
        } else {
            if ((table->options & OPTIONS_SYSTEM_TABLE) != 0)
                transaction->system = true;
            if ((table->options & OPTIONS_SCHEMA_TABLE) != 0)
                transaction->schema = true;
        }

        // Transaction size limit
        if (ctx->transactionSizeMax > 0 && transaction->size + redoLogRecord1->length + ROW_HEADER_TOTAL >= ctx->transactionSizeMax) {
            transaction->log(ctx, "siz ", redoLogRecord1);
            transactionBuffer->skipXidList.insert(transaction->xid);
            transactionBuffer->dropTransaction(redoLogRecord1->xid, redoLogRecord1->conId);
            transaction->purge(transactionBuffer);
            if (transaction == lastTransaction)
                lastTransaction = nullptr;
            delete transaction;
            return;
        }

        transaction->add(metadata, transactionBuffer, redoLogRecord1);
    }

    void Parser::appendToTransactionRollback(RedoLogRecord* redoLogRecord1) {
        if ((redoLogRecord1->opc != 0x0A16 && redoLogRecord1->opc != 0x0B01))
            return;

        if ((redoLogRecord1->flg & FLG_USERUNDODDONE) == 0)
            return;

        typeXid xid(redoLogRecord1->usn, redoLogRecord1->slt,  0);
        Transaction* transaction = transactionBuffer->findTransaction(xid, redoLogRecord1->conId, true, false, true);
        if (transaction == nullptr) {
            typeXidMap xidMap = (xid.getData() >> 32) | (static_cast<uint64_t>(redoLogRecord1->conId) << 32);
            auto brokenXidMapListIt = transactionBuffer->brokenXidMapList.find(xidMap);
            if (brokenXidMapListIt == transactionBuffer->brokenXidMapList.end()) {
                ctx->warning(60010, "no match found for transaction rollback, skipping, SLT: " +
                             std::to_string(static_cast<uint64_t>(redoLogRecord1->slt)) + ", USN: " +
                             std::to_string(static_cast<uint64_t>(redoLogRecord1->usn)));
                transactionBuffer->brokenXidMapList.insert(xidMap);
            }
            return;
        }
        lastTransaction = transaction;

        OracleTable* table = nullptr;
        {
            std::unique_lock<std::mutex> lckTransaction(metadata->mtxTransaction);
            table = metadata->schema->checkTableDict(redoLogRecord1->obj);
        }

        if (table == nullptr) {
            if (!FLAG(REDO_FLAGS_SCHEMALESS)) {
                transaction->log(ctx, "rls ", redoLogRecord1);
                return;
            }
        }

        transaction->rollbackLastOp(metadata, transactionBuffer, redoLogRecord1);
    }

    void Parser::appendToTransactionBegin(RedoLogRecord* redoLogRecord1) {
        // Skip SQN cleanup
        if (redoLogRecord1->xid.sqn() == 0)
            return;

        Transaction* transaction = transactionBuffer->findTransaction(redoLogRecord1->xid, redoLogRecord1->conId, false, true, false);
        transaction->begin = true;
        transaction->firstSequence = sequence;
        transaction->firstOffset = lwnCheckpointBlock * reader->getBlockSize();
        transaction->log(ctx, "B   ", redoLogRecord1);
        lastTransaction = transaction;
    }

    void Parser::appendToTransactionCommit(RedoLogRecord* redoLogRecord1) {
        // Clean LOB's if used
        for (auto lobIdToXidMapIt = ctx->lobIdToXidMap.begin(); lobIdToXidMapIt != ctx->lobIdToXidMap.end();) {
            if (lobIdToXidMapIt->second == redoLogRecord1->xid) {
                lobIdToXidMapIt = ctx->lobIdToXidMap.erase(lobIdToXidMapIt);
            } else
                ++lobIdToXidMapIt;
        }

        // Skip list
        auto skipXidListIt = transactionBuffer->skipXidList.find(redoLogRecord1->xid);
        if (skipXidListIt != transactionBuffer->skipXidList.end()) {
            transactionBuffer->skipXidList.erase(skipXidListIt);
            return;
        }

        // Broken transaction
        typeXidMap xidMap = (redoLogRecord1->xid.getData() >> 32) | (static_cast<uint64_t>(redoLogRecord1->conId) << 32);
        auto brokenXidMapListIt = transactionBuffer->brokenXidMapList.find(xidMap);
        if (brokenXidMapListIt != transactionBuffer->brokenXidMapList.end())
            transactionBuffer->brokenXidMapList.erase(xidMap);

        Transaction* transaction = transactionBuffer->findTransaction(redoLogRecord1->xid, redoLogRecord1->conId, true,
                                                                      FLAG(REDO_FLAGS_SHOW_INCOMPLETE_TRANSACTIONS), false);
        if (transaction == nullptr)
            return;

        transaction->log(ctx, "C   ", redoLogRecord1);
        transaction->commitTimestamp = lwnTimestamp;
        transaction->commitScn = redoLogRecord1->scnRecord;
        transaction->commitSequence = sequence;
        if ((redoLogRecord1->flg & FLG_ROLLBACK_OP0504) != 0)
            transaction->rollback = true;

        if ((transaction->commitScn > metadata->firstDataScn && !transaction->system) ||
            (transaction->commitScn > metadata->firstSchemaScn && transaction->system)) {

            if (transaction->begin) {
                transaction->flush(metadata, transactionBuffer, builder, lwnScn);

                if (ctx->stopTransactions > 0 && metadata->isNewData(lwnScn, builder->lwnIdx)) {
                    --ctx->stopTransactions;
                    if (ctx->stopTransactions == 0) {
                        ctx->info(0, "shutdown started - exhausted number of transactions");
                        ctx->stopSoft();
                    }
                }

                if (transaction->shutdown && metadata->isNewData(lwnScn, builder->lwnIdx)) {
                    ctx->info(0, "shutdown started - initiated by debug transaction " + transaction->xid.toString() +
                              " at scn " + std::to_string(transaction->commitScn));
                    ctx->stopSoft();
                }
            } else {
                ctx->warning(60011, "skipping transaction with no begin: " + transaction->toString());
            }
        } else {
            if (ctx->trace & TRACE_TRANSACTION)
                ctx->logTrace(TRACE_TRANSACTION, "skipping transaction already committed: " + transaction->toString());
        }

        transactionBuffer->dropTransaction(redoLogRecord1->xid, redoLogRecord1->conId);
        transaction->purge(transactionBuffer);
        lastTransaction = nullptr;
        delete transaction;
    }

    void Parser::appendToTransaction(RedoLogRecord* redoLogRecord1, RedoLogRecord* redoLogRecord2) {
        // Skip other PDB vectors
        if (metadata->conId > 0 && redoLogRecord2->conId != metadata->conId)
            return;

        // Skip list
        if (transactionBuffer->skipXidList.find(redoLogRecord1->xid) != transactionBuffer->skipXidList.end())
            return;

        Transaction* transaction = transactionBuffer->findTransaction(redoLogRecord1->xid, redoLogRecord1->conId, true,
                                                                      FLAG(REDO_FLAGS_SHOW_INCOMPLETE_TRANSACTIONS), false);
        if (transaction == nullptr)
            return;
        lastTransaction = transaction;

        typeObj obj;
        if (redoLogRecord1->dataObj != 0) {
            obj = redoLogRecord1->obj;
            redoLogRecord2->obj = redoLogRecord1->obj;
            redoLogRecord2->dataObj = redoLogRecord1->dataObj;
        } else {
            obj = redoLogRecord2->obj;
            redoLogRecord1->obj = redoLogRecord2->obj;
            redoLogRecord1->dataObj = redoLogRecord2->dataObj;
        }
        if (redoLogRecord1->bdba != redoLogRecord2->bdba && redoLogRecord1->bdba != 0 && redoLogRecord2->bdba != 0)
            throw RedoLogException(50045, "bdba does not match (" + std::to_string(redoLogRecord1->bdba) + ", " +
                                   std::to_string(redoLogRecord2->bdba) + "), offset: " + std::to_string(redoLogRecord1->dataOffset));

        switch (redoLogRecord2->opCode) {
            // Session information
            case 0x0513:
            case 0x0514:
                break;

            // Insert row piece
            case 0x0B02:
            // Delete row piece
            case 0x0B03:
            // Update row piece
            case 0x0B05:
            // Overwrite row piece
            case 0x0B06:
            // Change forwarding address
            case 0x0B08:
            // Insert multiple rows
            case 0x0B0B:
            // Delete multiple rows
            case 0x0B0C:
            // Supp log for update
            case 0x0B10:
            // Logminer support - KDOCMP
            case 0x0B16:
                {
                    OracleTable* table = nullptr;
                    {
                        std::unique_lock<std::mutex> lckTransaction(metadata->mtxTransaction);
                        table = metadata->schema->checkTableDict(obj);
                    }

                    if (table == nullptr) {
                        if (!FLAG(REDO_FLAGS_SCHEMALESS)) {
                            transaction->log(ctx, "tbl1", redoLogRecord1);
                            transaction->log(ctx, "tbl2", redoLogRecord2);
                            return;
                        }
                    } else {
                        if ((table->options & OPTIONS_SYSTEM_TABLE) != 0)
                            transaction->system = true;
                        if ((table->options & OPTIONS_SCHEMA_TABLE) != 0)
                            transaction->schema = true;
                        if ((table->options & OPTIONS_DEBUG_TABLE) != 0 && redoLogRecord2->opCode == 0x0B02 && !ctx->softShutdown)
                            transaction->shutdown = true;
                    }
                }
                break;

            default:
                transaction->log(ctx, "skp1", redoLogRecord1);
                transaction->log(ctx, "skp2", redoLogRecord2);
                return;
        }

        // Transaction size limit
        if (ctx->transactionSizeMax > 0 && transaction->size + redoLogRecord1->length + redoLogRecord2->length + ROW_HEADER_TOTAL >= ctx->transactionSizeMax) {
            transaction->log(ctx, "siz1", redoLogRecord1);
            transaction->log(ctx, "siz2", redoLogRecord2);
            transactionBuffer->skipXidList.insert(transaction->xid);
            transactionBuffer->dropTransaction(redoLogRecord1->xid, redoLogRecord1->conId);
            transaction->purge(transactionBuffer);
            if (transaction == lastTransaction)
                lastTransaction = nullptr;
            delete transaction;
            return;
        }

        transaction->add(metadata, transactionBuffer, redoLogRecord1, redoLogRecord2);
    }

    void Parser::appendToTransactionRollback(RedoLogRecord* redoLogRecord1, RedoLogRecord* redoLogRecord2) {
        // Skip other PDB vectors
        if (metadata->conId > 0 && redoLogRecord1->conId != metadata->conId)
            return;

        typeXid xid(redoLogRecord2->usn, redoLogRecord2->slt, 0);
        Transaction* transaction = transactionBuffer->findTransaction(xid, redoLogRecord2->conId, true, false, true);
        if (transaction == nullptr) {
            typeXidMap xidMap = (xid.getData() >> 32) | (static_cast<uint64_t>(redoLogRecord2->conId) << 32);
            auto brokenXidMapListIt = transactionBuffer->brokenXidMapList.find(xidMap);
            if (brokenXidMapListIt == transactionBuffer->brokenXidMapList.end()) {
                ctx->warning(60010, "no match found for transaction rollback, skipping, SLT: " +
                             std::to_string(static_cast<uint64_t>(redoLogRecord2->slt)) + ", USN: " +
                             std::to_string(static_cast<uint64_t>(redoLogRecord2->usn)));
                transactionBuffer->brokenXidMapList.insert(xidMap);
            }
            return;
        }
        lastTransaction = transaction;
        redoLogRecord1->xid = transaction->xid;

        // Skip list
        if (transactionBuffer->skipXidList.find(redoLogRecord1->xid) != transactionBuffer->skipXidList.end())
            return;

        typeObj obj;
        if (redoLogRecord1->dataObj != 0) {
            obj = redoLogRecord1->obj;
            redoLogRecord2->obj = redoLogRecord1->obj;
            redoLogRecord2->dataObj = redoLogRecord1->dataObj;
        } else {
            obj = redoLogRecord2->obj;
            redoLogRecord1->obj = redoLogRecord2->obj;
            redoLogRecord1->dataObj = redoLogRecord2->dataObj;
        }
        if (redoLogRecord1->bdba != redoLogRecord2->bdba && redoLogRecord1->bdba != 0 && redoLogRecord2->bdba != 0)
            throw RedoLogException(50045, "bdba does not match (" + std::to_string(redoLogRecord1->bdba) + ", " +
                                   std::to_string(redoLogRecord2->bdba) + "), offset: " + std::to_string(redoLogRecord1->dataOffset));

        OracleTable* table = nullptr;
        {
            std::unique_lock<std::mutex> lckTransaction(metadata->mtxTransaction);
            table = metadata->schema->checkTableDict(obj);
        }

        if (table == nullptr) {
            if (!FLAG(REDO_FLAGS_SCHEMALESS)) {
                transaction->log(ctx, "rls1", redoLogRecord1);
                transaction->log(ctx, "rls2", redoLogRecord2);
                return;
            }
        } else {
            if ((table->options & OPTIONS_SYSTEM_TABLE) != 0)
                transaction->system = true;
            if ((table->options & OPTIONS_SCHEMA_TABLE) != 0)
                transaction->schema = true;
        }

        switch (redoLogRecord1->opCode) {
            // Insert row piece
            case 0x0B02:
            // Delete row piece
            case 0x0B03:
            // Update row piece
            case 0x0B05:
            // Overwrite row piece
            case 0x0B06:
            // Change forwarding address
            case 0x0B08:
            // Insert multiple rows
            case 0x0B0B:
            // Delete multiple rows
            case 0x0B0C:
            // Supp log for update
            case 0x0B10:
            // Logminer support - KDOCMP
            case 0x0B16:
                break;

            default:
                transaction->log(ctx, "skp1", redoLogRecord1);
                transaction->log(ctx, "skp2", redoLogRecord2);
                return;
        }

        transaction->rollbackLastOp(metadata, transactionBuffer, redoLogRecord1, redoLogRecord2);
    }

    void Parser::appendToTransactionIndex(RedoLogRecord* redoLogRecord1, RedoLogRecord* redoLogRecord2) {
        // Skip other PDB vectors
        if (metadata->conId > 0 && redoLogRecord2->conId != metadata->conId)
            return;

        // Skip list
        if (transactionBuffer->skipXidList.find(redoLogRecord1->xid) != transactionBuffer->skipXidList.end())
            return;

        Transaction* transaction = transactionBuffer->findTransaction(redoLogRecord1->xid, redoLogRecord1->conId, true,
                                                                      FLAG(REDO_FLAGS_SHOW_INCOMPLETE_TRANSACTIONS), false);
        if (transaction == nullptr)
            return;
        lastTransaction = transaction;

        typeDataObj dataObj;
        if (redoLogRecord1->dataObj != 0) {
            dataObj = redoLogRecord1->dataObj;
            redoLogRecord2->obj = redoLogRecord1->obj;
            redoLogRecord2->dataObj = redoLogRecord1->dataObj;
        } else {
            dataObj = redoLogRecord2->dataObj;
            redoLogRecord1->obj = redoLogRecord2->obj;
            redoLogRecord1->dataObj = redoLogRecord2->dataObj;
        }
        if (redoLogRecord1->bdba != redoLogRecord2->bdba && redoLogRecord1->bdba != 0 && redoLogRecord2->bdba != 0)
            throw RedoLogException(50045, "bdba does not match (" + std::to_string(redoLogRecord1->bdba) + ", " +
                                   std::to_string(redoLogRecord2->bdba) + "), offset: " + std::to_string(redoLogRecord1->dataOffset));

        OracleLob* lob = nullptr;
        {
            std::unique_lock<std::mutex> lckTransaction(metadata->mtxTransaction);
            lob = metadata->schema->checkLobIndexDict(dataObj);
        }

        if (lob == nullptr && redoLogRecord2->opCode != 0x1A02) {
            if (ctx->trace & TRACE_LOB)
                ctx->logTrace(TRACE_LOB, "skip index dataobj: " + std::to_string(dataObj) + " (" +
                              std::to_string(redoLogRecord1->dataObj) + ", " + std::to_string(redoLogRecord2->dataObj) + ")" + " xid: " +
                              redoLogRecord1->xid.toString());

            transaction->log(ctx, "idx1", redoLogRecord1);
            transaction->log(ctx, "idx2", redoLogRecord2);
            return;
        }

        if (redoLogRecord2->opCode == 0x0A02) {
            if (redoLogRecord2->indKeyLength == 16 && redoLogRecord2->data[redoLogRecord2->indKey] == 10 &&
                    redoLogRecord2->data[redoLogRecord2->indKey + 11] == 4) {
                redoLogRecord2->lobId.set(redoLogRecord2->data + redoLogRecord2->indKey + 1);
                redoLogRecord2->lobPageNo = ctx->read32Big(redoLogRecord2->data + redoLogRecord2->indKey + 12);
            } else
                return;
        } else
        if (redoLogRecord2->opCode == 0x0A08) {
            if (redoLogRecord2->indKey == 0)
                return;

            if (redoLogRecord2->indKeyLength == 50 && redoLogRecord2->data[redoLogRecord2->indKey] == 0x01 &&
                    redoLogRecord2->data[redoLogRecord2->indKey + 1] == 0x01 &&
                    redoLogRecord2->data[redoLogRecord2->indKey + 34] == 10 && redoLogRecord2->data[redoLogRecord2->indKey + 45] == 4) {
                redoLogRecord2->lobId.set(redoLogRecord2->data + redoLogRecord2->indKey + 35);
                redoLogRecord2->lobPageNo = ctx->read32Big(redoLogRecord2->data + redoLogRecord2->indKey + 46);
                redoLogRecord2->indKeyData = redoLogRecord2->indKey + 2;
                redoLogRecord2->indKeyDataLength = 32;
            } else {
                ctx->warning(60014, "verify redo log file for OP:10.8, len: " + std::to_string(redoLogRecord2->indKeyLength) +
                             ", data = [" + std::to_string(static_cast<uint64_t>(redoLogRecord2->data[redoLogRecord2->indKey])) + ", " +
                             std::to_string(static_cast<uint64_t>(redoLogRecord2->data[redoLogRecord2->indKey + 1])) + ", " +
                             std::to_string(static_cast<uint64_t>(redoLogRecord2->data[redoLogRecord2->indKey + 34])) + ", " +
                             std::to_string(static_cast<uint64_t>(redoLogRecord2->data[redoLogRecord2->indKey + 45])) + "]");
                return;
            }

            auto lobIdToXidMapIt = ctx->lobIdToXidMap.find(redoLogRecord2->lobId);
            if (lobIdToXidMapIt != ctx->lobIdToXidMap.end()) {
                typeXid parentXid = lobIdToXidMapIt->second;

                if (parentXid != redoLogRecord1->xid) {
                    if (ctx->trace & TRACE_LOB)
                        ctx->logTrace(TRACE_LOB, "id: " + redoLogRecord2->lobId.lower() + " xid: " + parentXid.toString() +
                                      " sub-xid: " + redoLogRecord1->xid.toString());
                    redoLogRecord1->xid = parentXid;
                    redoLogRecord2->xid = parentXid;

                    transaction = transactionBuffer->findTransaction(redoLogRecord1->xid, redoLogRecord1->conId, true,
                                                                     FLAG(REDO_FLAGS_SHOW_INCOMPLETE_TRANSACTIONS), false);
                    if (transaction == nullptr) {
                        if (ctx->trace & TRACE_LOB)
                            ctx->logTrace(TRACE_LOB, "parent transaction not found");
                        return;
                    }
                    lastTransaction = transaction;
                }
            }
        } else
        if (redoLogRecord2->opCode == 0x0A12) {
            if (redoLogRecord1->indKeyLength == 16 && redoLogRecord1->data[redoLogRecord1->indKey] == 10 &&
                    redoLogRecord1->data[redoLogRecord1->indKey + 11] == 4) {
                redoLogRecord2->lobId.set(redoLogRecord1->data + redoLogRecord1->indKey + 1);
                redoLogRecord2->lobPageNo = ctx->read32Big(redoLogRecord1->data + redoLogRecord1->indKey + 12);
                redoLogRecord2->lobLengthPages = ctx->read32Big(redoLogRecord2->data + redoLogRecord2->indKeyData + 4);
                redoLogRecord2->lobLengthRest = ctx->read16Big(redoLogRecord2->data + redoLogRecord2->indKeyData + 8);
            } else
                return;
        }

        switch (redoLogRecord2->opCode) {
            // Insert leaf row
            case 0x0A02:
            // Init header
            case 0x0A08:
            // Update key data in row
            case 0x0A12:
            // LOB index 12+ and LOB redo
            case 0x1A02:
                break;

            default:
                transaction->log(ctx, "skp1", redoLogRecord1);
                transaction->log(ctx, "skp2", redoLogRecord2);
                return;
        }

        if (redoLogRecord2->lobId.data[0] != 0 || redoLogRecord2->lobId.data[1] != 0 ||
            redoLogRecord2->lobId.data[2] != 0 || redoLogRecord2->lobId.data[3] != 1)
            return;

        if ((ctx->trace & TRACE_LOB) != 0) {
            std::ostringstream ss;
            if (redoLogRecord1->indKeyLength > 0)
                ss << "0x";
            for (uint64_t i = 0; i < redoLogRecord1->indKeyLength; ++i)
                ss << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(redoLogRecord1->data[redoLogRecord1->indKey + i]);
            if (redoLogRecord2->indKeyLength > 0)
                ss << " 0x";
            for (uint64_t i = 0; i < redoLogRecord2->indKeyLength; ++i)
                ss << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(redoLogRecord2->data[redoLogRecord2->indKey + i]);

            ctx->logTrace(TRACE_LOB, "id: " + redoLogRecord2->lobId.lower() + " xid: " + redoLogRecord1->xid.toString() + " obj: " +
                          std::to_string(redoLogRecord2->dataObj) + " op: " + std::to_string(redoLogRecord1->opCode) + ":" +
                          std::to_string(redoLogRecord2->opCode) + " dba: " + std::to_string(redoLogRecord2->dba) + " page: " +
                          std::to_string(redoLogRecord2->lobPageNo) + " ind key: " + ss.str());
        }

        auto lobIdToXidMapIt = ctx->lobIdToXidMap.find(redoLogRecord2->lobId);
        if (lobIdToXidMapIt == ctx->lobIdToXidMap.end()) {
            if (ctx->trace & TRACE_LOB)
                ctx->logTrace(TRACE_LOB, "id: " + redoLogRecord2->lobId.lower() + " xid: " + redoLogRecord1->xid.toString() + " MAP");
            ctx->lobIdToXidMap.insert_or_assign(redoLogRecord2->lobId, redoLogRecord1->xid);
            transaction->lobCtx.checkOrphanedLobs(ctx, redoLogRecord2->lobId, redoLogRecord1->xid, redoLogRecord1->dataOffset);
        }

        if (lob != nullptr) {
            if (lob->table != nullptr && (lob->table->options & OPTIONS_SYSTEM_TABLE) != 0)
                transaction->system = true;
            if (lob->table != nullptr && (lob->table->options & OPTIONS_SCHEMA_TABLE) != 0)
                transaction->schema = true;
        }

        // Transaction size limit
        if (ctx->transactionSizeMax > 0 &&
                transaction->size + redoLogRecord1->length + redoLogRecord2->length + ROW_HEADER_TOTAL >= ctx->transactionSizeMax) {
            transactionBuffer->skipXidList.insert(transaction->xid);
            transactionBuffer->dropTransaction(redoLogRecord1->xid, redoLogRecord1->conId);
            transaction->purge(transactionBuffer);
            if (transaction == lastTransaction)
                lastTransaction = nullptr;
            delete transaction;
            return;
        }

        transaction->add(metadata, transactionBuffer, redoLogRecord1, redoLogRecord2);
    }

    void Parser::dumpRedoVector(uint8_t* data, uint64_t recordLength) const {
        if (ctx->logLevel >= LOG_LEVEL_WARNING) {
            std::ostringstream ss;
            ss << "dumping redo vector\n";
            ss << "##: " << std::dec << recordLength;
            for (uint64_t j = 0; j < recordLength; ++j) {
                if ((j & 0x0F) == 0)
                    ss << "\n##  " << std::setfill(' ') << std::setw(2) << std::hex << j << ": ";
                if ((j & 0x07) == 0)
                    ss << " ";
                ss << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(data[j]) << " ";
            }
            ctx->warning(70002, ss.str());
        }
    }

    uint64_t Parser::parse() {
        uint64_t lwnConfirmedBlock = 2;
        uint64_t lwnRecords = 0;

        if (firstScn == ZERO_SCN && nextScn == ZERO_SCN && reader->getFirstScn() != 0) {
            firstScn = reader->getFirstScn();
            nextScn = reader->getNextScn();
        }
        ctx->suppLogSize = 0;

        if (reader->getBufferStart() == reader->getBlockSize() * 2) {
            if (ctx->dumpRedoLog >= 1) {
                std::string fileName = ctx->dumpPath + "/" + std::to_string(sequence) + ".olr";
                ctx->dumpStream.open(fileName);
                if (!ctx->dumpStream.is_open()) {
                    ctx->error(10006, "file: " + fileName + " - open for write returned: " + strerror(errno));
                    ctx->warning(60012, "aborting log dump");
                    ctx->dumpRedoLog = 0;
                }
                std::ostringstream ss;
                reader->printHeaderInfo(ss, path);
                ctx->dumpStream << ss.str();
            }
        }

        // Continue started offset
        if (metadata->offset > 0) {
            if ((metadata->offset % reader->getBlockSize()) != 0)
                throw RedoLogException(50047, "incorrect offset start: " + std::to_string(metadata->offset) +
                                       " - not a multiplication of block size: " + std::to_string(reader->getBlockSize()));

            lwnConfirmedBlock = metadata->offset / reader->getBlockSize();
            if (ctx->trace & TRACE_CHECKPOINT)
                ctx->logTrace(TRACE_CHECKPOINT, "setting reader start position to " + std::to_string(metadata->offset) + " (block " +
                              std::to_string(lwnConfirmedBlock) + ")");
            metadata->offset = 0;
        }
        reader->setBufferStartEnd(lwnConfirmedBlock * reader->getBlockSize(),
                                  lwnConfirmedBlock * reader->getBlockSize());

        ctx->info(0, "processing redo log: " + toString() + " offset: " + std::to_string(reader->getBufferStart()));
        if (FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA) && !metadata->schema->loaded && ctx->versionStr.length() > 0) {
            metadata->loadAdaptiveSchema();
            metadata->schema->loaded = true;
        }

        if (metadata->resetlogs == 0)
            metadata->setResetlogs(reader->getResetlogs());

        if (metadata->resetlogs != reader->getResetlogs())
            throw RedoLogException(50048, "invalid resetlogs value (found: " + std::to_string(reader->getResetlogs()) + ", expected: " +
                                   std::to_string(metadata->resetlogs) + "): " + reader->fileName);

        if (reader->getActivation() != 0 && (metadata->activation == 0 || metadata->activation != reader->getActivation())) {
            ctx->info(0, "new activation detected: " + std::to_string(reader->getActivation()));
            metadata->setActivation(reader->getActivation());
        }

        time_t cStart = Timer::getTime();
        reader->setStatusRead();
        LwnMember* lwnMember;
        uint64_t currentBlock = lwnConfirmedBlock;
        uint64_t blockOffset = 16;
        uint64_t startBlock = lwnConfirmedBlock;
        uint64_t confirmedBufferStart = reader->getBufferStart();
        uint64_t recordLength4 = 0;
        uint64_t recordPos = 0;
        uint64_t recordLeftToCopy = 0;
        uint64_t lwnEndBlock = lwnConfirmedBlock;
        uint64_t lwnStartBlock = lwnConfirmedBlock;
        uint16_t lwnNum = 0;
        uint16_t lwnNumMax = 0;
        uint16_t lwnNumCur = 0;
        uint16_t lwnNumCnt = 0;
        lwnCheckpointBlock = lwnConfirmedBlock;
        bool switchRedo = false;

        while (!ctx->softShutdown) {
            // There is some work to do
            while (confirmedBufferStart < reader->getBufferEnd()) {
                uint64_t redoBufferPos = (currentBlock * reader->getBlockSize()) % MEMORY_CHUNK_SIZE;
                uint64_t redoBufferNum = ((currentBlock * reader->getBlockSize()) / MEMORY_CHUNK_SIZE) % ctx->readBufferMax;
                uint8_t* redoBlock = reader->redoBufferList[redoBufferNum] + redoBufferPos;

                blockOffset = 16;
                // New LWN block
                if (currentBlock == lwnEndBlock) {
                    uint8_t vld = redoBlock[blockOffset + 4];

                    if ((vld & 0x04) != 0) {
                        lwnNum = ctx->read16(redoBlock + blockOffset + 24);
                        uint32_t lwnLength = ctx->read32(redoBlock + blockOffset + 28);
                        lwnStartBlock = currentBlock;
                        lwnEndBlock = currentBlock + lwnLength;
                        lwnScn = ctx->readScn(redoBlock + blockOffset + 40);
                        lwnTimestamp = ctx->read32(redoBlock + blockOffset + 64);

                        if (lwnNumCnt == 0) {
                            lwnCheckpointBlock = currentBlock;
                            lwnNumMax = ctx->read16(redoBlock + blockOffset + 26);
                            // Verify LWN header start
                            if (lwnScn < reader->getFirstScn() || (lwnScn > reader->getNextScn() && reader->getNextScn() != ZERO_SCN))
                                throw RedoLogException(50049, "invalid lwn scn: " + std::to_string(lwnScn));
                        } else {
                            lwnNumCur = ctx->read16(redoBlock + blockOffset + 26);
                            if (lwnNumCur != lwnNumMax)
                                throw RedoLogException(50050, "invalid lwn max: " + std::to_string(lwnNum) + "/" +
                                                       std::to_string(lwnNumCur) + "/" + std::to_string(lwnNumMax));
                        }
                        ++lwnNumCnt;

                        if (ctx->trace & TRACE_LWN)
                            ctx->logTrace(TRACE_LWN, "at: " + std::to_string(lwnStartBlock) + " length: " + std::to_string(lwnLength) +
                                          " chk: " + std::to_string(lwnNum) + " max: " + std::to_string(lwnNumMax));

                    } else
                        throw RedoLogException(50051, "did not find lwn at offset: " + std::to_string(confirmedBufferStart));
                }

                while (blockOffset < reader->getBlockSize()) {
                    // Next record
                    if (recordLeftToCopy == 0) {
                        if (blockOffset + 20 >= reader->getBlockSize())
                            break;

                        recordLength4 = (static_cast<uint64_t>(ctx->read32(redoBlock + blockOffset)) + 3) & 0xFFFFFFFC;
                        if (recordLength4 > 0) {
                            uint64_t* length = reinterpret_cast<uint64_t*>(lwnChunks[lwnAllocated - 1]);

                            if (((*length + sizeof(struct LwnMember) + recordLength4 + 7) & 0xFFFFFFF8) > MEMORY_CHUNK_SIZE_MB * 1024 * 1024) {
                                if (lwnAllocated == MAX_LWN_CHUNKS)
                                    throw RedoLogException(50052, "all " + std::to_string(MAX_LWN_CHUNKS) + " lwn buffers allocated");

                                lwnChunks[lwnAllocated++] = ctx->getMemoryChunk("parser", false);
                                if (lwnAllocated > lwnAllocatedMax)
                                    lwnAllocatedMax = lwnAllocated;
                                length = reinterpret_cast<uint64_t*>(lwnChunks[lwnAllocated - 1]);
                                *length = sizeof(uint64_t);
                            }

                            if (((*length + sizeof(struct LwnMember) + recordLength4 + 7) & 0xFFFFFFF8) > MEMORY_CHUNK_SIZE_MB * 1024 * 1024)
                                throw RedoLogException(50053, "too big redo log record, length: " + std::to_string(recordLength4));

                            lwnMember = reinterpret_cast<struct LwnMember*>(lwnChunks[lwnAllocated - 1] + *length);
                            *length += (sizeof(struct LwnMember) + recordLength4 + 7) & 0xFFFFFFF8;
                            lwnMember->scn = ctx->read32(redoBlock + blockOffset + 8) |
                                             (static_cast<uint64_t>(ctx->read16(redoBlock + blockOffset + 6)) << 32);
                            lwnMember->subScn = ctx->read16(redoBlock + blockOffset + 12);
                            lwnMember->block = currentBlock;
                            lwnMember->offset = blockOffset;
                            lwnMember->length = recordLength4;
                            if (ctx->trace & TRACE_LWN)
                                ctx->logTrace(TRACE_LWN, "length: " + std::to_string(recordLength4) + " scn: " +
                                              std::to_string(lwnMember->scn) + " subscn: " + std::to_string(lwnMember->subScn));

                            uint64_t lwnPos = lwnRecords++;
                            if (lwnPos >= MAX_RECORDS_IN_LWN)
                                throw RedoLogException(50054, "all " + std::to_string(lwnPos) + " records in lwn were used");

                            while (lwnPos > 0 &&
                                   (lwnMembers[lwnPos - 1]->scn > lwnMember->scn ||
                                    (lwnMembers[lwnPos - 1]->scn == lwnMember->scn && lwnMembers[lwnPos - 1]->subScn > lwnMember->subScn))) {
                                lwnMembers[lwnPos] = lwnMembers[lwnPos - 1];
                                --lwnPos;
                            }
                            lwnMembers[lwnPos] = lwnMember;
                        }

                        recordLeftToCopy = recordLength4;
                        recordPos = 0;
                    }

                    // Nothing more
                    if (recordLeftToCopy == 0)
                        break;

                    uint64_t toCopy;
                    if (blockOffset + recordLeftToCopy > reader->getBlockSize())
                        toCopy = reader->getBlockSize() - blockOffset;
                    else
                        toCopy = recordLeftToCopy;

                    memcpy(reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(lwnMember) + sizeof(struct LwnMember) + recordPos),
                           reinterpret_cast<const void*>(redoBlock + blockOffset), toCopy);
                    recordLeftToCopy -= toCopy;
                    blockOffset += toCopy;
                    recordPos += toCopy;
                }

                ++currentBlock;
                confirmedBufferStart += reader->getBlockSize();
                redoBufferPos += reader->getBlockSize();

                // Checkpoint
                if (ctx->trace & TRACE_LWN)
                    ctx->logTrace(TRACE_LWN, "checkpoint at " + std::to_string(currentBlock) + "/" + std::to_string(lwnEndBlock) +
                                  " num: " + std::to_string(lwnNumCnt) + "/" + std::to_string(lwnNumMax));
                if (currentBlock == lwnEndBlock && lwnNumCnt == lwnNumMax) {
                    lastTransaction = nullptr;

                    if (ctx->trace & TRACE_LWN)
                        ctx->logTrace(TRACE_LWN, "* analyze: " + std::to_string(lwnScn));
                    for (uint64_t i = 0; i < lwnRecords; ++i) {
                        try {
                            analyzeLwn(lwnMembers[i]);
                        } catch (DataException &ex) {
                            if (FLAG(REDO_FLAGS_IGNORE_DATA_ERRORS)) {
                                ctx->error(ex.code, ex.msg);
                                ctx->warning(60013, "forced to continue working in spite of error");
                            } else
                                throw DataException(ex.code, "runtime error, aborting further redo log processing: " + ex.msg);
                        } catch (RedoLogException &ex) {
                            if (FLAG(REDO_FLAGS_IGNORE_DATA_ERRORS)) {
                                ctx->error(ex.code, ex.msg);
                                ctx->warning(60013, "forced to continue working in spite of error");
                            } else
                                throw RedoLogException(ex.code, "runtime error, aborting further redo log processing: " + ex.msg);
                        }
                    }

                    if (lwnScn > metadata->firstDataScn) {
                        if (ctx->trace & TRACE_CHECKPOINT)
                            ctx->logTrace(TRACE_CHECKPOINT, "on: " + std::to_string(lwnScn));
                        builder->processCheckpoint(lwnScn, sequence, lwnTimestamp, currentBlock * reader->getBlockSize(), switchRedo);

                        typeSeq minSequence = ZERO_SEQ;
                        uint64_t minOffset = -1;
                        typeXid minXid;
                        transactionBuffer->checkpoint(minSequence, minOffset, minXid);
                        if (ctx->trace & TRACE_LWN)
                            ctx->logTrace(TRACE_LWN, "* checkpoint: " + std::to_string(lwnScn));
                        metadata->checkpoint(lwnScn, lwnTimestamp, sequence,
                                             currentBlock * reader->getBlockSize(),
                                             (currentBlock - lwnConfirmedBlock) * reader->getBlockSize(), minSequence,
                                             minOffset, minXid);

                        if (ctx->stopCheckpoints > 0 && metadata->isNewData(lwnScn, builder->lwnIdx)) {
                            --ctx->stopCheckpoints;
                            if (ctx->stopCheckpoints == 0) {
                                ctx->info(0, "shutdown started - exhausted number of checkpoints");
                                ctx->stopSoft();
                            }
                        }
                    }

                    lwnNumCnt = 0;
                    freeLwn();
                    lwnRecords = 0;
                    lwnConfirmedBlock = currentBlock;
                } else if (lwnNumCnt > lwnNumMax)
                    throw RedoLogException(50055, "lwn overflow: " + std::to_string(lwnNumCnt) + "/" + std::to_string(lwnNumMax));

                // Free memory
                if (redoBufferPos == MEMORY_CHUNK_SIZE) {
                    redoBufferPos = 0;
                    reader->bufferFree(redoBufferNum);
                    if (++redoBufferNum == ctx->readBufferMax)
                        redoBufferNum = 0;
                    reader->confirmReadData(confirmedBufferStart);
                }
            }

            // Processing finished
            if (!switchRedo && lwnScn > 0 && lwnScn > metadata->firstDataScn &&
                    confirmedBufferStart == reader->getBufferEnd() && reader->getRet() == REDO_FINISHED) {
                switchRedo = true;
                if (ctx->trace & TRACE_CHECKPOINT)
                    ctx->logTrace(TRACE_CHECKPOINT, "on: " + std::to_string(lwnScn) + " with switch");
                builder->processCheckpoint(lwnScn, sequence, lwnTimestamp, currentBlock * reader->getBlockSize(), switchRedo);
            } else if (ctx->softShutdown) {
                if (ctx->trace & TRACE_CHECKPOINT)
                    ctx->logTrace(TRACE_CHECKPOINT, "on: " + std::to_string(lwnScn) + " at exit");
                builder->processCheckpoint(lwnScn, sequence, lwnTimestamp, currentBlock * reader->getBlockSize(), false);
            }

            if (ctx->softShutdown) {
                reader->setRet(REDO_SHUTDOWN);
            } else {
                if (reader->checkFinished(confirmedBufferStart)) {
                    if (reader->getRet() == REDO_FINISHED && nextScn == ZERO_SCN && reader->getNextScn() != ZERO_SCN)
                        nextScn = reader->getNextScn();
                    if (reader->getRet() == REDO_STOPPED || reader->getRet() == REDO_OVERWRITTEN)
                        metadata->offset = lwnConfirmedBlock * reader->getBlockSize();
                    break;
                }
            }
        }

        // Print performance information
        if ((ctx->trace & TRACE_PERFORMANCE) != 0) {
            double suppLogPercent = 0.0;
            if (currentBlock != startBlock)
                suppLogPercent = 100.0 * ctx->suppLogSize / ((currentBlock - startBlock) * reader->getBlockSize());

            if (group == 0) {
                time_t cEnd = Timer::getTime();
                double mySpeed = 0;
                double myTime = (double)(cEnd - cStart) / 1000.0;
                if (myTime > 0)
                    mySpeed = (double)(currentBlock - startBlock) * reader->getBlockSize() * 1000.0 / 1024 / 1024 / myTime;

                double myReadSpeed = 0;
                if (reader->getSumTime() > 0)
                    myReadSpeed = ((double)reader->getSumRead() * 1000000.0 / 1024 / 1024 / (double)reader->getSumTime());

                ctx->logTrace(TRACE_PERFORMANCE, std::to_string(myTime) + " ms, " +
                              "Speed: " + std::to_string(mySpeed) + " MB/s, " +
                              "Redo log size: " + std::to_string((currentBlock - startBlock) * reader->getBlockSize() / 1024 / 1024) + " MB, " +
                              "Read size: " + std::to_string(reader->getSumRead() / 1024 / 1024) + " MB, " +
                              "Read speed: " + std::to_string(myReadSpeed) + " MB/s, " +
                              "Max LWN size: " + std::to_string(lwnAllocatedMax) + ", " +
                              "Supplemental redo log size: " + std::to_string(ctx->suppLogSize) + " bytes " +
                              "(" + std::to_string(suppLogPercent) + " %)");
            } else {
                ctx->logTrace(TRACE_PERFORMANCE,
                              "Redo log size: " + std::to_string((currentBlock - startBlock) * reader->getBlockSize() / 1024 / 1024) + " MB, " +
                              "Max LWN size: " + std::to_string(lwnAllocatedMax) + ", " +
                              "Supplemental redo log size: " + std::to_string(ctx->suppLogSize) + " bytes " +
                              "(" + std::to_string(suppLogPercent) + " %)");
            }
        }

        if (ctx->dumpRedoLog >= 1 && ctx->dumpStream.is_open()) {
            ctx->dumpStream << "END OF REDO DUMP\n";
            ctx->dumpStream.close();
        }

        freeLwn();
        return reader->getRet();
    }

    std::string Parser::toString() {
        return "group: " + std::to_string(group) + " scn: " + std::to_string(firstScn) + " to " +
                std::to_string(nextScn != ZERO_SCN ? nextScn : 0) + " seq: " + std::to_string(sequence) + " path: " + path;
    }
}
