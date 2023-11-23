/* Oracle Redo OpCode: 5.14
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
#include "OpCode0514.h"
#include "Transaction.h"

namespace OpenLogReplicator {
    void OpCode0514::process(Ctx* ctx, RedoLogRecord* redoLogRecord, Transaction* transaction) {
        OpCode::process(ctx, redoLogRecord);

        if (transaction == nullptr) {
            ctx->logTrace(TRACE_TRANSACTION, "attributes with no transaction, offset: " + std::to_string(redoLogRecord->dataOffset));
            return;
        }
        uint64_t fieldPos = 0;
        typeField fieldNum = 0;
        uint16_t fieldLength = 0;

        RedoLogRecord::nextField(ctx, redoLogRecord, fieldNum, fieldPos, fieldLength, 0x051401);
        // Field: 1
        attributeSessionSerial(ctx, redoLogRecord, fieldPos, fieldLength, transaction);

        if (!RedoLogRecord::nextFieldOpt(ctx, redoLogRecord, fieldNum, fieldPos, fieldLength, 0x051402))
            return;
        // Field: 2
        attribute(ctx, redoLogRecord, fieldPos, fieldLength, "transaction name = ", "transaction name", transaction);

        if (!RedoLogRecord::nextFieldOpt(ctx, redoLogRecord, fieldNum, fieldPos, fieldLength, 0x051403))
            return;
        // Field: 3
        attributeFlags(ctx, redoLogRecord, fieldPos, fieldLength, transaction);

        if (!RedoLogRecord::nextFieldOpt(ctx, redoLogRecord, fieldNum, fieldPos, fieldLength, 0x051404))
            return;
        // Field: 4
        attributeVersion(ctx, redoLogRecord, fieldPos, fieldLength, transaction);

        if (!RedoLogRecord::nextFieldOpt(ctx, redoLogRecord, fieldNum, fieldPos, fieldLength, 0x051405))
            return;
        // Field: 5
        attributeAuditSessionId(ctx, redoLogRecord, fieldPos, fieldLength, transaction);

        if (!RedoLogRecord::nextFieldOpt(ctx, redoLogRecord, fieldNum, fieldPos, fieldLength, 0x051406))
            return;
        // Field: 6

        if (!RedoLogRecord::nextFieldOpt(ctx, redoLogRecord, fieldNum, fieldPos, fieldLength, 0x051407))
            return;
        // Field: 7
        attribute(ctx, redoLogRecord, fieldPos, fieldLength, "Client Id = ", "client id", transaction);

        if (!RedoLogRecord::nextFieldOpt(ctx, redoLogRecord, fieldNum, fieldPos, fieldLength, 0x051408))
            return;
        // Field: 8
        attribute(ctx, redoLogRecord, fieldPos, fieldLength, "login   username = ", "login username", transaction);
    }
}
