/* Buffer to handle transactions
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

#include "../common/RedoLogException.h"
#include "../common/RedoLogRecord.h"
#include "OpCode0501.h"
#include "OpCode050B.h"
#include "Transaction.h"
#include "TransactionBuffer.h"

namespace OpenLogReplicator {
    TransactionBuffer::TransactionBuffer(Ctx* newCtx) :
        ctx(newCtx) {
    }

    TransactionBuffer::~TransactionBuffer() {
        if (!partiallyFullChunks.empty())
            ctx->error(50062, "non-free blocks in transaction buffer: " + std::to_string(partiallyFullChunks.size()));

        skipXidList.clear();
        dumpXidList.clear();
        brokenXidMapList.clear();

        for (const auto& orphanedLobsIt: orphanedLobs) {
            uint8_t* data = orphanedLobsIt.second;
            delete[] data;
        }
        orphanedLobs.clear();
    }

    void TransactionBuffer::purge() {
        for (auto xidTransactionMapIt : xidTransactionMap) {
            Transaction* transaction = xidTransactionMapIt.second;
            transaction->purge(this);
            delete transaction;
        }
        xidTransactionMap.clear();
    }

    Transaction* TransactionBuffer::findTransaction(typeXid xid, typeConId conId, bool old, bool add, bool rollback) {
        typeXidMap xidMap = (xid.getData() >> 32) | ((static_cast<uint64_t>(conId)) << 32);
        Transaction* transaction;

        auto xidTransactionMapIt = xidTransactionMap.find(xidMap);
        if (xidTransactionMapIt != xidTransactionMap.end()) {
            transaction = xidTransactionMapIt->second;
            if (!rollback && (!old || transaction->xid != xid))
                throw RedoLogException(50039, "transaction " + xid.toString() + " conflicts with " + transaction->xid.toString());
        } else {
            if (!add)
                return nullptr;

            transaction = new Transaction(xid, &orphanedLobs);
            {
                std::unique_lock<std::mutex> lck(mtx);
                xidTransactionMap.insert_or_assign(xidMap, transaction);
            }

            if (dumpXidList.find(xid) != dumpXidList.end())
                transaction->dump = true;
        }

        return transaction;
    }

    void TransactionBuffer::dropTransaction(typeXid xid, typeConId conId) {
        typeXidMap xidMap = (xid.getData() >> 32) | (static_cast<uint64_t>(conId) << 32);
        {
            std::unique_lock<std::mutex> lck(mtx);
            xidTransactionMap.erase(xidMap);
        }
    }

    TransactionChunk* TransactionBuffer::newTransactionChunk() {
        uint8_t* chunk;
        TransactionChunk* tc;
        uint64_t pos;
        uint64_t freeMap;
        if (!partiallyFullChunks.empty()) {
            auto partiallyFullChunksIt = partiallyFullChunks.cbegin();
            chunk = partiallyFullChunksIt->first;
            freeMap = partiallyFullChunksIt->second;
            pos = ffs(freeMap) - 1;
            freeMap &= ~(1 << pos);
            if (freeMap == 0)
                partiallyFullChunks.erase(chunk);
            else
                partiallyFullChunks.insert_or_assign(chunk, freeMap);
        } else {
            chunk = ctx->getMemoryChunk("transaction", false);
            pos = 0;
            freeMap = BUFFERS_FREE_MASK & (~1);
            partiallyFullChunks.insert_or_assign(chunk, freeMap);
        }

        tc = reinterpret_cast<TransactionChunk*>(chunk + FULL_BUFFER_SIZE * pos);
        memset(reinterpret_cast<void*>(tc), 0, HEADER_BUFFER_SIZE);
        tc->header = chunk;
        tc->pos = pos;
        return tc;
    }

    void TransactionBuffer::deleteTransactionChunk(TransactionChunk* tc) {
        uint8_t* chunk = tc->header;
        uint64_t pos = tc->pos;
        uint64_t freeMap = partiallyFullChunks[chunk];

        freeMap |= (1 << pos);

        if (freeMap == BUFFERS_FREE_MASK) {
            ctx->freeMemoryChunk("transaction", chunk, false);
            partiallyFullChunks.erase(chunk);
        } else
            partiallyFullChunks.insert_or_assign(chunk, freeMap);
    }

    void TransactionBuffer::deleteTransactionChunks(TransactionChunk* tc) {
        TransactionChunk* nextTc;
        while (tc != nullptr) {
            nextTc = tc->next;
            deleteTransactionChunk(tc);
            tc = nextTc;
        }
    }

    void TransactionBuffer::addTransactionChunk(Transaction* transaction, RedoLogRecord* redoLogRecord) {
        uint64_t length = redoLogRecord->length + ROW_HEADER_TOTAL;

        if (length > DATA_BUFFER_SIZE)
            throw RedoLogException(50040, "block size (" + std::to_string(length) + ") exceeding max block size (" +
                                   std::to_string(FULL_BUFFER_SIZE) + "), try increasing the FULL_BUFFER_SIZE parameter");

        if (transaction->lastSplit) {
            if ((redoLogRecord->flg & FLG_MULTIBLOCKUNDOMID) == 0)
                throw RedoLogException(50041, "bad split offset: " + std::to_string(redoLogRecord->dataOffset) + " xid: " +
                                       transaction->xid.toString());

            uint64_t lengthLast = *(reinterpret_cast<uint64_t*>(transaction->lastTc->buffer + transaction->lastTc->size - ROW_HEADER_TOTAL + ROW_HEADER_SIZE));
            RedoLogRecord* last501 = reinterpret_cast<RedoLogRecord*>(transaction->lastTc->buffer + transaction->lastTc->size - lengthLast + ROW_HEADER_REDO1);
            last501->data = transaction->lastTc->buffer + transaction->lastTc->size - lengthLast + ROW_HEADER_DATA;

            uint64_t size = last501->length + redoLogRecord->length;
            transaction->mergeBuffer = new uint8_t[size];
            mergeBlocks(transaction->mergeBuffer, redoLogRecord, last501);
            rollbackTransactionChunk(transaction);
        }
        if ((redoLogRecord->flg & (FLG_MULTIBLOCKUNDOTAIL | FLG_MULTIBLOCKUNDOMID)) != 0)
            transaction->lastSplit = true;
        else
            transaction->lastSplit = false;

        // Empty list
        if (transaction->lastTc == nullptr) {
            transaction->lastTc = newTransactionChunk();
            transaction->firstTc = transaction->lastTc;
        }

        // New block needed
        if (transaction->lastTc->size + length > DATA_BUFFER_SIZE) {
            TransactionChunk* tcNew = newTransactionChunk();
            tcNew->prev = transaction->lastTc;
            transaction->lastTc->next = tcNew;
            transaction->lastTc = tcNew;
        }

        // Append to the chunk at the end
        TransactionChunk* tc = transaction->lastTc;
        *(reinterpret_cast<typeOp2*>(tc->buffer + tc->size + ROW_HEADER_OP)) = (redoLogRecord->opCode << 16);
        memcpy(reinterpret_cast<void*>(tc->buffer + tc->size + ROW_HEADER_REDO1),
               reinterpret_cast<const void*>(redoLogRecord), sizeof(RedoLogRecord));
        memset(reinterpret_cast<void*>(tc->buffer + tc->size + ROW_HEADER_REDO2), 0, sizeof(RedoLogRecord));
        memcpy(reinterpret_cast<void*>(tc->buffer + tc->size + ROW_HEADER_DATA),
               reinterpret_cast<const void*>(redoLogRecord->data), redoLogRecord->length);

        *(reinterpret_cast<uint64_t*>(tc->buffer + tc->size + ROW_HEADER_SIZE + redoLogRecord->length)) = length;

        tc->size += length;
        ++tc->elements;
        transaction->size += length;

        if (transaction->mergeBuffer != nullptr) {
            delete[] transaction->mergeBuffer;
            transaction->mergeBuffer = nullptr;
        }
    }

    void TransactionBuffer::addTransactionChunk(Transaction* transaction, RedoLogRecord* redoLogRecord1, RedoLogRecord* redoLogRecord2) {
        uint64_t length = redoLogRecord1->length + redoLogRecord2->length + ROW_HEADER_TOTAL;

        if (length > DATA_BUFFER_SIZE)
            throw RedoLogException(50040, "block size (" + std::to_string(length) +  ") exceeding max block size (" +
                                   std::to_string(FULL_BUFFER_SIZE) + "), try increasing the FULL_BUFFER_SIZE parameter");

        if (transaction->lastSplit) {
            if ((redoLogRecord1->opCode) != 0x0501)
                throw RedoLogException(50042, "split undo HEAD no 5.1 offset: " + std::to_string(redoLogRecord1->dataOffset));

            if ((redoLogRecord1->flg & FLG_MULTIBLOCKUNDOHEAD) == 0)
                throw RedoLogException(50043, "bad split offset: " + std::to_string(redoLogRecord1->dataOffset) + " xid: " +
                                       transaction->xid.toString() + " second position");

            uint64_t lengthLast = *(reinterpret_cast<uint64_t*>(transaction->lastTc->buffer + transaction->lastTc->size - ROW_HEADER_TOTAL + ROW_HEADER_SIZE));
            RedoLogRecord* last501 = reinterpret_cast<RedoLogRecord*>(transaction->lastTc->buffer + transaction->lastTc->size - lengthLast + ROW_HEADER_REDO1);
            last501->data = transaction->lastTc->buffer + transaction->lastTc->size - lengthLast + ROW_HEADER_DATA;

            uint64_t size = last501->length + redoLogRecord1->length;
            transaction->mergeBuffer = new uint8_t[size];
            mergeBlocks(transaction->mergeBuffer, redoLogRecord1, last501);

            uint16_t fieldPos = redoLogRecord1->fieldPos;
            uint16_t fieldLength = ctx->read16(redoLogRecord1->data + redoLogRecord1->fieldLengthsDelta + 1 * 2);
            fieldPos += (fieldLength + 3) & 0xFFFC;

            ctx->write16(redoLogRecord1->data + fieldPos + 20, redoLogRecord1->flg);
            OpCode0501::process(ctx, redoLogRecord1);
            length = redoLogRecord1->length + redoLogRecord2->length + ROW_HEADER_TOTAL;

            rollbackTransactionChunk(transaction);
            transaction->lastSplit = false;
        }

        // Empty list
        if (transaction->lastTc == nullptr) {
            transaction->lastTc = newTransactionChunk();
            transaction->firstTc = transaction->lastTc;
        }

        // New block needed
        if (transaction->lastTc->size + length > DATA_BUFFER_SIZE) {
            TransactionChunk* tcNew = newTransactionChunk();
            tcNew->prev = transaction->lastTc;
            transaction->lastTc->next = tcNew;
            transaction->lastTc = tcNew;
        }

        // Append to the chunk at the end
        TransactionChunk* tc = transaction->lastTc;
        *(reinterpret_cast<typeOp2*>(tc->buffer + tc->size + ROW_HEADER_OP)) = (redoLogRecord1->opCode << 16) | redoLogRecord2->opCode;
        memcpy(reinterpret_cast<void*>(tc->buffer + tc->size + ROW_HEADER_REDO1),
               reinterpret_cast<const void*>(redoLogRecord1), sizeof(RedoLogRecord));
        memcpy(reinterpret_cast<void*>(tc->buffer + tc->size + ROW_HEADER_REDO2),
               reinterpret_cast<const void*>(redoLogRecord2), sizeof(RedoLogRecord));
        memcpy(reinterpret_cast<void*>(tc->buffer + tc->size + ROW_HEADER_DATA),
               reinterpret_cast<const void*>(redoLogRecord1->data), redoLogRecord1->length);
        memcpy(reinterpret_cast<void*>(tc->buffer + tc->size + ROW_HEADER_DATA + redoLogRecord1->length),
               reinterpret_cast<const void*>(redoLogRecord2->data), redoLogRecord2->length);

        *(reinterpret_cast<uint64_t*>(tc->buffer + tc->size + ROW_HEADER_SIZE + redoLogRecord1->length + redoLogRecord2->length)) = length;

        tc->size += length;
        ++tc->elements;
        transaction->size += length;

        if (transaction->mergeBuffer != nullptr) {
            delete[] transaction->mergeBuffer;
            transaction->mergeBuffer = nullptr;
        }
    }

    void TransactionBuffer::rollbackTransactionChunk(Transaction* transaction) {
        if (transaction->lastTc == nullptr || transaction->lastTc->size < ROW_HEADER_TOTAL || transaction->lastTc->elements == 0)
            throw RedoLogException(50044, "trying to remove from empty buffer size: " + std::to_string(transaction->lastTc->size) +
                                   " elements: " + std::to_string(transaction->lastTc->elements));

        uint64_t length = *(reinterpret_cast<uint64_t*>(transaction->lastTc->buffer + transaction->lastTc->size - ROW_HEADER_TOTAL + ROW_HEADER_SIZE));
        transaction->lastTc->size -= length;
        --transaction->lastTc->elements;
        transaction->size -= length;

        if (transaction->lastTc->elements == 0) {
            TransactionChunk* tc = transaction->lastTc;
            transaction->lastTc = tc->prev;

            if (transaction->lastTc != nullptr) {
                transaction->lastTc->next = nullptr;
            } else {
                transaction->firstTc = nullptr;
            }
            deleteTransactionChunk(tc);
        }
    }

    void TransactionBuffer::mergeBlocks(uint8_t* mergeBuffer, RedoLogRecord* redoLogRecord1, RedoLogRecord* redoLogRecord2) {
        memcpy(reinterpret_cast<void*>(mergeBuffer),
               reinterpret_cast<const void*>(redoLogRecord1->data), redoLogRecord1->fieldLengthsDelta);
        uint64_t pos = redoLogRecord1->fieldLengthsDelta;
        uint16_t fieldCnt;
        uint16_t fieldPos1;
        uint16_t fieldPos2;

        if ((redoLogRecord1->flg & FLG_LASTBUFFERSPLIT) != 0) {
            redoLogRecord1->flg &= ~FLG_LASTBUFFERSPLIT;
            uint16_t length1 = ctx->read16(redoLogRecord1->data + redoLogRecord1->fieldLengthsDelta + redoLogRecord1->fieldCnt * 2);
            uint16_t length2 = ctx->read16(redoLogRecord2->data + redoLogRecord2->fieldLengthsDelta + 6);
            ctx->write16(redoLogRecord2->data + redoLogRecord2->fieldLengthsDelta + 6, length1 + length2);
            --redoLogRecord1->fieldCnt;
        }

        // Field list
        fieldCnt = redoLogRecord1->fieldCnt + redoLogRecord2->fieldCnt - 2;
        ctx->write16(mergeBuffer + pos, fieldCnt);
        memcpy(reinterpret_cast<void*>(mergeBuffer + pos + 2),
               reinterpret_cast<const void*>(redoLogRecord1->data + redoLogRecord1->fieldLengthsDelta + 2), redoLogRecord1->fieldCnt * 2);
        memcpy(reinterpret_cast<void*>(mergeBuffer + pos + 2 + redoLogRecord1->fieldCnt * 2),
               reinterpret_cast<const void*>(redoLogRecord2->data + redoLogRecord2->fieldLengthsDelta + 6), redoLogRecord2->fieldCnt * 2 - 4);
        pos += (((fieldCnt + 1) * 2) + 2) & (0xFFFC);
        fieldPos1 = pos;

        memcpy(reinterpret_cast<void*>(mergeBuffer + pos),
               reinterpret_cast<const void*>(redoLogRecord1->data + redoLogRecord1->fieldPos), redoLogRecord1->length - redoLogRecord1->fieldPos);
        pos += (redoLogRecord1->length - redoLogRecord1->fieldPos + 3) & (0xFFFC);
        fieldPos2 = redoLogRecord2->fieldPos +
                    ((ctx->read16(redoLogRecord2->data + redoLogRecord2->fieldLengthsDelta + 2) + 3) & 0xFFFC) +
                    ((ctx->read16(redoLogRecord2->data + redoLogRecord2->fieldLengthsDelta + 4) + 3) & 0xFFFC);

        memcpy(reinterpret_cast<void*>(mergeBuffer + pos),
               reinterpret_cast<const void*>(redoLogRecord2->data + fieldPos2), redoLogRecord2->length - fieldPos2);
        pos += (redoLogRecord2->length - fieldPos2 + 3) & (0xFFFC);

        redoLogRecord1->length = pos;
        redoLogRecord1->fieldCnt = fieldCnt;
        redoLogRecord1->fieldPos = fieldPos1;
        redoLogRecord1->data = mergeBuffer;
        redoLogRecord1->flg |= redoLogRecord2->flg;
        if ((redoLogRecord1->flg & FLG_MULTIBLOCKUNDOTAIL) != 0)
            redoLogRecord1->flg &= ~(FLG_MULTIBLOCKUNDOHEAD | FLG_MULTIBLOCKUNDOMID | FLG_MULTIBLOCKUNDOTAIL);
    }

    void TransactionBuffer::checkpoint(typeSeq& minSequence, uint64_t& minOffset, typeXid& minXid) {
        for (auto xidTransactionMapIt : xidTransactionMap) {
            Transaction* transaction = xidTransactionMapIt.second;
            if (transaction->firstSequence < minSequence) {
                minSequence = transaction->firstSequence;
                minOffset = transaction->firstOffset;
                minXid = transaction->xid;
            } else if (transaction->firstSequence == minSequence && transaction->firstOffset < minOffset) {
                minOffset = transaction->firstOffset;
                minXid = transaction->xid;
            }
        }
    }

    void TransactionBuffer::addOrphanedLob(RedoLogRecord* redoLogRecord1) {
        if (ctx->trace & TRACE_LOB)
            ctx->logTrace(TRACE_LOB, "id: " + redoLogRecord1->lobId.upper() + " page: " + std::to_string(redoLogRecord1->dba) +
                          " can't match, offset: " + std::to_string(redoLogRecord1->dataOffset));

        LobKey lobKey(redoLogRecord1->lobId, redoLogRecord1->dba);

        if (orphanedLobs.find(lobKey) != orphanedLobs.end()) {
            ctx->warning(60009, "duplicate orphaned lob: " + redoLogRecord1->lobId.lower() + ", page: " +
                         std::to_string(redoLogRecord1->dba));
            return;
        }

        orphanedLobs.insert_or_assign(lobKey, allocateLob(redoLogRecord1));
    }

    uint8_t* TransactionBuffer::allocateLob(RedoLogRecord* redoLogRecord1) {
        uint64_t length = redoLogRecord1->length + sizeof(RedoLogRecord) + sizeof(uint64_t);
        uint8_t* data = new uint8_t[length];
        *(reinterpret_cast<uint64_t*>(data)) = length;
        memcpy(reinterpret_cast<void*>(data + sizeof(uint64_t)),
               reinterpret_cast<const void*>(redoLogRecord1), sizeof(RedoLogRecord));
        memcpy(reinterpret_cast<void*>(data + sizeof(uint64_t) + sizeof(RedoLogRecord)),
               reinterpret_cast<const void*>(redoLogRecord1->data), redoLogRecord1->length);
        redoLogRecord1 = reinterpret_cast<RedoLogRecord*>(data + sizeof(uint64_t));
        redoLogRecord1->data = data + sizeof(uint64_t) + sizeof(RedoLogRecord);

        return data;
    }
}
