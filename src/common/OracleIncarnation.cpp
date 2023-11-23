/* Information about incarnation in Oracle database
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

#include "OracleIncarnation.h"

namespace OpenLogReplicator {
    OracleIncarnation::OracleIncarnation(uint32_t newIncarnation, typeScn newResetlogsScn, typeScn newPriorResetlogsScn, const char* newStatus,
                                         typeResetlogs newResetlogs, uint32_t newPriorIncarnation) :
        incarnation(newIncarnation),
        resetlogsScn(newResetlogsScn),
        priorResetlogsScn(newPriorResetlogsScn),
        status(newStatus),
        resetlogs(newResetlogs),
        priorIncarnation(newPriorIncarnation) {

        if (this->status == "CURRENT")
            current = true;
        else
            current = false;
    }

    std::ostream& operator<<(std::ostream& os, const OracleIncarnation& i) {
        os << "(" << std::dec << i.incarnation << ", " << i.resetlogsScn << ", " << i.priorResetlogsScn  << ", " << i.status << ", " << i.resetlogs << ", " <<
                i.priorIncarnation << ")";
        return os;
    }
}
