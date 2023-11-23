/* Header for OpCode0514 class
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

#include "OpCode0513.h"

#ifndef OPCODE0514_H_
#define OPCODE0514_H_

namespace OpenLogReplicator {
    class Transaction;

    class OpCode0514 final : public OpCode0513 {
    public:
        static void process(Ctx* ctx, RedoLogRecord* redoLogRecord, Transaction* transaction);
    };
}

#endif
