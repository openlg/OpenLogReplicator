/* Header for OpCode050B class
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

#include "OpCode.h"

#ifndef OP_CODE_05_0B_H_
#define OP_CODE_05_0B_H_

namespace OpenLogReplicator {
    class OpCode050B final : public OpCode {
    protected:
        static void init(Ctx* ctx, RedoLogRecord* redoLogRecord);

    public:
        static void process(Ctx* ctx, RedoLogRecord* redoLogRecord);
    };
}

#endif
