/* Oracle Redo OpCode: 5.4
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

#include "../common/RedoLogRecord.h"
#include "OpCode0504.h"

namespace OpenLogReplicator {
    void OpCode0504::process(Ctx* ctx, RedoLogRecord* redoLogRecord) {
        OpCode::process(ctx, redoLogRecord);
        uint64_t fieldPos = 0;
        typeField fieldNum = 0;
        uint16_t fieldLength = 0;

        RedoLogRecord::nextField(ctx, redoLogRecord, fieldNum, fieldPos, fieldLength, 0x050401);
        // Field: 1
        ktucm(ctx, redoLogRecord, fieldPos, fieldLength);

        if (!RedoLogRecord::nextFieldOpt(ctx, redoLogRecord, fieldNum, fieldPos, fieldLength, 0x050402))
            return;
        // Field: 2
        if ((redoLogRecord->flg & FLG_KTUCF_OP0504) != 0)
            ktucf(ctx, redoLogRecord, fieldPos, fieldLength);

        if (ctx->dumpRedoLog >= 1) {
            ctx->dumpStream << '\n';
            if ((redoLogRecord->flg & FLG_ROLLBACK_OP0504) != 0)
                ctx->dumpStream << "rolled back transaction\n";
        }
    }

    void OpCode0504::ktucm(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        if (fieldLength < 20)
            throw RedoLogException(50061, "too short field ktucm: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->xid = typeXid(redoLogRecord->usn,
                                     ctx->read16(redoLogRecord->data + fieldPos + 0),
                                     ctx->read32(redoLogRecord->data + fieldPos + 4));
        redoLogRecord->flg = redoLogRecord->data[fieldPos + 16];

        if (ctx->dumpRedoLog >= 1) {
            uint16_t srt = ctx->read16(redoLogRecord->data + fieldPos + 8);  // TODO: find field position/size
            uint32_t sta = ctx->read32(redoLogRecord->data + fieldPos + 12);

            ctx->dumpStream << "ktucm redo: slt: 0x" << std::setfill('0') << std::setw(4) << std::hex <<
                    static_cast<uint64_t>(redoLogRecord->xid.slt()) <<
                    " sqn: 0x" << std::setfill('0') << std::setw(8) << std::hex << redoLogRecord->xid.sqn() <<
                    " srt: " << std::dec << srt <<
                    " sta: " << std::dec << sta <<
                    " flg: 0x" << std::hex << redoLogRecord->flg << " ";
        }
    }

    void OpCode0504::ktucf(Ctx* ctx, RedoLogRecord* redoLogRecord, uint64_t& fieldPos, uint16_t& fieldLength) {
        if (fieldLength < 16)
            throw RedoLogException(50061, "too short field ktucf: " + std::to_string(fieldLength) + " offset: " +
                                   std::to_string(redoLogRecord->dataOffset));

        redoLogRecord->uba = ctx->read56(redoLogRecord->data + fieldPos + 0);

        if (ctx->dumpRedoLog >= 1) {
            uint16_t ext = ctx->read16(redoLogRecord->data + fieldPos + 8);
            uint16_t spc = ctx->read16(redoLogRecord->data + fieldPos + 10);
            uint8_t fbi = redoLogRecord->data[fieldPos + 12];

            ctx->dumpStream << "ktucf redo:" <<
                    " uba: " << PRINTUBA(redoLogRecord->uba) <<
                    " ext: " << std::dec << ext <<
                    " spc: " << std::dec << spc <<
                    " fbi: " << std::dec << static_cast<uint64_t>(fbi) <<
                    " ";
        }
    }
}
