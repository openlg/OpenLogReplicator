/* Oracle Redo Generic OpCode
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

#include <cstring>

#include "../common/RedoLogRecord.h"
#include "OpCode.h"

namespace OpenLogReplicator {
    void OpCode::process(Ctx* ctx, RedoLogRecord* redoLogRecord) {
        if (ctx->dumpRedoLog >= 1) {
            bool encrypted = false;
            if ((redoLogRecord->typ & 0x80) != 0)
                encrypted = true;

            if (ctx->version < REDO_VERSION_12_1) {
                if (redoLogRecord->typ == 6)
                    ctx->dumpStream << "CHANGE #" << std::dec << static_cast<uint64_t>(redoLogRecord->vectorNo) <<
                            " MEDIA RECOVERY MARKER" <<
                            " SCN:" << PRINTSCN48(redoLogRecord->scnRecord) <<
                            " SEQ:" << std::dec << static_cast<uint64_t>(redoLogRecord->seq) <<
                            " OP:" << static_cast<uint64_t>(redoLogRecord->opCode >> 8) << "." << static_cast<uint64_t>(redoLogRecord->opCode & 0xFF) <<
                            " ENC:" << std::dec << static_cast<uint64_t>(encrypted) << '\n';
                else
                    ctx->dumpStream << "CHANGE #" << std::dec << static_cast<uint64_t>(redoLogRecord->vectorNo) <<
                            " TYP:" << static_cast<uint64_t>(redoLogRecord->typ) <<
                            " CLS:" << redoLogRecord->cls <<
                            " AFN:" << redoLogRecord->afn <<
                            " DBA:0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->dba <<
                            " OBJ:" << std::dec << redoLogRecord->recordDataObj <<
                            " SCN:" << PRINTSCN48(redoLogRecord->scnRecord) <<
                            " SEQ:" << std::dec << static_cast<uint64_t>(redoLogRecord->seq) <<
                            " OP:" << static_cast<uint64_t>(redoLogRecord->opCode >> 8) << "." << static_cast<uint64_t>(redoLogRecord->opCode & 0xFF) <<
                            " ENC:" << std::dec << static_cast<uint64_t>(encrypted) <<
                            " RBL:" << std::dec << redoLogRecord->rbl << '\n';
            } else if (ctx->version < REDO_VERSION_12_2) {
                if (redoLogRecord->typ == 6)
                    ctx->dumpStream << "CHANGE #" << std::dec << static_cast<uint64_t>(redoLogRecord->vectorNo) <<
                            " MEDIA RECOVERY MARKER" <<
                            " CON_ID:" << redoLogRecord->conId <<
                            " SCN:" << PRINTSCN48(redoLogRecord->scnRecord) <<
                            " SEQ:" << std::dec << static_cast<uint64_t>(redoLogRecord->seq) <<
                            " OP:" << static_cast<uint64_t>(redoLogRecord->opCode >> 8) << "." << static_cast<uint64_t>(redoLogRecord->opCode & 0xFF) <<
                            " ENC:" << std::dec << static_cast<uint64_t>(encrypted) <<
                            " FLG:0x" << std::setw(4) << std::hex << redoLogRecord->flgRecord << '\n';
                else
                    ctx->dumpStream << "CHANGE #" << std::dec << static_cast<uint64_t>(redoLogRecord->vectorNo) <<
                            " CON_ID:" << redoLogRecord->conId <<
                            " TYP:" << static_cast<uint64_t>(redoLogRecord->typ) <<
                            " CLS:" << redoLogRecord->cls <<
                            " AFN:" << redoLogRecord->afn <<
                            " DBA:0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->dba <<
                            " OBJ:" << std::dec << redoLogRecord->recordDataObj <<
                            " SCN:" << PRINTSCN48(redoLogRecord->scnRecord) <<
                            " SEQ:" << std::dec << static_cast<uint64_t>(redoLogRecord->seq) <<
                            " OP:" << static_cast<uint64_t>(redoLogRecord->opCode >> 8) << "." << static_cast<uint64_t>(redoLogRecord->opCode & 0xFF) <<
                            " ENC:" << std::dec << static_cast<uint64_t>(encrypted) <<
                            " RBL:" << std::dec << redoLogRecord->rbl <<
                            " FLG:0x" << std::setw(4) << std::hex << redoLogRecord->flgRecord << '\n';
            } else {
                if (redoLogRecord->typ == 6)
                    ctx->dumpStream << "CHANGE #" << std::dec << static_cast<uint64_t>(redoLogRecord->vectorNo) <<
                            " MEDIA RECOVERY MARKER" <<
                            " CON_ID:" << redoLogRecord->conId <<
                            " SCN:" << PRINTSCN64(redoLogRecord->scnRecord) <<
                            " SEQ:" << std::dec << static_cast<uint64_t>(redoLogRecord->seq) <<
                            " OP:" << static_cast<uint64_t>(redoLogRecord->opCode >> 8) << "." << static_cast<uint64_t>(redoLogRecord->opCode & 0xFF) <<
                            " ENC:" << std::dec << static_cast<uint64_t>(encrypted) <<
                            " FLG:0x" << std::setw(4) << std::hex << redoLogRecord->flgRecord << '\n';
                else
                    ctx->dumpStream << "CHANGE #" << std::dec << static_cast<uint64_t>(redoLogRecord->vectorNo) <<
                            " CON_ID:" << redoLogRecord->conId <<
                            " TYP:" << static_cast<uint64_t>(redoLogRecord->typ) <<
                            " CLS:" << redoLogRecord->cls <<
                            " AFN:" << redoLogRecord->afn <<
                            " DBA:0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->dba <<
                            " OBJ:" << std::dec << redoLogRecord->recordDataObj <<
                            " SCN:" << PRINTSCN64(redoLogRecord->scnRecord) <<
                            " SEQ:" << std::dec << static_cast<uint64_t>(redoLogRecord->seq) <<
                            " OP:" << static_cast<uint64_t>(redoLogRecord->opCode >> 8) << "." << static_cast<uint64_t>(redoLogRecord->opCode & 0xFF) <<
                            " ENC:" << std::dec << static_cast<uint64_t>(encrypted) <<
                            " RBL:" << std::dec << redoLogRecord->rbl <<
                            " FLG:0x" << std::setw(4) << std::hex << redoLogRecord->flgRecord << '\n';
            }
        }

        if (ctx->dumpRawData)
            dumpHex(ctx, redoLogRecord);
    }

    void OpCode::ktbRedo(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        if (fieldLength < 8)
            return;

        if (redoLogRecord->opc == 0x0A16)
            ctx->dumpStream << "index undo for leaf key operations\n";
        else if (redoLogRecord->opc == 0x0B01)
            ctx->dumpStream << "KDO undo record:\n";

        auto ktbOp = (int8_t)redoLogRecord->data[fieldPos + 0];
        uint8_t flg = redoLogRecord->data[fieldPos + 1];
        uint8_t ver = flg & 0x03;
        if (ctx->dumpRedoLog >= 1) {
            ctx->dumpStream << "KTB Redo \n";
            ctx->dumpStream << "op: 0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int32_t>(ktbOp) << " " <<
                    " ver: 0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(ver) << "  \n";
            ctx->dumpStream << "compat bit: " << std::dec << static_cast<uint64_t>(flg & 0x04) << " ";
            if ((flg & 0x04) != 0)
                ctx->dumpStream << "(post-11)";
            else
                ctx->dumpStream << "(pre-11)";

            uint64_t padding = ((flg & 0x10) != 0) ? 0 : 1;
            ctx->dumpStream << " padding: " << padding << '\n';
        }
        char opCode = '?';
        uint64_t startPos = 8;
        if ((flg & 0x08) == 0)
            startPos = 4;

        if ((ktbOp & 0x0F) == KTBOP_C) {
            opCode = 'C';

            if (fieldLength < startPos + 8)
                throw RedoLogException(50061, "too short field KTP Redo C: " + std::to_string(fieldLength) + " offset: " +
                                       std::to_string(redoLogRecord->dataOffset));

            redoLogRecord->uba = ctx->read56(redoLogRecord->data + fieldPos + startPos);

            if (ctx->dumpRedoLog >= 1) {
                ctx->dumpStream << "op: " << opCode << " " << " uba: " << PRINTUBA(redoLogRecord->uba) << '\n';
            }

        } else if ((ktbOp & 0x0F) == KTBOP_Z) {
            opCode = 'Z';

            if (ctx->dumpRedoLog >= 1) {
                ctx->dumpStream << "op: " << opCode << '\n';
            }

        } else if ((ktbOp & 0x0F) == KTBOP_L) {
            opCode = 'L';

            if (fieldLength < startPos + 24)
                throw RedoLogException(50061, "too short field KTP Redo L2: " + std::to_string(fieldLength) + " offset: " +
                                       std::to_string(redoLogRecord->dataOffset));

            redoLogRecord->uba = ctx->read56(redoLogRecord->data + fieldPos + startPos + 8);

            if (ctx->dumpRedoLog >= 1) {
                typeXid itlXid = typeXid(static_cast<typeUsn>(ctx->read16(redoLogRecord->data + fieldPos + startPos)),
                                         ctx->read16(redoLogRecord->data + fieldPos + startPos + 2),
                                         ctx->read32(redoLogRecord->data + fieldPos + startPos + 4));

                ctx->dumpStream << "op: " << opCode << " " <<
                        " itl:" <<
                        " xid:  " << itlXid.toString() <<
                        " uba: " << PRINTUBA(redoLogRecord->uba) << '\n';

                uint8_t lkc;
                uint8_t flag;
                if (ctx->isBigEndian()) {
                    lkc = redoLogRecord->data[fieldPos + startPos + 17];
                    flag = redoLogRecord->data[fieldPos + startPos + 16];
                } else {
                    lkc = redoLogRecord->data[fieldPos + startPos + 16];
                    flag = redoLogRecord->data[fieldPos + startPos + 17];
                }
                char flagStr[5] = "----";
                if ((flag & 0x10) != 0) flagStr[3] = 'T';
                if ((flag & 0x20) != 0) flagStr[2] = 'U';
                if ((flag & 0x40) != 0) flagStr[1] = 'B';
                if ((flag & 0x80) != 0) flagStr[0] = 'C';
                typeScn scnx = ctx->readScnR(redoLogRecord->data + fieldPos + startPos + 18);

                if (ctx->version < REDO_VERSION_12_2)
                    ctx->dumpStream << "                     " <<
                            " flg: " << flagStr << "   " <<
                            " lkc:  " << static_cast<uint64_t>(lkc) << "    " <<
                            " scn: " << PRINTSCN48(scnx) << '\n';
                else
                    ctx->dumpStream << "                     " <<
                            " flg: " << flagStr << "   " <<
                            " lkc:  " << static_cast<uint64_t>(lkc) << "    " <<
                            " scn:  " << PRINTSCN64(scnx) << '\n';
            }

        } else if ((ktbOp & 0x0F) == KTBOP_R) {
            opCode = 'R';

            if (ctx->dumpRedoLog >= 1) {
                int16_t itc = ctx->read16(redoLogRecord->data + fieldPos + startPos + 2);
                ctx->dumpStream << "op: " << opCode << "  itc: " << std::dec << itc << '\n';
                if (itc < 0)
                    itc = 0;

                if (fieldLength < startPos + 12 + itc * 24)
                    throw RedoLogException(50061, "too short field KTB Redo R: " + std::to_string(fieldLength) + " offset: " +
                                           std::to_string(redoLogRecord->dataOffset));

                ctx->dumpStream << " Itl           Xid                  Uba         Flag  Lck        Scn/Fsc\n";
                for (uint64_t i = 0; i < static_cast<uint64_t>(itc); ++i) {
                    typeXid itcXid = typeXid(static_cast<typeUsn>(ctx->read16(redoLogRecord->data + fieldPos + startPos + 12 + i * 24)),
                                             ctx->read16(redoLogRecord->data + fieldPos + startPos + 12 + 2 + i * 24),
                                             ctx->read32(redoLogRecord->data + fieldPos + startPos + 12 + 4 + i * 24));

                    typeUba itcUba = ctx->read56(redoLogRecord->data + fieldPos + startPos + 12 + 8 + i * 24);
                    char flagsStr[5] = "----";
                    typeScn scnfsc;
                    const char* scnfscStr = "fsc";
                    uint16_t lck = ctx->read16(redoLogRecord->data + fieldPos + startPos + 12 + 16 + i * 24);
                    if ((lck & 0x1000) != 0) flagsStr[3] = 'T';
                    if ((lck & 0x2000) != 0) flagsStr[2] = 'U';
                    if ((lck & 0x4000) != 0) flagsStr[1] = 'B';
                    if ((lck & 0x8000) != 0) {
                        flagsStr[0] = 'C';
                        scnfscStr = "scn";
                        lck = 0;
                        scnfsc = ctx->readScn(redoLogRecord->data + fieldPos + startPos + 12 + 18 + i * 24);
                    } else
                        scnfsc = (static_cast<uint64_t>(ctx->read16(redoLogRecord->data + fieldPos + startPos + 12 + 18 + i * 24)) << 32) |
                                static_cast<uint64_t>(ctx->read32(redoLogRecord->data + fieldPos + startPos + 12 + 20 + i * 24));
                    lck &= 0x0FFF;

                    ctx->dumpStream << "0x" << std::setfill('0') << std::setw(2) << std::hex << (i + 1) << "   " <<
                            itcXid.toString() << "  " <<
                            PRINTUBA(itcUba) << "  " << flagsStr << "  " <<
                            std::setfill(' ') << std::setw(3) << std::dec << static_cast<uint64_t>(lck) << "  " << scnfscStr << " " <<
                            PRINTSCN48(scnfsc) << '\n';
                }
            }

        } else if ((ktbOp & 0x0F) == KTBOP_N) {
            opCode = 'N';

            if (ctx->dumpRedoLog >= 1) {
                ctx->dumpStream << "op: " << opCode << '\n';
            }

        } else if ((ktbOp & 0x0F) == KTBOP_F) {
            opCode = 'F';

            if (fieldLength < startPos + 16)
                throw RedoLogException(50061, "too short field KTB Redo F: " + std::to_string(fieldLength) + " offset: " +
                                       std::to_string(redoLogRecord->dataOffset));

            redoLogRecord->xid = typeXid(static_cast<typeUsn>(ctx->read16(redoLogRecord->data + fieldPos + startPos)),
                                         ctx->read16(redoLogRecord->data + fieldPos + startPos + 2),
                                         ctx->read32(redoLogRecord->data + fieldPos + startPos + 4));
            redoLogRecord->uba = ctx->read56(redoLogRecord->data + fieldPos + startPos + 8);

            if (ctx->dumpRedoLog >= 1) {
                ctx->dumpStream << "op: " << opCode << " " <<
                        " xid:  " << redoLogRecord->xid.toString() <<
                        "    uba: " << PRINTUBA(redoLogRecord->uba) << '\n';
            }
        }

        // Block clean record
        if ((ktbOp & KTBOP_BLOCKCLEANOUT) != 0) {
            if (ctx->dumpRedoLog >= 1) {
                typeScn scn = ctx->readScn(redoLogRecord->data + fieldPos + startPos + 40);
                uint8_t opt = redoLogRecord->data[fieldPos + startPos + 36];
                uint8_t ver2 = redoLogRecord->data[fieldPos + startPos + 38];
                uint8_t entries = redoLogRecord->data[fieldPos + startPos + 37];

                if (ctx->version < REDO_VERSION_12_2)
                    ctx->dumpStream << "Block cleanout record, scn: " <<
                            " " << PRINTSCN48(scn) <<
                            " ver: 0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(ver2) <<
                            " opt: 0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(opt) <<
                            ", entries follow...\n";
                else {
                    char bigscn = 'N';
                    char compat = 'N';
                    if ((ver2 & 0x08) != 0)
                        bigscn = 'Y';
                    if ((ver2 & 0x04) != 0)
                        compat = 'Y';
                    uint32_t spare = 0; // TODO: find field position/size
                    ver2 &= 0x03;
                    ctx->dumpStream << "Block cleanout record, scn: " <<
                            " " << PRINTSCN64(scn) <<
                            " ver: 0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(ver2) <<
                            " opt: 0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(opt) <<
                            " bigscn: " << bigscn <<
                            " compact: " << compat <<
                            " spare: " << std::setfill('0') << std::setw(8) << std::hex << spare <<
                            ", entries follow...\n";
                }

                if (fieldLength < startPos + 48 + entries * static_cast<uint64_t>(8))
                    throw RedoLogException(50061, "too short field KTB Read F2: " + std::to_string(fieldLength) + " offset: " +
                                           std::to_string(redoLogRecord->dataOffset));

                for (uint64_t j = 0; j < entries; ++j) {
                    uint8_t itli = redoLogRecord->data[fieldPos + startPos + 48 + j * 8];
                    uint8_t flg2 = redoLogRecord->data[fieldPos + startPos + 49 + j * 8];
                    typeScn scnx = ctx->readScnR(redoLogRecord->data + fieldPos + startPos + 50 + j * 8);
                    if (ctx->version < REDO_VERSION_12_1)
                        ctx->dumpStream << "  itli: " << std::dec << static_cast<uint64_t>(itli) << " " <<
                                " flg: " << static_cast<uint64_t>(flg2) << " " <<
                                " scn: " << PRINTSCN48(scnx) << '\n';
                    else if (ctx->version < REDO_VERSION_12_2)
                        ctx->dumpStream << "  itli: " << std::dec << static_cast<uint64_t>(itli) << " " <<
                                " flg: (opt=" << static_cast<uint64_t>(flg2 & 0x03) << " whr=" << static_cast<uint64_t>(flg2 >> 2) << ") " <<
                                " scn: " << PRINTSCN48(scnx) << '\n';
                    else {
                        uint8_t opt2 = flg2 & 0x03;
                        uint8_t whr = flg2 >> 2;
                        ctx->dumpStream << "  itli: " << std::dec << static_cast<uint64_t>(itli) << " " <<
                                " flg: (opt=" << static_cast<uint64_t>(opt2) << " whr=" << static_cast<uint64_t>(whr) << ") " <<
                                " scn:  " << PRINTSCN64(scnx) << '\n';
                    }
                }
            }
        }
    }

    void OpCode::kdli(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        uint8_t code = redoLogRecord->data[fieldPos + 0];

        switch (code) {
            case KDLI_CODE_INFO:
                kdliInfo(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_LOAD_COMMON:
                kdliLoadCommon(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_LOAD_DATA:
                kdliLoadData(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_ZERO:
                kdliZero(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_FILL:
                kdliFill(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_LMAP:
                kdliLmap(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_LMAPX:
                kdliLmapx(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_SUPLOG:
                kdliSuplog(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_GMAP:
                kdliGmap(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_FPLOAD:
                kdliFpload(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_LOAD_LHB:
                kdliLoadLhb(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_ALMAP:
                kdliAlmap(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_ALMAPX:
                kdliAlmapx(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_LOAD_ITREE:
                kdliLoadItree(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_IMAP:
                kdliImap(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;

            case KDLI_CODE_IMAPX:
                kdliImapx(ctx, redoLogRecord, fieldPos, fieldLength, code);
                break;
        }
    }

    void OpCode::kdliInfo(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength, uint8_t code) {
        if (fieldLength < 17)
            throw RedoLogException(50061, "too short field kdli info: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->lobId.set(redoLogRecord->data + fieldPos + 1);

        if (ctx->dumpRedoLog >= 1) {
            typeDba block = ctx->read32Big(redoLogRecord->data + fieldPos + 11);
            uint16_t slot = ctx->read16Big(redoLogRecord->data + fieldPos + 15);

            ctx->dumpStream << "KDLI info [" << std::dec << static_cast<uint64_t>(code) << "." << fieldLength << "]\n";
            ctx->dumpStream << "  lobid " << redoLogRecord->lobId.lower() << '\n';
            ctx->dumpStream << "  block 0x" << std::setfill('0') << std::setw(8) << std::hex << block << '\n';
            ctx->dumpStream << "  slot  0x" << std::setfill('0') << std::setw(4) << std::hex << slot << '\n';
        }
    }

    void OpCode::kdliLoadCommon(Ctx* ctx, RedoLogRecord* redoLogRecord __attribute__((unused)), uint64_t& fieldPos __attribute__((unused)),
                                uint16_t& fieldLength, uint8_t code) {
        if (ctx->dumpRedoLog >= 1) {
            ctx->dumpStream << "KDLI load common [" << std::dec << static_cast<uint64_t>(code) << "." << fieldLength << "]\n";
            // TODO: finish
        }
    }

    void OpCode::kdliLoadData(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength, uint8_t code) {
        if (fieldLength < 56)
            throw RedoLogException(50061, "too short field kdli load data: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->lobId.set(redoLogRecord->data + fieldPos + 12);
        redoLogRecord->lobPageNo = INVALID_LOB_PAGE_NO;
        if (ctx->dumpRedoLog >= 1) {
            typeScn scn = ctx->readScnR(redoLogRecord->data + fieldPos + 2);
            uint8_t flg0 = redoLogRecord->data[fieldPos + 10];
            const char* flg0typ = "";
            switch (flg0 & KDLI_TYPE_MASK) {
                case KDLI_TYPE_NEW:
                    flg0typ = "new";
                    break;

                case KDLI_TYPE_LHB:
                    flg0typ = "lhb";
                    break;

                case KDLI_TYPE_DATA:
                    flg0typ = "data";
                    break;

                case KDLI_TYPE_BTREE:
                    flg0typ = "btree";
                    break;

                case KDLI_TYPE_ITREE:
                    flg0typ = "itree";
                    break;

                case KDLI_TYPE_AUX:
                    flg0typ = "aux";
                    break;
            }
            const char* flg0lock = "n";
            if (flg0 & KDLI_TYPE_LOCK)
                flg0lock = "y";
            const char* flg0ver = "0";
            if (flg0 & KDLI_TYPE_VER1)
                flg0ver = "1";
            uint8_t flg1 = redoLogRecord->data[fieldPos + 11];
            uint16_t rid1 = ctx->read16(redoLogRecord->data + fieldPos + 22);
            uint32_t rid2 = ctx->read32(redoLogRecord->data + fieldPos + 24);
            uint8_t flg2 = redoLogRecord->data[fieldPos + 28];
            const char* flg2pfill = "n";
            if (flg2 & KDLI_FLG2_121_PFILL)
                flg2pfill = "y";
            const char* flg2cmap = "n";
            if (flg2 & KDLI_FLG2_121_CMAP)
                flg2cmap = "y";
            const char* flg2hash = "n";
            if (flg2 & KDLI_FLG2_121_HASH)
                flg2hash = "y";
            const char* flg2lid = "short-rowid";
            if (flg2 & KDLI_FLG2_121_LHB)
                flg2lid = "lhb-dba";
            const char* flg2ver1 = "0";
            if (flg2 & KDLI_FLG2_121_VER1)
                flg2ver1 = "1";
            uint8_t flg3 = redoLogRecord->data[fieldPos + 29];
            uint8_t pskip = redoLogRecord->data[fieldPos + 30];
            uint8_t sskip = redoLogRecord->data[fieldPos + 31];
            uint8_t hash[20];
            memcpy(reinterpret_cast<void*>(hash),
                   reinterpret_cast<const void*>(redoLogRecord->data + fieldPos + 32), 20);
            uint16_t hwm = ctx->read16(redoLogRecord->data + fieldPos + 52);
            uint16_t spr = ctx->read16(redoLogRecord->data + fieldPos + 54);

            ctx->dumpStream << "KDLI load data [" << std::dec << static_cast<uint64_t>(code) << "." << fieldLength << "]\n";
            ctx->dumpStream << "bdba    [0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->dba << "]\n";
            ctx->dumpStream << "kdlich  [0xXXXXXXXXXXXX 0]\n";
            ctx->dumpStream << "  flg0  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg0) <<
                            " [ver=" << flg0ver << " typ=" << flg0typ << " lock=" << flg0lock << "]\n";
            ctx->dumpStream << "  flg1  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg1) << '\n';
            if (ctx->version < REDO_VERSION_12_2)
                ctx->dumpStream << "  scn   0x" << std::setfill('0') << std::setw(12) << std::hex << scn << " [0x" << PRINTSCN48(scn) << "]\n";
            else
                ctx->dumpStream << "  scn   0x" << std::setfill('0') << std::setw(16) << std::hex << (scn & 0xFFFF7FFFFFFFFFFF) <<
                                " [" << PRINTSCN64D(scn) << "]\n";
            ctx->dumpStream << "  lid   " << redoLogRecord->lobId.lower() << '\n';
            ctx->dumpStream << "  rid   0x" << std::setfill('0') << std::setw(8) << std::hex << rid2 << "." << std::setfill('0') <<
                            std::setw(4) << std::hex << rid1 << '\n';
            ctx->dumpStream << "kdlidh  [0xXXXXXXXXXXXX 24]\n";
            ctx->dumpStream << "  flg2  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg2) <<
                            " [ver=" << flg2ver1 << " lid=" << flg2lid << " hash=" << flg2hash << " cmap=" << flg2cmap << " pfill=" << flg2pfill << "]\n";
            ctx->dumpStream << "  flg3  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg3) << '\n';
            ctx->dumpStream << "  pskip " << std::dec << static_cast<uint64_t>(pskip) << '\n';
            ctx->dumpStream << "  sskip " << std::dec << static_cast<uint64_t>(sskip) << '\n';
            ctx->dumpStream << "  hash  ";
            for (uint64_t j = 0; j < 20; ++j)
                ctx->dumpStream << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(hash[j]);
            ctx->dumpStream << '\n';
            ctx->dumpStream << "  hwm   " << std::dec << hwm << '\n';
            ctx->dumpStream << "  spr   " << std::dec << spr << '\n';
        }
    }

    void OpCode::kdliZero(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength, uint8_t code) {
        if (fieldLength < 6)
            throw RedoLogException(50061, "too short field kdli zero: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        if (ctx->dumpRedoLog >= 1) {
            uint16_t zoff = ctx->read16(redoLogRecord->data + fieldPos + 2);
            uint16_t zsiz = ctx->read16(redoLogRecord->data + fieldPos + 4);

            ctx->dumpStream << "KDLI zero [" << std::dec << static_cast<uint64_t>(code) << "." << fieldLength << "]\n";
            ctx->dumpStream << "  zoff  0x" << std::setfill('0') << std::setw(4) << std::hex << zoff << '\n';
            ctx->dumpStream << "  zsiz  " << std::dec << zsiz << '\n';
        }
    }

    void OpCode::kdliFill(Ctx* ctx, RedoLogRecord* redoLogRecord __attribute__((unused)), uint64_t& fieldPos __attribute__((unused)), uint16_t& fieldLength,
                          uint8_t code) {

        redoLogRecord->indKeyDataCode = code;
        redoLogRecord->lobOffset = ctx->read16(redoLogRecord->data + fieldPos + 2);;
        redoLogRecord->lobData = fieldPos + 8;
        redoLogRecord->lobDataLength = ctx->read16(redoLogRecord->data + fieldPos + 6);

        if (ctx->dumpRedoLog >= 1) {
            uint16_t fsiz = ctx->read16(redoLogRecord->data + fieldPos + 4);

            ctx->dumpStream << "KDLI fill [" << std::dec << static_cast<uint64_t>(code) << "." << fieldLength << "]\n";
            ctx->dumpStream << "  foff  0x" << std::setfill('0') << std::setw(4) << std::hex << redoLogRecord->lobOffset << '\n';
            ctx->dumpStream << "  fsiz  " << std::dec << fsiz << '\n';
            ctx->dumpStream << "  flen  " << std::dec << redoLogRecord->lobDataLength << '\n';

            ctx->dumpStream << "  data\n";
            for (uint64_t j = 0; j < static_cast<uint64_t>(fieldLength - 8); ++j) {
                ctx->dumpStream << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(redoLogRecord->data[fieldPos + j + 8]);
                if ((j % 26) < 25)
                    ctx->dumpStream << " ";
                if ((j % 26) == 25 || j == static_cast<uint64_t>(fieldLength - 8) - 1)
                    ctx->dumpStream << '\n';
            }
        }
    }

    void OpCode::kdliLmap(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength, uint8_t code) {
        if (fieldLength < 8)
            throw RedoLogException(50061, "too short field kdli lmap: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->indKeyDataCode = code;
        redoLogRecord->indKeyData = fieldPos;
        redoLogRecord->indKeyDataLength = fieldLength;

        if (ctx->dumpRedoLog >= 1) {
            uint32_t asiz = ctx->read32(redoLogRecord->data + fieldPos + 4);

            if (fieldLength < 8 + asiz * 8)
                ctx->warning(70001, "too short field kdli lmap asiz: " + std::to_string(fieldLength) + " offset: " +
                             std::to_string(redoLogRecord->dataOffset));

            ctx->dumpStream << "KDLI lmap [" << std::dec << static_cast<uint64_t>(code) << "." << fieldLength << "]\n";
            ctx->dumpStream << "  asiz  " << std::dec << asiz << '\n';

            for (uint64_t i = 0; i < asiz; ++i) {
                uint8_t num1 = redoLogRecord->data[fieldPos + i * 8 + 8 + 0];
                uint8_t num2 = redoLogRecord->data[fieldPos + i * 8 + 8 + 1];
                uint16_t num3 = ctx->read16(redoLogRecord->data + fieldPos + i * 8 + 8 + 2);
                typeDba dba = ctx->read32(redoLogRecord->data + fieldPos + i * 8 + 8 + 4);

                ctx->dumpStream << "    [" << std::dec << i << "] " <<
                                "0x" << std::hex << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(num1) << " " <<
                                "0x" << std::hex << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(num2) << " " <<
                                std::dec << num3 << " " <<
                                "0x" << std::hex << std::setfill('0') << std::setw(8) << std::hex << dba << '\n';
            }
        }
    }

    void OpCode::kdliLmapx(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength, uint8_t code) {
        if (fieldLength < 8)
            throw RedoLogException(50061, "too short field kdli lmapx: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->indKeyDataCode = code;
        redoLogRecord->indKeyData = fieldPos;
        redoLogRecord->indKeyDataLength = fieldLength;

        if (ctx->dumpRedoLog >= 1) {
            uint32_t asiz = ctx->read32(redoLogRecord->data + fieldPos + 4);

            if (fieldLength < 8 + static_cast<uint64_t>(asiz) * 16) {
                ctx->warning(70001, "too short field kdli lmapx asiz: " + std::to_string(fieldLength) + " offset: " +
                             std::to_string(redoLogRecord->dataOffset));
                return;
            }

            ctx->dumpStream << "KDLI lmapx [" << std::dec << static_cast<uint64_t>(code) << "." << fieldLength << "]\n";
            ctx->dumpStream << "  asiz  " << std::dec << asiz << '\n';

            for (uint64_t i = 0; i < asiz; ++i) {
                uint8_t num1 = redoLogRecord->data[fieldPos + i * 16 + 8 + 0];
                uint8_t num2 = redoLogRecord->data[fieldPos + i * 16 + 8 + 1];
                uint16_t num3 = ctx->read16(redoLogRecord->data + fieldPos + i * 16 + 8 + 2);
                typeDba dba = ctx->read32(redoLogRecord->data + fieldPos + i * 16 + 8 + 4);
                int32_t num4 = ctx->read32(redoLogRecord->data + fieldPos + i * 16 + 8 + 8);
                int32_t num5 = ctx->read32(redoLogRecord->data + fieldPos + i * 16 + 8 + 12);

                ctx->dumpStream << "    [" << std::dec << i << "] " <<
                                "0x" << std::hex << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(num1) << " " <<
                                "0x" << std::hex << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(num2) << " " <<
                                std::dec << num3 << " " <<
                                "0x" << std::hex << std::setfill('0') << std::setw(8) << std::hex << dba <<
                                " " << std::dec << num4 << "." << std::dec << num5 << '\n';
            }
        }
    }

    void OpCode::kdliSuplog(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength, uint8_t code) {
        if (fieldLength < 24)
            throw RedoLogException(50061, "too short field kdli suplog: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->xid = typeXid(static_cast<typeUsn>(ctx->read16(redoLogRecord->data + fieldPos + 4)),
                                     ctx->read16(redoLogRecord->data + fieldPos + 6),
                                     ctx->read32(redoLogRecord->data + fieldPos + 8));
        redoLogRecord->obj = ctx->read32(redoLogRecord->data + fieldPos + 12);
        redoLogRecord->col = ctx->read16(redoLogRecord->data + fieldPos + 18);

        if (ctx->dumpRedoLog >= 1) {
            uint16_t objv = ctx->read16(redoLogRecord->data + fieldPos + 16);
            uint32_t flag = ctx->read32(redoLogRecord->data + fieldPos + 20);

            ctx->dumpStream << "KDLI suplog [" << std::dec << static_cast<uint64_t>(code) << "." << std::dec << fieldLength << "]\n";
            ctx->dumpStream << "  xid   " << redoLogRecord->xid.toString() << '\n';
            ctx->dumpStream << "  objn  " << std::dec << redoLogRecord->obj << '\n';
            ctx->dumpStream << "  objv# " << std::dec << objv << '\n';
            ctx->dumpStream << "  col#  " << std::dec << redoLogRecord->col << '\n';
            ctx->dumpStream << "  flag  0x" << std::setfill('0') << std::setw(8) << std::hex << flag << '\n';
        }
    }

    void OpCode::kdliGmap(Ctx* ctx, RedoLogRecord* redoLogRecord __attribute__((unused)), uint64_t& fieldPos __attribute__((unused)),
                          uint16_t& fieldLength __attribute__((unused)), uint8_t code __attribute__((unused))) {
        if (ctx->dumpRedoLog >= 1) {
            ctx->dumpStream << "KDLI GMAP Generic/Auxiliary Mapping Change:\n";
            // TODO: finish
        }
    }

    void OpCode::kdliFpload(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength, uint8_t code) {
        if (fieldLength < 28)
            throw RedoLogException(50061, "too short field kdli fpload: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->xid = typeXid(static_cast<typeUsn>(ctx->read16(redoLogRecord->data + fieldPos + 16)),
                                     ctx->read16(redoLogRecord->data + fieldPos + 18),
                                     ctx->read32(redoLogRecord->data + fieldPos + 20));
        redoLogRecord->dataObj = ctx->read32(redoLogRecord->data + fieldPos + 24);

        if (ctx->dumpRedoLog >= 1) {
            uint32_t bsz = ctx->read32(redoLogRecord->data + fieldPos + 4);
            typeScn scn = ctx->readScn(redoLogRecord->data + fieldPos + 8);

            ctx->dumpStream << "KDLI fpload [" << std::dec << static_cast<uint64_t>(code) << "." << fieldLength << "]\n";
            ctx->dumpStream << "  bsz   " << std::dec << bsz << '\n';
            if (ctx->version < REDO_VERSION_12_2)
                ctx->dumpStream << "  scn   " << PRINTSCN48(scn) << '\n';
            else
                ctx->dumpStream << "  scn   " << PRINTSCN64(scn) << '\n';
            ctx->dumpStream << "  xid   " << redoLogRecord->xid.toString() << '\n';
            ctx->dumpStream << "  objd  " << std::dec << redoLogRecord->dataObj << '\n';
        }
    }

    void OpCode::kdliLoadLhb(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength, uint8_t code) {
        if (fieldLength < 112)
            throw RedoLogException(50061, "too short field kdli load lhb: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->lobId.set(redoLogRecord->data + fieldPos + 12);
        redoLogRecord->lobPageNo = INVALID_LOB_PAGE_NO;
        redoLogRecord->dba0 = ctx->read32(redoLogRecord->data + fieldPos + 64);
        redoLogRecord->dba1 = ctx->read32(redoLogRecord->data + fieldPos + 68);
        redoLogRecord->dba2 = ctx->read32(redoLogRecord->data + fieldPos + 72);
        redoLogRecord->dba3 = ctx->read32(redoLogRecord->data + fieldPos + 76);

        if (ctx->dumpRedoLog >= 1) {
            typeScn scn = static_cast<uint64_t>(ctx->read32(redoLogRecord->data + fieldPos + 4)) |
                          (static_cast<uint64_t>(ctx->read16(redoLogRecord->data + fieldPos + 8)) << 32);
            uint8_t flg0 = redoLogRecord->data[fieldPos + 10];
            uint8_t flg1 = redoLogRecord->data[fieldPos + 11];
            uint32_t spare = ctx->read32(redoLogRecord->data + fieldPos + 24);
            const char* flg0typ = "???";
            switch (flg0 & KDLI_TYPE_MASK) {
                case KDLI_TYPE_NEW:
                    flg0typ = "new";
                    break;

                case KDLI_TYPE_LHB:
                    flg0typ = "lhb";
                    break;

                case KDLI_TYPE_DATA:
                    flg0typ = "data";
                    break;

                case KDLI_TYPE_BTREE:
                    flg0typ = "btree";
                    break;

                case KDLI_TYPE_ITREE:
                    flg0typ = "itree";
                    break;

                case KDLI_TYPE_AUX:
                    flg0typ = "aux";
                    break;
            }
            const char* flg0lock = "n";
            if (flg0 & KDLI_TYPE_LOCK)
                flg0lock = "y";
            const char* flg0ver = "0";
            if (flg0 & KDLI_TYPE_VER1)
                flg0ver = "1";

            ctx->dumpStream << "KDLI load lhb [" << std::dec << static_cast<uint64_t>(code) << "." << fieldLength << "]\n";
            ctx->dumpStream << "bdba    [0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->dba << "]\n";
            ctx->dumpStream << "kdlich  [0xXXXXXXXXXXXX 0]\n";
            ctx->dumpStream << "  flg0  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg0) <<
                            " [ver=" << flg0ver << " typ=" << flg0typ << " lock=" << flg0lock << "]\n";
            ctx->dumpStream << "  flg1  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg1) << '\n';
            if (ctx->version < REDO_VERSION_12_2)
                ctx->dumpStream << "  scn   0x" << std::setfill('0') << std::setw(12) << std::hex << scn << " [0x" << PRINTSCN48(scn) << "]\n";
            else
                ctx->dumpStream << "  scn   0x" << std::setfill('0') << std::setw(16) << std::hex << (scn & 0xFFFF7FFFFFFFFFFF) <<
                                " [" << PRINTSCN64D(scn) << "]\n";
            ctx->dumpStream << "  lid   " << redoLogRecord->lobId.lower() << '\n';
            ctx->dumpStream << "  spare 0x" << std::setfill('0') << std::setw(8) << std::hex << spare << '\n';

            uint8_t flg2 = redoLogRecord->data[fieldPos + 28];
            uint8_t flg3 = redoLogRecord->data[fieldPos + 29];
            uint8_t flg4 = redoLogRecord->data[fieldPos + 30];
            uint8_t flg5 = redoLogRecord->data[fieldPos + 31];
            int32_t llen1 = ctx->read32(redoLogRecord->data + fieldPos + 32);
            int32_t llen2 = ctx->read32(redoLogRecord->data + fieldPos + 36);
            int32_t ver1 = ctx->read32(redoLogRecord->data + fieldPos + 40);
            int32_t ver2 = ctx->read32(redoLogRecord->data + fieldPos + 44);
            int32_t ext = ctx->read32(redoLogRecord->data + fieldPos + 48);
            uint16_t asiz = ctx->read16(redoLogRecord->data + fieldPos + 52);
            uint16_t hwm = ctx->read16(redoLogRecord->data + fieldPos + 54);
            uint32_t ovr1 = ctx->read32(redoLogRecord->data + fieldPos + 56);
            int32_t ovr2 = ctx->read32(redoLogRecord->data + fieldPos + 60);
            typeDba ldba = ctx->read32(redoLogRecord->data + fieldPos + 80);
            int32_t nblk = ctx->read32(redoLogRecord->data + fieldPos + 84);
            typeScn deScn1 = 0;
            typeScn deScn2 = ctx->read64(redoLogRecord->data + fieldPos + 88);
            uint8_t hash[16];
            memcpy(reinterpret_cast<void*>(hash),
                   reinterpret_cast<const void*>(redoLogRecord->data + fieldPos + 96), 16);

            if (flg3 & KDLI_FLG3_VLL) {
                ctx->dumpStream << "kdlihh  [0xXXXXXXXXXXXX 24]\n";

                if (ctx->version < REDO_VERSION_12_2) {
                    const char* flg2pfill = "n";
                    if (flg2 & KDLI_FLG2_121_PFILL)
                        flg2pfill = "y";
                    const char* flg2cmap = "n";
                    if (flg2 & KDLI_FLG2_121_CMAP)
                        flg2cmap = "y";
                    const char* flg2hash = "n";
                    if (flg2 & KDLI_FLG2_121_HASH)
                        flg2hash = "y";
                    const char* flg2lid = "short-rowid";
                    if (flg2 & KDLI_FLG2_121_LHB)
                        flg2lid = "lhb-dba";
                    const char* flg2ver1 = "0";
                    if (flg2 & KDLI_FLG2_121_VER1)
                        flg2ver1 = "1";

                    ctx->dumpStream << "  flg2  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg2) <<
                                    " [ver=" << flg2ver1 << " lid=" << flg2lid << " hash=" << flg2hash << " cmap=" << flg2cmap << " pfill=" <<
                                    flg2pfill << "]\n";
                } else {
                    const char* flg2descn = "n";
                    if (flg2 & KDLI_FLG2_122_DESCN)
                        flg2descn = "y";
                    const char* flg2ovr = "n";
                    if (flg2 & KDLI_FLG2_122_OVR)
                        flg2ovr = "y";
                    const char* flg2xfm = "n";
                    if (flg2 & KDLI_FLG2_122_XFM)
                        flg2xfm = "y";
                    const char* flg2bt = "n";
                    if (flg2 & KDLI_FLG2_122_BT)
                        flg2bt = "y";
                    const char* flg2it = "n";
                    if (flg2 & KDLI_FLG2_122_IT)
                        flg2it = "y";
                    const char* flg2hash = "n";
                    if (flg2 & KDLI_FLG2_122_HASH)
                        flg2hash = "y";
                    const char* flg2lid = "short-rowid";
                    if (flg2 & KDLI_FLG2_122_LID)
                        flg2lid = "iot-guess";
                    const char* flg2ver1 = "0";
                    if (flg2 & KDLI_FLG2_121_VER1)
                        flg2ver1 = "1";

                    ctx->dumpStream << "  flg2  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg2) <<
                                    " [ver=" << flg2ver1 << " lid=" << flg2lid << " hash=" << flg2hash << " it=" << flg2it << " bt=" <<
                                    flg2bt << " xfm=" << flg2xfm << " ovr=" << flg2ovr << " descn=" << flg2descn << "]\n";
                }

                const char* flg3vll = "n";
                if (flg3 & KDLI_FLG3_VLL)
                    flg3vll = "y";
                ctx->dumpStream << "  flg3  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg3) <<
                                " [vll=" << flg3vll << "]\n";

                ctx->dumpStream << "  flg4  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg4) << '\n';
                ctx->dumpStream << "  flg5  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg5) << '\n';
                ctx->dumpStream << "  hash  ";
                for (uint64_t j = 0; j < 16; ++j)
                    ctx->dumpStream << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(hash[j]);
                ctx->dumpStream << '\n';
                ctx->dumpStream << "  llen  " << std::dec << llen1 << "." << llen2 << '\n';
                ctx->dumpStream << "  ver   " << std::dec << ver1 << "." << ver2 << '\n';
                ctx->dumpStream << "  #ext  " << ext << '\n';
                ctx->dumpStream << "  asiz  " << std::dec << asiz << '\n';
                ctx->dumpStream << "  hwm   " << std::dec << hwm << '\n';
                ctx->dumpStream << "  ovr   0x" << std::setfill('0') << std::setw(8) << std::hex << ovr1 << "." << std::dec << ovr2 << '\n';
                if (ctx->version < REDO_VERSION_12_2)
                    ctx->dumpStream << "  descn 0x" << std::setfill('0') << std::setw(12) << std::hex << deScn1 << " [0x" << PRINTSCN48(deScn2) << "]\n";
                else
                    ctx->dumpStream << "  descn 0x" << std::setfill('0') << std::setw(16) << std::hex << deScn1 << " [" << PRINTSCN64D(deScn2) << "]\n";
                ctx->dumpStream << "  dba0  0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->dba0 << '\n';
                ctx->dumpStream << "  dba1  0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->dba1 << '\n';
                ctx->dumpStream << "  dba2  0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->dba2 << '\n';
                ctx->dumpStream << "  dba3  0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->dba3 << '\n';
                ctx->dumpStream << "  ldba  0x" << std::setfill('0') << std::setw(8) << std::hex << ldba << '\n';
                ctx->dumpStream << "  nblk  " << std::dec << nblk << '\n';
            } else {
                // TODO: finish
                ctx->dumpStream << "kdlihho [0xXXXXXXXXXXXX 24]\n";
                ctx->dumpStream << "  flg2  0x00 [ver=0 lid=short-rowid hash=n plen=n root=n xfm=n ovr=n aux=n]\n";
                ctx->dumpStream << "  flg3  0x00\n";
                ctx->dumpStream << "  flg4  0x00\n";
                ctx->dumpStream << "  flg5  0x00\n";
                ctx->dumpStream << "  hash  0000000000000000000000000000000000000000\n";
                ctx->dumpStream << "  llen  0.0\n";
                ctx->dumpStream << "  plen  0.0\n";
                ctx->dumpStream << "  ver   0.0\n";
                ctx->dumpStream << "  #ext  0.0\n";
                ctx->dumpStream << "  ovr   0x00000000.0\n";
                ctx->dumpStream << "  asiz  0\n";
                ctx->dumpStream << "  root  0x00000000\n";
                ctx->dumpStream << "  roff  0.0\n";
                ctx->dumpStream << "  auxp  0x00000000\n";
            }
        }
    }

    void OpCode::kdliAlmap(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength, uint8_t code) {
        if (fieldLength < 12)
            throw RedoLogException(50061, "too short field kdli kmap: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->indKeyDataCode = code;
        redoLogRecord->indKeyData = fieldPos;
        redoLogRecord->indKeyDataLength = fieldLength;

        if (ctx->dumpRedoLog >= 1) {
            uint32_t nent = ctx->read32(redoLogRecord->data + fieldPos + 4);
            uint32_t sidx = ctx->read32(redoLogRecord->data + fieldPos + 8);

            if (fieldLength < 12 + nent * 8)
                throw RedoLogException(50061, "too short field kdli almap nent: " + std::to_string(fieldLength) + " offset: " +
                                       std::to_string(redoLogRecord->dataOffset));

            ctx->dumpStream << "KDLI almap [" << std::dec << static_cast<uint64_t>(code) << "." << fieldLength << "]\n";
            ctx->dumpStream << "  nent  " << std::dec << nent << '\n';
            ctx->dumpStream << "  sidx  " << std::dec << sidx << '\n';

            for (uint64_t i = 0; i < nent; ++i) {
                uint8_t num1 = redoLogRecord->data[fieldPos + i * 8 + 12 + 0];
                uint8_t num2 = redoLogRecord->data[fieldPos + i * 8 + 12 + 1];
                uint16_t num3 = ctx->read16(redoLogRecord->data + fieldPos + i * 8 + 12 + 2);
                typeDba dba = ctx->read32(redoLogRecord->data + fieldPos + i * 8 + 12 + 4);

                ctx->dumpStream << "    [" << std::dec << i << "] " <<
                                "0x" << std::hex << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(num1) << " " <<
                                "0x" << std::hex << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(num2) << " " <<
                                std::dec << num3 << " " <<
                                "0x" << std::hex << std::setfill('0') << std::setw(8) << std::hex << dba << '\n';
            }
        }
    }

    void OpCode::kdliAlmapx(Ctx* ctx, RedoLogRecord* redoLogRecord __attribute__((unused)), uint64_t& fieldPos __attribute__((unused)), uint16_t& fieldLength,
                            uint8_t code) {
        if (ctx->dumpRedoLog >= 1) {
            ctx->dumpStream << "KDLI almapx [" << std::dec << static_cast<uint64_t>(code) << "." << fieldLength << "]\n";
            // TODO: finish
        }
    }

    void OpCode::kdliLoadItree(Ctx* ctx, RedoLogRecord* redoLogRecord __attribute__((unused)), uint64_t& fieldPos __attribute__((unused)),
                               uint16_t& fieldLength, uint8_t code) {
        if (fieldLength < 40)
            throw RedoLogException(50061, "too short field kdli load itree: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->lobId.set(redoLogRecord->data + fieldPos + 12);
        redoLogRecord->lobPageNo = INVALID_LOB_PAGE_NO;

        if (ctx->dumpRedoLog >= 1) {
            typeScn scn = ctx->readScnR(redoLogRecord->data + fieldPos + 2);
            uint8_t flg0 = redoLogRecord->data[fieldPos + 10];
            const char* flg0typ = "";
            switch (flg0 & KDLI_TYPE_MASK) {
                case KDLI_TYPE_NEW:
                    flg0typ = "new";
                    break;

                case KDLI_TYPE_LHB:
                    flg0typ = "lhb";
                    break;

                case KDLI_TYPE_DATA:
                    flg0typ = "data";
                    break;

                case KDLI_TYPE_BTREE:
                    flg0typ = "btree";
                    break;

                case KDLI_TYPE_ITREE:
                    flg0typ = "itree";
                    break;

                case KDLI_TYPE_AUX:
                    flg0typ = "aux";
                    break;
            }
            const char* flg0lock = "n";
            if (flg0 & KDLI_TYPE_LOCK)
                flg0lock = "y";
            const char* flg0ver = "0";
            if (flg0 & KDLI_TYPE_VER1)
                flg0ver = "1";
            uint8_t flg1 = redoLogRecord->data[fieldPos + 11];
            uint16_t rid1 = ctx->read16(redoLogRecord->data + fieldPos + 22);
            uint32_t rid2 = ctx->read32(redoLogRecord->data + fieldPos + 24);
            uint8_t flg2 = redoLogRecord->data[fieldPos + 28];
            const char* flg2xfm = "n";
            if (flg2 & KDLI_FLG2_122_XFM)
                flg2xfm = "y";
            const char* flg2ver1 = "0";
            if (flg2 & KDLI_FLG2_121_VER1)
                flg2ver1 = "1";
            uint8_t flg3 = redoLogRecord->data[fieldPos + 29];
            uint16_t lvl = ctx->read16(redoLogRecord->data + fieldPos + 30);
            uint16_t asiz = ctx->read16(redoLogRecord->data + fieldPos + 32);
            uint16_t hwm = ctx->read16(redoLogRecord->data + fieldPos + 34);
            uint16_t par = ctx->read32(redoLogRecord->data + fieldPos + 36);

            ctx->dumpStream << "KDLI load itree [" << std::dec << static_cast<uint64_t>(code) << "." << fieldLength << "]\n";
            ctx->dumpStream << "bdba    [0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->dba << "]\n";
            ctx->dumpStream << "kdlich  [0xXXXXXXXXXXXX 0]\n";
            ctx->dumpStream << "  flg0  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg0) <<
                            " [ver=" << flg0ver << " typ=" << flg0typ << " lock=" << flg0lock << "]\n";
            ctx->dumpStream << "  flg1  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg1) << '\n';
            if (ctx->version < REDO_VERSION_12_2)
                ctx->dumpStream << "  scn   0x" << std::setfill('0') << std::setw(12) << std::hex << scn << '\n';
            else
                ctx->dumpStream << "  scn   0x" << std::setfill('0') << std::setw(16) << std::hex << (scn & 0xFFFF7FFFFFFFFFFF) <<
                                " [" << PRINTSCN64D(scn) << "]\n";
            ctx->dumpStream << "  lid   " << redoLogRecord->lobId.lower() << '\n';
            ctx->dumpStream << "  rid   0x" << std::setfill('0') << std::setw(8) << std::hex << rid2 << "." << std::setfill('0') <<
                            std::setw(4) << std::hex << rid1 << '\n';

            ctx->dumpStream << "kdliih  [0xXXXXXXXXXXXX 24]\n";
            ctx->dumpStream << "  flg2  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg2) <<
                            " [ver=" << flg2ver1 << " xfm=" << flg2xfm << "]\n";
            ctx->dumpStream << "  flg3  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg3) << '\n';
            ctx->dumpStream << "  lvl   " << std::dec << lvl << '\n';
            ctx->dumpStream << "  asiz  " << std::dec << asiz << '\n';
            ctx->dumpStream << "  hwm   " << std::dec << hwm << '\n';
            ctx->dumpStream << "  par   0x" << std::setfill('0') << std::setw(8) << std::hex << par << '\n';
        }
    }

    void OpCode::kdliImap(Ctx* ctx, RedoLogRecord* redoLogRecord __attribute__((unused)), uint64_t& fieldPos __attribute__((unused)), uint16_t& fieldLength,
                          uint8_t code) {

        redoLogRecord->indKeyDataCode = code;
        redoLogRecord->indKeyData = fieldPos;
        redoLogRecord->indKeyDataLength = fieldLength;

        if (ctx->dumpRedoLog >= 1) {
            uint32_t asiz = ctx->read32(redoLogRecord->data + fieldPos + 4);

            if (fieldLength < 8 + asiz * 8)
                ctx->warning(70001, "too short field kdli imap asiz: " + std::to_string(fieldLength) + " offset: " +
                                    std::to_string(redoLogRecord->dataOffset));

            ctx->dumpStream << "KDLI imap [" << std::dec << static_cast<uint64_t>(code) << "." << fieldLength << "]\n";
            ctx->dumpStream << "  asiz  " << std::dec << asiz << '\n';

            for (uint64_t i = 0; i < asiz; ++i) {
                uint8_t num1 = redoLogRecord->data[fieldPos + i * 8 + 8 + 0];
                uint8_t num2 = redoLogRecord->data[fieldPos + i * 8 + 8 + 1];
                uint16_t num3 = ctx->read16(redoLogRecord->data + fieldPos + i * 8 + 8 + 2);
                typeDba dba = ctx->read32(redoLogRecord->data + fieldPos + i * 8 + 8 + 4);

                ctx->dumpStream << "    [" << std::dec << i << "] " <<
                                "0x" << std::hex << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(num1) << " " <<
                                "0x" << std::hex << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(num2) << " " <<
                                std::dec << num3 << " " <<
                                "0x" << std::hex << std::setfill('0') << std::setw(8) << std::hex << dba << '\n';
            }
        }
    }

    void OpCode::kdliImapx(Ctx* ctx, RedoLogRecord* redoLogRecord __attribute__((unused)), uint64_t& fieldPos __attribute__((unused)), uint16_t& fieldLength,
                           uint8_t code) {
        if (ctx->dumpRedoLog >= 1) {
            ctx->dumpStream << "KDLI imap [" << std::dec << static_cast<uint64_t>(code) << "." << fieldLength << "]\n";
            // TODO: finish
        }
    }

    void OpCode::kdliDataLoad(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        redoLogRecord->lobData = fieldPos;
        redoLogRecord->lobDataLength = fieldLength;

        if (ctx->dumpRedoLog >= 1) {
            ctx->dumpStream << "KDLI data load [0xXXXXXXXXXXXX." << std::dec << fieldLength << "]\n";

            for (uint64_t j = 0; j < fieldLength; ++j) {
                ctx->dumpStream << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(redoLogRecord->data[fieldPos + j]);
                if ((j % 26) < 25)
                    ctx->dumpStream << " ";
                if ((j % 26) == 25 || j == static_cast<uint64_t>(fieldLength) - 1)
                    ctx->dumpStream << '\n';
            }
        }
    }

    void OpCode::kdliCommon(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        if (fieldLength < 12)
            throw RedoLogException(50061, "too short field kdli common: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->opc = redoLogRecord->data[fieldPos + 0];
        redoLogRecord->dba = ctx->read32(redoLogRecord->data + fieldPos + 8);

        if (ctx->dumpRedoLog >= 1) {
            const char* opCode = "????";
            switch (redoLogRecord->opc) {
                case KDLI_OP_REDO:
                    opCode = "REDO";
                    break;

                case KDLI_OP_UNDO:
                    opCode = "UNDO";
                    break;

                case KDLI_OP_CR:
                    opCode = "CR";
                    break;

                case KDLI_OP_FRMT:
                    opCode = "FRMT";
                    break;

                case KDLI_OP_INVL:
                    opCode = "INVL";
                    break;

                case KDLI_OP_LOAD:
                    opCode = "LOAD";
                    break;

                case KDLI_OP_BIMG:
                    opCode = "BIMG";
                    break;

                case KDLI_OP_SINV:
                    opCode = "SINV";
                    break;
            }
            uint8_t type = redoLogRecord->data[fieldPos + 1];
            const char* typeCode = "???";
            switch (type & KDLI_TYPE_MASK) {
                case KDLI_TYPE_NEW:
                    typeCode = "new";
                    break;

                case KDLI_TYPE_LHB:
                    typeCode = "lhb";
                    break;

                case KDLI_TYPE_DATA:
                    typeCode = "data";
                    break;

                case KDLI_TYPE_BTREE:
                    typeCode = "btree";
                    break;

                case KDLI_TYPE_ITREE:
                    typeCode = "itree";
                    break;

                case KDLI_TYPE_AUX:
                    typeCode = "aux";
                    break;
            }

            uint8_t flg0 = redoLogRecord->data[fieldPos + 2];
            uint8_t flg1 = redoLogRecord->data[fieldPos + 3];
            uint16_t psiz = ctx->read32(redoLogRecord->data + fieldPos + 4);
            uint16_t poff = ctx->read32(redoLogRecord->data + fieldPos + 6);

            ctx->dumpStream << "KDLI common [" << std::dec << fieldLength << "]\n";
            ctx->dumpStream << "  op    0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(redoLogRecord->opc) <<
                            " [" << opCode << "]\n";
            ctx->dumpStream << "  type  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(type) <<
                            " [" << typeCode << "]\n";
            ctx->dumpStream << "  flg0  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg0) << '\n';
            ctx->dumpStream << "  flg1  0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flg1) << '\n';
            ctx->dumpStream << "  psiz  " << std::dec << psiz << '\n';
            ctx->dumpStream << "  poff  " << std::dec << poff << '\n';
            ctx->dumpStream << "  dba   0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->dba << '\n';
        }
    }

    void OpCode::kdoOpCodeIRP(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        if (fieldLength < 48)
            throw RedoLogException(50061, "too short field kdo OpCode IRP: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->fb = redoLogRecord->data[fieldPos + 16];
        redoLogRecord->cc = redoLogRecord->data[fieldPos + 18];
        redoLogRecord->sizeDelt = ctx->read16(redoLogRecord->data + fieldPos + 40);
        redoLogRecord->slot = ctx->read16(redoLogRecord->data + fieldPos + 42);

        typeDba nridBdba = 0;
        typeSlot nridSlot = 0;
        if ((redoLogRecord->fb & FB_L) == 0) {
            nridBdba = ctx->read32(redoLogRecord->data + fieldPos + 28);
            nridSlot = ctx->read16(redoLogRecord->data + fieldPos + 32);
        }

        if (fieldLength < 45 + (static_cast<uint64_t>(redoLogRecord->cc) + 7) / 8)
            throw RedoLogException(50061, "too short field kdo OpCode IRP for nulls: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->nullsDelta = fieldPos + 45;
        uint8_t* nulls = redoLogRecord->data + redoLogRecord->nullsDelta;
        uint8_t bits = 1;
        for (uint64_t i = 0; i < static_cast<uint64_t>(redoLogRecord->cc); ++i) {
            if ((*nulls & bits) == 0)
                redoLogRecord->ccData = i + 1;
            bits <<= 1;
            if (bits == 0) {
                bits = 1;
                ++nulls;
            }
        }

        if (ctx->dumpRedoLog >= 1) {
            uint8_t tabn = redoLogRecord->data[fieldPos + 44];

            ctx->dumpStream << "tabn: " << static_cast<uint64_t>(tabn) <<
                    " slot: " << std::dec << static_cast<uint64_t>(redoLogRecord->slot) << "(0x" << std::hex << redoLogRecord->slot << ")" <<
                    " size/delt: " << std::dec << redoLogRecord->sizeDelt << '\n';

            char fbStr[9] = "--------";
            processFbFlags(redoLogRecord->fb, fbStr);
            uint8_t lb = redoLogRecord->data[fieldPos + 17];

            ctx->dumpStream << "fb: " << fbStr <<
                    " lb: 0x" << std::hex << static_cast<uint64_t>(lb) << " " <<
                    " cc: " << std::dec << static_cast<uint64_t>(redoLogRecord->cc);
            if (fbStr[1] == 'C') {
                uint8_t cki = redoLogRecord->data[fieldPos + 19];
                ctx->dumpStream << " cki: " << std::dec << static_cast<uint64_t>(cki) << '\n';
            } else
                ctx->dumpStream << '\n';

            if ((redoLogRecord->fb & FB_F) != 0  && (redoLogRecord->fb & FB_H) == 0) {
                typeDba hrid1 = ctx->read32(redoLogRecord->data + fieldPos + 20);
                typeSlot hrid2 = ctx->read16(redoLogRecord->data + fieldPos + 24);
                ctx->dumpStream << "hrid: 0x" << std::setfill('0') << std::setw(8) << std::hex << hrid1 << "." << std::hex << hrid2 << '\n';
            }

            // Next DBA/SLT
            if ((redoLogRecord->fb & FB_L) == 0) {
                ctx->dumpStream << "nrid:  0x" << std::setfill('0') << std::setw(8) << std::hex << nridBdba << "." << std::hex << nridSlot << '\n';
            }

            if ((redoLogRecord->fb & FB_K) != 0) {
                uint8_t curc = 0; // TODO: find field position/size
                uint8_t comc = 0; // TODO: find field position/size
                uint32_t pk = ctx->read32(redoLogRecord->data + fieldPos + 20);
                uint16_t pk1 = ctx->read16(redoLogRecord->data + fieldPos + 24);
                uint32_t nk = ctx->read32(redoLogRecord->data + fieldPos + 28);
                uint16_t nk1 = ctx->read16(redoLogRecord->data + fieldPos + 32);

                ctx->dumpStream << "curc: " << std::dec << static_cast<uint64_t>(curc) <<
                        " comc: " << std::dec << static_cast<uint64_t>(comc) <<
                        " pk: 0x" << std::setfill('0') << std::setw(8) << std::hex << pk << "." << std::hex << pk1 <<
                        " nk: 0x" << std::setfill('0') << std::setw(8) << std::hex << nk << "." << std::hex << nk1 << '\n';
            }

            ctx->dumpStream << "null:";
            if (redoLogRecord->cc >= 11)
                ctx->dumpStream << "\n01234567890123456789012345678901234567890123456789012345678901234567890123456789\n";
            else
                ctx->dumpStream << " ";

            nulls = redoLogRecord->data + redoLogRecord->nullsDelta;
            bits = 1;
            for (uint64_t i = 0; i < redoLogRecord->cc; ++i) {

                if ((*nulls & bits) != 0)
                    ctx->dumpStream << "N";
                else
                    ctx->dumpStream << "-";
                if ((i % 80) == 79 && i < redoLogRecord->cc)
                    ctx->dumpStream << '\n';

                bits <<= 1;
                if (bits == 0) {
                    bits = 1;
                    ++nulls;
                }
            }
            ctx->dumpStream << '\n';
        }
    }

    void OpCode::kdoOpCodeDRP(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        if (fieldLength < 20)
            throw RedoLogException(50061, "too short field kdo OpCode DRP: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->slot = ctx->read16(redoLogRecord->data + fieldPos + 16);

        if (ctx->dumpRedoLog >= 1) {
            uint8_t tabn = redoLogRecord->data[fieldPos + 18];

            ctx->dumpStream << "tabn: " << static_cast<uint64_t>(tabn) <<
                    " slot: " << std::dec << static_cast<uint64_t>(redoLogRecord->slot) << "(0x" << std::hex << redoLogRecord->slot << ")\n";
        }
    }

    void OpCode::kdoOpCodeLKR(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        if (fieldLength < 20)
            throw RedoLogException(50061, "too short field KDO OpCode LKR: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->slot = ctx->read16(redoLogRecord->data + fieldPos + 16);

        if (ctx->dumpRedoLog >= 1) {
            uint8_t tabn = redoLogRecord->data[fieldPos + 18];
            uint8_t to = redoLogRecord->data[fieldPos + 19];

            ctx->dumpStream << "tabn: " << static_cast<uint64_t>(tabn) <<
                " slot: " << std::dec << redoLogRecord->slot <<
                " to: " << std::dec << static_cast<uint64_t>(to) << '\n';
        }
    }

    void OpCode::kdoOpCodeURP(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        if (fieldLength < 28)
            throw RedoLogException(50061, "too short field kdo OpCode URP: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->fb = redoLogRecord->data[fieldPos + 16];
        redoLogRecord->slot = ctx->read16(redoLogRecord->data + fieldPos + 20);
        redoLogRecord->cc = redoLogRecord->data[fieldPos + 23];

        if (fieldLength < 26 + (static_cast<uint64_t>(redoLogRecord->cc) + 7) / 8)
            throw RedoLogException(50061, "too short field kdo OpCode URP for nulls: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->nullsDelta = fieldPos + 26;
        uint8_t* nulls = redoLogRecord->data + redoLogRecord->nullsDelta;
        uint8_t bits = 1;
        for (uint64_t i = 0; i < static_cast<uint64_t>(redoLogRecord->cc); ++i) {
            if ((*nulls & bits) == 0)
                redoLogRecord->ccData = i + 1;
            bits <<= 1;
            if (bits == 0) {
                bits = 1;
                ++nulls;
            }
        }

        if (ctx->dumpRedoLog >= 1) {
            uint8_t lock = redoLogRecord->data[fieldPos + 17];
            uint8_t ckix = redoLogRecord->data[fieldPos + 18];
            uint8_t tabn = redoLogRecord->data[fieldPos + 19];
            uint8_t ncol = redoLogRecord->data[fieldPos + 22];
            auto size = (int16_t)ctx->read16(redoLogRecord->data + fieldPos + 24); // Signed

            ctx->dumpStream << "tabn: " << static_cast<uint64_t>(tabn) <<
                    " slot: " << std::dec << redoLogRecord->slot << "(0x" << std::hex << redoLogRecord->slot << ")" <<
                    " flag: 0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(redoLogRecord->fb) <<
                    " lock: " << std::dec << static_cast<uint64_t>(lock) <<
                    " ckix: " << std::dec << static_cast<uint64_t>(ckix) << '\n';
            ctx->dumpStream << "ncol: " << std::dec << static_cast<uint64_t>(ncol) <<
                    " nnew: " << std::dec << static_cast<uint64_t>(redoLogRecord->cc) <<
                    " size: " << std::dec << size << '\n';
        }
    }

    void OpCode::kdoOpCodeCFA(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        if (fieldLength < 32)
            throw RedoLogException(50061, "too short field kdo OpCode ORP: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->slot = ctx->read16(redoLogRecord->data + fieldPos + 24);

        if (ctx->dumpRedoLog >= 1) {
            typeDba nridBdba = ctx->read32(redoLogRecord->data + fieldPos + 16);
            typeSlot nridSlot = ctx->read16(redoLogRecord->data + fieldPos + 20);
            uint8_t flag = redoLogRecord->data[fieldPos + 26];
            uint8_t tabn = redoLogRecord->data[fieldPos + 27];
            uint8_t lock = redoLogRecord->data[fieldPos + 28];
            ctx->dumpStream <<
                    "tabn: " << std::dec << static_cast<uint64_t>(tabn) <<
                    " slot: " << std::dec << redoLogRecord->slot << "(0x" << std::hex << redoLogRecord->slot << ")" <<
                    " flag: 0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(flag) << '\n' <<
                    "lock: " << std::dec << static_cast<uint64_t>(lock) <<
                    " nrid: 0x" << std::setfill('0') << std::setw(8) << std::hex << nridBdba << "." << std::hex << nridSlot << '\n';
        }
    }

    void OpCode::kdoOpCodeSKL(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        if (fieldLength < 20)
            throw RedoLogException(50061, "too short field kdo OpCode SKL: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->slot = redoLogRecord->data[fieldPos + 27];

        if (ctx->dumpRedoLog >= 1) {
            char flagStr[3] = "--";
            uint8_t lock = redoLogRecord->data[fieldPos + 29];
            uint8_t flag = redoLogRecord->data[fieldPos + 28];
            if ((flag & 0x01) != 0) flagStr[0] = 'F';
            if ((flag & 0x02) != 0) flagStr[1] = 'B';

            ctx->dumpStream << "flag: " << flagStr <<
                    " lock: " << std::dec << static_cast<uint64_t>(lock) <<
                    " slot: " << std::dec << redoLogRecord->slot << "(0x" << std::hex << redoLogRecord->slot << ")\n";

            if ((flag & 0x01) != 0) {
                uint8_t fwd[4];
                uint16_t fwd2 = ctx->read16(redoLogRecord->data + fieldPos + 20);
                memcpy(reinterpret_cast<void*>(fwd),
                       reinterpret_cast<const void*>(redoLogRecord->data + fieldPos + 16), 4);
                ctx->dumpStream << "fwd: 0x" <<
                        std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(fwd[0]) <<
                        std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(fwd[1]) <<
                        std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(fwd[2]) <<
                        std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(fwd[3]) << "." <<
                        std::dec << fwd2 << " \n";
            }

            if ((flag & 0x02) != 0) {
                uint8_t bkw[4];
                uint16_t bkw2 = ctx->read16(redoLogRecord->data + fieldPos + 26);
                memcpy(reinterpret_cast<void*>(bkw),
                       reinterpret_cast<const void*>(redoLogRecord->data + fieldPos + 22), 4);
                ctx->dumpStream << "bkw: 0x" <<
                        std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(bkw[0]) <<
                        std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(bkw[1]) <<
                        std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(bkw[2]) <<
                        std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(bkw[3]) << "." <<
                        std::dec << bkw2 << '\n';
            }
        }
    }

    void OpCode::kdoOpCodeORP(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        if (fieldLength < 48)
            throw RedoLogException(50061, "too short field kdo OpCode ORP: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->fb = redoLogRecord->data[fieldPos + 16];
        redoLogRecord->cc = redoLogRecord->data[fieldPos + 18];
        redoLogRecord->slot = ctx->read16(redoLogRecord->data + fieldPos + 42);

        if (fieldLength < 45 + (static_cast<uint64_t>(redoLogRecord->cc) + 7) / 8)
            throw RedoLogException(50061, "too short field kdo OpCode ORP for nulls: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->nullsDelta = fieldPos + 45;
        uint8_t* nulls = redoLogRecord->data + redoLogRecord->nullsDelta;
        uint8_t bits = 1;
        for (uint64_t i = 0; i < static_cast<uint64_t>(redoLogRecord->cc); ++i) {
            if ((*nulls & bits) == 0)
                redoLogRecord->ccData = i + 1;
            bits <<= 1;
            if (bits == 0) {
                bits = 1;
                ++nulls;
            }
        }

        typeDba nridBdba = 0;
        typeSlot nridSlot = 0;
        if ((redoLogRecord->fb & FB_L) == 0) {
            nridBdba = ctx->read32(redoLogRecord->data + fieldPos + 28);
            nridSlot = ctx->read16(redoLogRecord->data + fieldPos + 32);
        }
        redoLogRecord->sizeDelt = ctx->read16(redoLogRecord->data + fieldPos + 40);

        if (ctx->dumpRedoLog >= 1) {
            uint8_t tabn = redoLogRecord->data[fieldPos + 44];

            ctx->dumpStream << "tabn: " << static_cast<uint64_t>(tabn) <<
                " slot: " << std::dec << static_cast<uint64_t>(redoLogRecord->slot) <<
                "(0x" << std::hex << static_cast<uint64_t>(redoLogRecord->slot) << ")" <<
                " size/delt: " << std::dec << redoLogRecord->sizeDelt << '\n';

            char fbStr[9] = "--------";
            processFbFlags(redoLogRecord->fb, fbStr);
            uint8_t lb = redoLogRecord->data[fieldPos + 17];

            ctx->dumpStream << "fb: " << fbStr <<
                    " lb: 0x" << std::hex << static_cast<uint64_t>(lb) << " " <<
                    " cc: " << std::dec << static_cast<uint64_t>(redoLogRecord->cc);
            if (fbStr[1] == 'C') {
                uint8_t cki = redoLogRecord->data[fieldPos + 19];
                ctx->dumpStream << " cki: " << std::dec << static_cast<uint64_t>(cki) << '\n';
            } else
                ctx->dumpStream << '\n';

            if ((redoLogRecord->fb & FB_L) == 0) {
                ctx->dumpStream << "nrid:  0x" << std::setfill('0') << std::setw(8) << std::hex << nridBdba << "." << std::hex << nridSlot << '\n';
            }

            ctx->dumpStream << "null:";
            if (redoLogRecord->cc >= 11)
                ctx->dumpStream << "\n01234567890123456789012345678901234567890123456789012345678901234567890123456789\n";
            else
                ctx->dumpStream << " ";

            nulls = redoLogRecord->data + redoLogRecord->nullsDelta;
            bits = 1;
            for (uint64_t i = 0; i < redoLogRecord->cc; ++i) {

                if ((*nulls & bits) != 0)
                    ctx->dumpStream << "N";
                else
                    ctx->dumpStream << "-";
                if ((i % 80) == 79 && i < redoLogRecord->cc)
                    ctx->dumpStream << '\n';

                bits <<= 1;
                if (bits == 0) {
                    bits = 1;
                    ++nulls;
                }
            }
            ctx->dumpStream << '\n';
        }
    }

    void OpCode::kdoOpCodeQM(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        if (fieldLength < 24)
            throw RedoLogException(50061, "too short field kdo OpCode QMI (1): " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->nRow = redoLogRecord->data[fieldPos + 18];
        redoLogRecord->slotsDelta = fieldPos + 20;

        if (ctx->dumpRedoLog >= 1) {
            uint8_t tabn = redoLogRecord->data[fieldPos + 16];
            uint8_t lock = redoLogRecord->data[fieldPos + 17];

            ctx->dumpStream << "tabn: " << static_cast<uint64_t>(tabn) <<
                " lock: " << std::dec << static_cast<uint64_t>(lock) <<
                " nrow: " << std::dec << static_cast<uint64_t>(redoLogRecord->nRow) << '\n';

            if (fieldLength < 22 + static_cast<uint64_t>(redoLogRecord->nRow) * 2)
                throw RedoLogException(50061, "too short field kdo OpCode QMI (2): " + std::to_string(fieldLength) + " offset: " +
                                       std::to_string(redoLogRecord->dataOffset));
        }
    }

    void OpCode::kdoOpCode(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        if (fieldLength < 16)
            throw RedoLogException(50061, "too short field kdo OpCode: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->bdba = ctx->read32(redoLogRecord->data + fieldPos + 0);
        redoLogRecord->op = redoLogRecord->data[fieldPos + 10];
        redoLogRecord->flags = redoLogRecord->data[fieldPos + 11];
        redoLogRecord->itli = redoLogRecord->data[fieldPos + 12];

        if (ctx->dumpRedoLog >= 1) {
            typeDba hdba = ctx->read32(redoLogRecord->data + fieldPos + 4);
            uint16_t maxFr = ctx->read16(redoLogRecord->data + fieldPos + 8);
            uint8_t ispac = redoLogRecord->data[fieldPos + 13];

            const char* opCode = "???";
            switch (redoLogRecord->op & 0x1F) {
                // Interpret Undo Redo
                case OP_IUR:
                    opCode = "IUR";
                    break;

                // Insert Row Piece
                case OP_IRP:
                    opCode = "IRP";
                    break;

                // Delete Row Piece
                case OP_DRP:
                    opCode = "DRP";
                    break;

                // LocK Row
                case OP_LKR:
                    opCode = "LKR";
                    break;

                // Update Row Piece
                case OP_URP:
                    opCode = "URP";
                    break;

                // Overwrite Row Piece
                case OP_ORP:
                    opCode = "ORP";
                    break;

                // Manipulate First Column
                case OP_MFC:
                    opCode = "MFC";
                    break;

                // Change Forwarding Address
                case OP_CFA:
                    opCode = "CFA";
                    break;

                // Change Cluster key Index
                case OP_CKI:
                    opCode = "CKI";
                    break;

                // Set Key Links
                case OP_SKL:
                    opCode = "SKL";
                    break;

                // Quick Multi-row Insert
                case OP_QMI:
                    opCode = "QMI";
                    break;

                // Quick Multi-row Delete
                case OP_QMD:
                    opCode = "QMD";
                    break;

                case OP_DSC:
                    opCode = "DSC";
                    break;

                case OP_LMN:
                    opCode = "LMN";
                    break;

                case OP_LLB:
                    opCode = "LLB";
                    break;

                case OP_SHK:
                    opCode = "SHK";
                    break;

                case OP_CMP:
                    opCode = "CMP";
                    break;

                case OP_DCU:
                    opCode = "DCU";
                    break;

                case OP_MRK:
                    opCode = "MRK";
                    break;

                case OP_021:
                    opCode = " 21";
                    break;

                default:
                    opCode = "XXX";
                    if (ctx->dumpRedoLog >= 1)
                        ctx->dumpStream << "DEBUG op: " << std::dec << static_cast<uint64_t>(redoLogRecord->op & 0x1F) << '\n';
            }

            const char* xtype("0");
            const char* rtype("");
            switch (redoLogRecord->flags & 0x03) {
                // Redo
                case FLAGS_XA:
                    xtype = "XA";
                    break;

                // Rollback
                case FLAGS_XR:
                    xtype = "XR";
                    break;

                // Unknown
                case FLAGS_CR:
                    xtype = "CR";
                    break;
            }
            redoLogRecord->flags &= 0xFC;

            if ((redoLogRecord->flags & FLAGS_KDO_KDOM2) != 0)
                rtype = "xtype KDO_KDOM2";

            const char* rowDependencies("Disabled");
            if ((redoLogRecord->op & OP_ROWDEPENDENCIES) != 0)
                rowDependencies = "Enabled";

            ctx->dumpStream << "KDO Op code: " << opCode << " row dependencies " << rowDependencies << '\n';
            ctx->dumpStream << "  xtype: " << xtype << rtype <<
                    " flags: 0x" << std::setfill('0') << std::setw(8) << std::hex << static_cast<uint64_t>(redoLogRecord->flags) << " " <<
                    " bdba: 0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->bdba << " " <<
                    " hdba: 0x" << std::setfill('0') << std::setw(8) << std::hex << hdba << '\n';
            ctx->dumpStream << "itli: " << std::dec << static_cast<uint64_t>(redoLogRecord->itli) << " " <<
                    " ispac: " << std::dec << static_cast<uint64_t>(ispac) << " " <<
                    " maxfr: " << std::dec << static_cast<uint64_t>(maxFr) << '\n';

            switch ((redoLogRecord->op & 0x1F)) {
                case OP_SKL:
                    if (fieldLength >= 32) {
                        char fwdFl = '-';
                        uint32_t fwd = (static_cast<uint32_t>(redoLogRecord->data[fieldPos + 16]) << 24) |
                                (static_cast<uint32_t>(redoLogRecord->data[fieldPos + 17]) << 16) |
                                (static_cast<uint32_t>(redoLogRecord->data[fieldPos + 18]) << 8) |
                                static_cast<uint32_t>(redoLogRecord->data[fieldPos + 19]);
                        uint16_t fwdPos = (static_cast<uint16_t>(redoLogRecord->data[fieldPos + 20]) << 8) |
                                static_cast<uint16_t>(redoLogRecord->data[fieldPos + 21]);
                        char bkwFl = '-';
                        uint32_t bkw = (static_cast<uint32_t>(redoLogRecord->data[fieldPos + 22]) << 24) |
                                (static_cast<uint32_t>(redoLogRecord->data[fieldPos + 23]) << 16) |
                                (static_cast<uint32_t>(redoLogRecord->data[fieldPos + 24]) << 8) |
                                static_cast<uint32_t>(redoLogRecord->data[fieldPos + 25]);
                        uint16_t bkwPos = (static_cast<uint16_t>(redoLogRecord->data[fieldPos + 26]) << 8) |
                                static_cast<uint16_t>(redoLogRecord->data[fieldPos + 27]);
                        uint8_t fl = redoLogRecord->data[fieldPos + 28];
                        uint8_t lock = redoLogRecord->data[fieldPos + 29];
                        uint8_t slot = redoLogRecord->data[fieldPos + 30];

                        if (fl & 0x01) fwdFl = 'F';
                        if (fl & 0x02) bkwFl = 'B';

                        ctx->dumpStream << "flag: " << fwdFl << bkwFl <<
                                " lock: " << std::dec << static_cast<uint64_t>(lock) <<
                                " slot: " << std::dec << static_cast<uint64_t>(slot) <<
                                "(0x" << std::hex << static_cast<uint64_t>(slot) << ")\n";

                        if (fwdFl == 'F')
                            ctx->dumpStream << "fwd: 0x" << std::setfill('0') << std::setw(8) << std::hex << fwd << "." << fwdPos << " \n";
                        if (bkwFl == 'B')
                            ctx->dumpStream << "bkw: 0x" << std::setfill('0') << std::setw(8) << std::hex << bkw << "." << bkwPos << '\n';
                    }
                    break;

                case OP_DSC:
                    if (fieldLength >= 24) {
                        uint16_t slot = ctx->read16(redoLogRecord->data + fieldPos + 16);
                        uint8_t tabn = redoLogRecord->data[fieldPos + 18];
                        uint8_t rel = redoLogRecord->data[fieldPos + 19];

                        ctx->dumpStream << "tabn: " << std::dec << static_cast<uint64_t>(tabn) << " slot: " << slot <<
                                "(0x" << std::hex << slot << ")\n";
                        ctx->dumpStream << "piece relative column number: " << std::dec << static_cast<uint64_t>(rel) << '\n';
                    }
                    break;
            }
        }

        switch (redoLogRecord->op & 0x1F) {
            case OP_IRP:
                kdoOpCodeIRP(ctx, redoLogRecord, fieldPos, fieldLength);
                break;

            case OP_DRP:
                kdoOpCodeDRP(ctx, redoLogRecord, fieldPos, fieldLength);
                break;

            case OP_LKR:
                kdoOpCodeLKR(ctx, redoLogRecord, fieldPos, fieldLength);
                break;

            case OP_URP:
                kdoOpCodeURP(ctx, redoLogRecord, fieldPos, fieldLength);
                break;

            case OP_ORP:
                kdoOpCodeORP(ctx, redoLogRecord, fieldPos, fieldLength);
                break;

            case OP_CKI:
                kdoOpCodeSKL(ctx, redoLogRecord, fieldPos, fieldLength);
                break;

            case OP_CFA:
                kdoOpCodeCFA(ctx, redoLogRecord, fieldPos, fieldLength);
                break;

            case OP_QMI:
            case OP_QMD:
                kdoOpCodeQM(ctx, redoLogRecord, fieldPos, fieldLength);
                break;
        }
    }

    void OpCode::ktub(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength, bool isKtubl) {
        if (fieldLength < 24)
            throw RedoLogException(50061, "too short field ktub (1): " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->obj = ctx->read32(redoLogRecord->data + fieldPos + 0);
        redoLogRecord->dataObj = ctx->read32(redoLogRecord->data + fieldPos + 4);
        redoLogRecord->tsn = ctx->read32(redoLogRecord->data + fieldPos + 8);
        redoLogRecord->undo = ctx->read32(redoLogRecord->data + fieldPos + 12);
        redoLogRecord->opc = (static_cast<typeOp1>(redoLogRecord->data[fieldPos + 16]) << 8) | redoLogRecord->data[fieldPos + 17];
        redoLogRecord->slt = redoLogRecord->data[fieldPos + 18];
        redoLogRecord->rci = redoLogRecord->data[fieldPos + 19];
        redoLogRecord->flg = ctx->read16(redoLogRecord->data + fieldPos + 20);

        const char* ktuType("ktubu");
        const char* prevObj("");
        const char* postObj("");
        bool ktubl = false;

        if ((redoLogRecord->flg & FLG_BEGIN_TRANS) != 0 && isKtubl) {
            ktubl = true;
            ktuType = "ktubl";
            if (ctx->version < REDO_VERSION_19_0) {
                prevObj = "[";
                postObj = "]";
            }
        }

        if (ctx->version < REDO_VERSION_19_0) {
            ctx->dumpStream <<
                    ktuType << " redo:" <<
                    " slt: " << std::dec << static_cast<uint64_t>(redoLogRecord->slt) <<
                    " rci: " << std::dec << static_cast<uint64_t>(redoLogRecord->rci) <<
                    " opc: " << std::dec << static_cast<uint64_t>(redoLogRecord->opc >> 8) << "." << static_cast<uint64_t>(redoLogRecord->opc & 0xFF) <<
                    " " << prevObj << "objn: " << std::dec << redoLogRecord->obj <<
                    " objd: " << std::dec << redoLogRecord->dataObj <<
                    " tsn: " << std::dec << redoLogRecord->tsn << postObj << '\n';
        } else {
            typeDba prevDba = ctx->read32(redoLogRecord->data + fieldPos + 12);
            uint16_t wrp = ctx->read16(redoLogRecord->data + fieldPos + 22);

            ctx->dumpStream <<
                    ktuType << " redo:" <<
                    " slt: "  << std::dec << static_cast<uint64_t>(redoLogRecord->slt) <<
                    " wrp: " << std::dec << wrp <<
                    " flg: 0x" << std::setfill('0') << std::setw(4) << std::hex << redoLogRecord->flg <<
                    " prev dba:  0x" << std::setfill('0') << std::setw(8) << std::hex << prevDba <<
                    " rci: " << std::dec << static_cast<uint64_t>(redoLogRecord->rci) <<
                    " opc: " << std::dec << static_cast<uint64_t>(redoLogRecord->opc >> 8) << "." << static_cast<uint64_t>(redoLogRecord->opc & 0xFF) <<
                    " [objn: " << std::dec << redoLogRecord->obj <<
                    " objd: " << std::dec << redoLogRecord->dataObj <<
                    " tsn: " << std::dec << redoLogRecord->tsn << "]\n";
        }

        const char* lastBufferSplit;
        if ((redoLogRecord->flg & FLG_LASTBUFFERSPLIT) != 0)
            lastBufferSplit = "Yes";
        else {
            if (ctx->version < REDO_VERSION_19_0)
                lastBufferSplit = "No";
            else
                lastBufferSplit = " No";
        }

        const char* userUndoDone;
        if ((redoLogRecord->flg & FLG_USERUNDODDONE) != 0)
            userUndoDone = "Yes";
        else {
            if (ctx->version < REDO_VERSION_19_0)
                userUndoDone = "No";
            else
                userUndoDone = " No";
        }

        const char* undoType;
        if (ctx->version < REDO_VERSION_12_2) {
            if ((redoLogRecord->flg & FLG_MULTIBLOCKUNDOHEAD) != 0)
                undoType = "Multi-block undo - HEAD";
            else if ((redoLogRecord->flg & FLG_MULTIBLOCKUNDOTAIL) != 0)
                undoType = "Multi-Block undo - TAIL";
            else if ((redoLogRecord->flg & FLG_MULTIBLOCKUNDOMID) != 0)
                undoType = "Multi-block undo - MID";
            else
                undoType = "Regular undo      ";
        } else if (ctx->version < REDO_VERSION_19_0) {
            if ((redoLogRecord->flg & FLG_MULTIBLOCKUNDOHEAD) != 0)
                undoType = "Multi-block undo - HEAD";
            else if ((redoLogRecord->flg & FLG_MULTIBLOCKUNDOTAIL) != 0)
                undoType = "Multi-Block undo - TAIL";
            else if ((redoLogRecord->flg & FLG_MULTIBLOCKUNDOMID) != 0)
                undoType = "Multi-Block undo - MID";
            else
                undoType = "Regular undo      ";
        } else {
            if ((redoLogRecord->flg & FLG_MULTIBLOCKUNDOHEAD) != 0)
                undoType = "MBU - HEAD  ";
            else if ((redoLogRecord->flg & FLG_MULTIBLOCKUNDOTAIL) != 0)
                undoType = "MBU - TAIL  ";
            else if ((redoLogRecord->flg & FLG_MULTIBLOCKUNDOMID) != 0)
                undoType = "MBU - MID   ";
            else
                undoType = "Regular undo";
        }

        const char* tempObject;
        if ((redoLogRecord->flg & FLG_ISTEMPOBJECT) != 0)
            tempObject = "Yes";
        else {
            if (ctx->version < REDO_VERSION_19_0)
                tempObject = "No";
            else
                tempObject = " No";
        }

        const char* tablespaceUndo;
        if ((redoLogRecord->flg & FLG_TABLESPACEUNDO) != 0)
            tablespaceUndo = "Yes";
        else {
            if (ctx->version < REDO_VERSION_19_0)
                tablespaceUndo = "No";
            else
                tablespaceUndo = " No";
        }

        const char* userOnly(" No");
        if ((redoLogRecord->flg & FLG_USERONLY) != 0)
            userOnly = "Yes";
        else {
            if (ctx->version < REDO_VERSION_19_0)
                userOnly = "No";
            else
                userOnly = " No";
        }

        if (ctx->dumpRedoLog < 1)
            return;

        if (ktubl) {
            // KTUBL
            if (fieldLength < 28) {
                ctx->warning(50061, "too short field ktubl: " + std::to_string(fieldLength) + " offset: " +
                             std::to_string(redoLogRecord->dataOffset));
                return;
            }

            if (fieldLength == 28) {
                uint16_t flg2 = ctx->read16(redoLogRecord->data + fieldPos + 24);
                auto buExtIdx = (int16_t)ctx->read16(redoLogRecord->data + fieldPos + 26);

                if (ctx->version < REDO_VERSION_19_0) {
                    ctx->dumpStream <<
                            "Undo type:  " << undoType << "  " <<
                            "Begin trans    Last buffer split:  " << lastBufferSplit << " \n" <<
                            "Temp Object:  " << tempObject << " \n" <<
                            "Tablespace Undo:  " << tablespaceUndo << " \n" <<
                            "             0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->undo << " \n";

                    ctx->dumpStream <<
                            " BuExt idx: " << std::dec << buExtIdx <<
                            " flg2: " << std::hex << flg2 << '\n';
                } else {
                    ctx->dumpStream <<
                            "[Undo type  ] " << undoType << " " <<
                            " [User undo done   ] " << userUndoDone << " " <<
                            " [Last buffer split] " << lastBufferSplit << " \n" <<
                            "[Temp object]          " << tempObject << " " <<
                            " [Tablespace Undo  ] " << tablespaceUndo << " " <<
                            " [User only        ] " << userOnly << " \n" <<
                            "Begin trans    \n";

                    ctx->dumpStream <<
                            "BuExt idx: " << std::dec << buExtIdx <<
                            " flg2: " << std::hex << flg2 << '\n';
                }
            } else if (fieldLength >= 76) {
                uint16_t flg2 = ctx->read16(redoLogRecord->data + fieldPos + 24);
                auto buExtIdx = (int16_t)ctx->read16(redoLogRecord->data + fieldPos + 26);
                typeUba prevCtlUba = ctx->read56(redoLogRecord->data + fieldPos + 28);
                typeScn prevCtlMaxCmtScn = ctx->readScn(redoLogRecord->data + fieldPos + 36);
                typeScn prevTxCmtScn = ctx->readScn(redoLogRecord->data + fieldPos + 44);
                typeScn txStartScn = ctx->readScn(redoLogRecord->data + fieldPos + 56);
                uint32_t prevBrb = ctx->read32(redoLogRecord->data + fieldPos + 64);
                uint32_t prevBcl = ctx->read32(redoLogRecord->data + fieldPos + 68);
                uint32_t logonUser = ctx->read32(redoLogRecord->data + fieldPos + 72);

                if (ctx->version < REDO_VERSION_12_2) {
                    ctx->dumpStream <<
                            "Undo type:  " << undoType << "  " <<
                            "Begin trans    Last buffer split:  " << lastBufferSplit << " \n" <<
                            "Temp Object:  " << tempObject << " \n" <<
                            "Tablespace Undo:  " << tablespaceUndo << " \n" <<
                            "             0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->undo << " " <<
                            " prev ctl uba: " << PRINTUBA(prevCtlUba) << " \n" <<
                            "prev ctl max cmt scn:  " << PRINTSCN48(prevCtlMaxCmtScn) << " " <<
                            " prev tx cmt scn:  " << PRINTSCN48(prevTxCmtScn) << " \n";

                    ctx->dumpStream <<
                            "txn start scn:  " << PRINTSCN48(txStartScn) << " " <<
                            " logon user: " << std::dec << logonUser << " " <<
                            " prev brb: " << prevBrb << " " <<
                            " prev bcl: " << std::dec << prevBcl;

                    ctx->dumpStream <<
                            " BuExt idx: " << std::dec << buExtIdx <<
                            " flg2: " << std::hex << flg2 << '\n';
                } else if (ctx->version < REDO_VERSION_19_0) {
                    ctx->dumpStream <<
                            "Undo type:  " << undoType << "  " <<
                            "Begin trans    Last buffer split:  " << lastBufferSplit << " \n" <<
                            "Temp Object:  " << tempObject << " \n" <<
                            "Tablespace Undo:  " << tablespaceUndo << " \n" <<
                            "             0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->undo << " " <<
                            " prev ctl uba: " << PRINTUBA(prevCtlUba) << " \n" <<
                            "prev ctl max cmt scn:  " << PRINTSCN64(prevCtlMaxCmtScn) << " " <<
                            " prev tx cmt scn:  " << PRINTSCN64(prevTxCmtScn) << " \n";

                    ctx->dumpStream <<
                            "txn start scn:  " << PRINTSCN64(txStartScn) << " " <<
                            " logon user: " << std::dec << logonUser << " " <<
                            " prev brb: " << prevBrb << " " <<
                            " prev bcl: " << std::dec << prevBcl;

                    ctx->dumpStream <<
                            " BuExt idx: " << std::dec << buExtIdx <<
                            " flg2: " << std::hex << flg2 << '\n';
                } else {
                    ctx->dumpStream <<
                            "[Undo type  ] " << undoType << " " <<
                            " [User undo done   ] " << userUndoDone << " " <<
                            " [Last buffer split] " << lastBufferSplit << " \n" <<
                            "[Temp object]          " << tempObject << " " <<
                            " [Tablespace Undo  ] " << tablespaceUndo << " " <<
                            " [User only        ] " << userOnly << " \n" <<
                            "Begin trans    \n" <<
                            " prev ctl uba: " << PRINTUBA(prevCtlUba) <<
                            " prev ctl max cmt scn:  " << PRINTSCN64(prevCtlMaxCmtScn) << " \n" <<
                            " prev tx cmt scn:  " << PRINTSCN64(prevTxCmtScn) << " \n";

                    ctx->dumpStream <<
                            " txn start scn:  " << PRINTSCN64(txStartScn) <<
                            "  logon user: " << std::dec << logonUser << '\n' <<
                            " prev brb:  0x" << std::setfill('0') << std::setw(8) << std::hex << prevBrb <<
                            "  prev bcl:  0x" << std::setfill('0') << std::setw(8) << std::hex << prevBcl << '\n';

                    ctx->dumpStream <<
                            "BuExt idx: " << std::dec << buExtIdx <<
                            " flg2: " << std::hex << flg2 << '\n';
                }
            }
        } else {
            // KTUBU
            if (ctx->version < REDO_VERSION_19_0) {
                ctx->dumpStream <<
                        "Undo type:  " << undoType << " " <<
                        "Undo type:  ";
                if ((redoLogRecord->flg & FLG_USERUNDODDONE) != 0)
                    ctx->dumpStream << "User undo done   ";
                if ((redoLogRecord->flg & FLG_BEGIN_TRANS) != 0)
                    ctx->dumpStream << " Begin trans    ";
                ctx->dumpStream <<
                        "Last buffer split:  " << lastBufferSplit << " \n" <<
                        "Tablespace Undo:  " << tablespaceUndo << " \n" <<
                        "             0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->undo << '\n';

                if ((redoLogRecord->flg & FLG_BUEXT) != 0) {
                    uint16_t flg2 = ctx->read16(redoLogRecord->data + fieldPos + 24);
                    auto buExtIdx = (int16_t)ctx->read16(redoLogRecord->data + fieldPos + 26);

                    ctx->dumpStream <<
                            "BuExt idx: " << std::dec << buExtIdx <<
                            " flg2: " << std::hex << flg2 << '\n';
                }

            } else {
                ctx->dumpStream <<
                        "[Undo type  ] " << undoType << " " <<
                        " [User undo done   ] " << userUndoDone << " " <<
                        " [Last buffer split] " << lastBufferSplit << " \n" <<
                        "[Temp object]          " << tempObject << " " <<
                        " [Tablespace Undo  ] " << tablespaceUndo << " " <<
                        " [User only        ] " << userOnly << " \n";
            }
        }
    }

    void OpCode::dumpMemory(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        if (ctx->dumpRedoLog >= 1) {
            ctx->dumpStream << "Dump of memory from 0xXXXXXXXXXXXXXXXX to 0xXXXXXXXXXXXXXXXX\n";

            uint64_t start = fieldPos & 0xFFFFFFFFFFFFFFF0;
            uint64_t end = (fieldPos + fieldLength + 15) & 0xFFFFFFFFFFFFFFF0;
            for (uint64_t i = start; i < end; i += 16) {
                ctx->dumpStream << "XXXXXXXXXXXX";

                int64_t first = -1, last = -1;
                for (uint64_t j = 0; j < 4; ++j) {
                    if (i + j * 4 >= fieldPos && i + j * 4 < fieldPos + fieldLength) {
                        if (first == -1)
                            first = j;
                        last = j;
                        uint32_t val = ctx->read32(redoLogRecord->data + i + j * 4);
                        ctx->dumpStream << " " << std::setfill('0') << std::setw(8) << std::hex << std::uppercase << val;
                    } else {
                        ctx->dumpStream << "         ";
                    }
                }
                ctx->dumpStream << "  ";

                for (int64_t j = 0; j < first; ++j)
                    ctx->dumpStream << "    ";
                ctx->dumpStream << "[";

                for (int64_t j = first; j <= last; ++j)
                    ctx->dumpStream << "....";
                ctx->dumpStream << "]\n";
            }
            ctx->dumpStream << std::nouppercase;
        }
    }

    void OpCode::dumpColVector(Ctx* ctx, RedoLogRecord* redoLogRecord, uint8_t* data, uint64_t colNum) {
        uint64_t pos = 0;

        ctx->dumpStream << "Vector content: \n";

        for (uint64_t k = 0; k < redoLogRecord->cc; ++k) {
            uint16_t fieldLength = data[pos];
            ++pos;
            uint8_t isNull = (fieldLength == 0xFF);

            if (fieldLength == 0xFE) {
                fieldLength = ctx->read16(data + pos);
                pos += 2;
            }

            dumpCols(ctx, redoLogRecord, data + pos, colNum + k, fieldLength, isNull);

            if (!isNull)
                pos += fieldLength;
        }
    }

    void OpCode::dumpCompressed(Ctx* ctx, RedoLogRecord* redoLogRecord, uint8_t* data, uint16_t fieldLength) {
        std::ostringstream ss;
        ss << "kdrhccnt=" << std::dec << static_cast<uint64_t>(redoLogRecord->cc) << ",full row:";
        ss << std::uppercase;

        for (uint64_t j = 0; j < fieldLength; ++j) {
            ss << " " << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(data[j]);
            if (ss.str().length() > 256) {
                ctx->dumpStream << ss.str() << '\n';
                ss.str(std::string());
            }
        }

        if (ss.str().length()  > 0) {
            ctx->dumpStream << ss.str() << '\n';
        }
    }

    void OpCode::dumpCols(Ctx* ctx, RedoLogRecord* redoLogRecord __attribute__((unused)), uint8_t* data, uint64_t colNum, uint16_t fieldLength,
                          uint8_t isNull) {
        if (isNull) {
            ctx->dumpStream << "col " << std::setfill(' ') << std::setw(2) << std::dec << colNum << ": *NULL*\n";
        } else {
            ctx->dumpStream << "col " << std::setfill(' ') << std::setw(2) << std::dec << colNum << ": " <<
                    "[" << std::setfill(' ') << std::setw(2) << std::dec << fieldLength << "]";

            if (fieldLength <= 20)
                ctx->dumpStream << " ";
            else
                ctx->dumpStream << '\n';

            for (uint64_t j = 0; j < fieldLength; ++j) {
                ctx->dumpStream << " " << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(data[j]);
                if ((j % 25) == 24 && j != static_cast<uint64_t>(fieldLength) - 1)
                    ctx->dumpStream << '\n';
            }

            ctx->dumpStream << '\n';
        }
    }

    void OpCode::dumpRows(Ctx* ctx, RedoLogRecord* redoLogRecord, uint8_t* data) {
        if (ctx->dumpRedoLog >= 1) {
            uint64_t pos = 0;
            char fbStr[9] = "--------";

            for (uint64_t r = 0; r < redoLogRecord->nRow; ++r) {
                ctx->dumpStream << "slot[" << std::dec << r << "]: " << std::dec << ctx->read16(redoLogRecord->data + redoLogRecord->slotsDelta + r * 2) <<
                        '\n';
                processFbFlags(data[pos + 0], fbStr);
                uint8_t lb = data[pos + 1];
                uint8_t jcc = data[pos + 2];
                uint16_t tl = ctx->read16(redoLogRecord->data + redoLogRecord->rowLenghsDelta + r * 2);

                ctx->dumpStream << "tl: " << std::dec << tl <<
                        " fb: " << fbStr <<
                        " lb: 0x" << std::hex << static_cast<uint64_t>(lb) << " " <<
                        " cc: " << std::dec << static_cast<uint64_t>(jcc) << '\n';
                pos += 3;

                if ((redoLogRecord->op & OP_ROWDEPENDENCIES) != 0) {
                    if (ctx->version < REDO_VERSION_12_2)
                        pos += 6;
                    else
                        pos += 8;
                }

                for (uint64_t k = 0; k < jcc; ++k) {
                    uint16_t fieldLength = data[pos];
                    ++pos;
                    uint8_t isNull = (fieldLength == 0xFF);

                    if (fieldLength == 0xFE) {
                        fieldLength = ctx->read16(data + pos);
                        pos += 2;
                    }

                    dumpCols(ctx, redoLogRecord, data + pos, k, fieldLength, isNull);

                    if (!isNull)
                        pos += fieldLength;
                }
            }
        }
    }

    void OpCode::dumpHex(Ctx* ctx, RedoLogRecord* redoLogRecord) {
        std::string header = "## 0: [" + std::to_string(redoLogRecord->dataOffset) + "] " + std::to_string(redoLogRecord->fieldLengthsDelta);
        ctx->dumpStream << header;
        if (header.length() < 36)
            ctx->dumpStream << std::string(36 - header.length(), ' ');

        for (uint64_t j = 0; j < redoLogRecord->fieldLengthsDelta; ++j)
            ctx->dumpStream << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(redoLogRecord->data[j]) << " ";
        ctx->dumpStream << '\n';

        uint64_t fieldPosLocal = redoLogRecord->fieldPos;
        for (uint64_t i = 1; i <= redoLogRecord->fieldCnt; ++i) {
            uint16_t fieldLength = ctx->read16(redoLogRecord->data + redoLogRecord->fieldLengthsDelta + i * 2);
            header = "## " + std::to_string(i) + ": [" + std::to_string(redoLogRecord->dataOffset + fieldPosLocal) + "] " + std::to_string(fieldLength) +  "   ";
            ctx->dumpStream << header;
            if (header.length() < 36)
                ctx->dumpStream << std::string(36 - header.length(), ' ');

            for (uint64_t j = 0; j < fieldLength; ++j)
                ctx->dumpStream << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint64_t>(redoLogRecord->data[fieldPosLocal + j]) << " ";
            ctx->dumpStream << '\n';

            fieldPosLocal += (fieldLength + 3) & 0xFFFC;
        }
    }

    void OpCode::processFbFlags(uint8_t fb, char* fbStr) {
        if ((fb & FB_N) != 0) fbStr[7] = 'N'; else fbStr[7] = '-'; // The last column continues in the Next piece
        if ((fb & FB_P) != 0) fbStr[6] = 'P'; else fbStr[6] = '-'; // The first column continues from the Previous piece
        if ((fb & FB_L) != 0) fbStr[5] = 'L'; else fbStr[5] = '-'; // Last ctx piece
        if ((fb & FB_F) != 0) fbStr[4] = 'F'; else fbStr[4] = '-'; // First ctx piece
        if ((fb & FB_D) != 0) fbStr[3] = 'D'; else fbStr[3] = '-'; // Deleted row
        if ((fb & FB_H) != 0) fbStr[2] = 'H'; else fbStr[2] = '-'; // Head piece of row
        if ((fb & FB_C) != 0) fbStr[1] = 'C'; else fbStr[1] = '-'; // Clustered table member
        if ((fb & FB_K) != 0) fbStr[0] = 'K'; else fbStr[0] = '-'; // Cluster Key
        fbStr[8] = 0;
    }
}
