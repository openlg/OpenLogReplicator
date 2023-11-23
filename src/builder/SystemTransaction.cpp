/* System transaction to change metadata
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

#include "../common/OracleColumn.h"
#include "../common/OracleTable.h"
#include "../common/RuntimeException.h"
#include "../common/SysCCol.h"
#include "../common/SysCDef.h"
#include "../common/SysCol.h"
#include "../common/SysDeferredStg.h"
#include "../common/SysECol.h"
#include "../common/SysLob.h"
#include "../common/SysLobCompPart.h"
#include "../common/SysLobFrag.h"
#include "../common/SysObj.h"
#include "../common/SysTab.h"
#include "../common/SysTabComPart.h"
#include "../common/SysTabPart.h"
#include "../common/SysTabSubPart.h"
#include "../common/SysUser.h"
#include "../metadata/Metadata.h"
#include "../metadata/Schema.h"
#include "../metadata/SchemaElement.h"
#include "Builder.h"
#include "SystemTransaction.h"

namespace OpenLogReplicator {

    SystemTransaction::SystemTransaction(Builder* newBuilder, Metadata* newMetadata) :
            ctx(newMetadata->ctx),
            builder(newBuilder),
            metadata(newMetadata),
            sysCColTmp(nullptr),
            sysCDefTmp(nullptr),
            sysColTmp(nullptr),
            sysDeferredStgTmp(nullptr),
            sysEColTmp(nullptr),
            sysLobTmp(nullptr),
            sysLobCompPartTmp(nullptr),
            sysLobFragTmp(nullptr),
            sysObjTmp(nullptr),
            sysTabTmp(nullptr),
            sysTabComPartTmp(nullptr),
            sysTabPartTmp(nullptr),
            sysTabSubPartTmp(nullptr),
            sysTsTmp(nullptr),
            sysUserTmp(nullptr) {
        ctx->logTrace(TRACE_SYSTEM, "begin");
    }

    SystemTransaction::~SystemTransaction() {
        if (sysCColTmp != nullptr) {
            delete sysCColTmp;
            sysCColTmp = nullptr;
        }

        if (sysCColTmp != nullptr) {
            delete sysCColTmp;
            sysCColTmp = nullptr;
        }

        if (sysCDefTmp != nullptr) {
            delete sysCDefTmp;
            sysCDefTmp = nullptr;
        }

        if (sysColTmp != nullptr) {
            delete sysColTmp;
            sysColTmp = nullptr;
        }

        if (sysDeferredStgTmp != nullptr) {
            delete sysDeferredStgTmp;
            sysDeferredStgTmp = nullptr;
        }

        if (sysEColTmp != nullptr) {
            delete sysEColTmp;
            sysEColTmp = nullptr;
        }

        if (sysLobTmp != nullptr) {
            delete sysLobTmp;
            sysLobTmp = nullptr;
        }

        if (sysLobCompPartTmp != nullptr) {
            delete sysLobCompPartTmp;
            sysLobCompPartTmp = nullptr;
        }

        if (sysLobFragTmp != nullptr) {
            delete sysLobFragTmp;
            sysLobFragTmp = nullptr;
        }

        if (sysObjTmp != nullptr) {
            delete sysObjTmp;
            sysObjTmp = nullptr;
        }

        if (sysTabTmp != nullptr) {
            delete sysTabTmp;
            sysTabTmp = nullptr;
        }

        if (sysTabComPartTmp != nullptr) {
            delete sysTabComPartTmp;
            sysTabComPartTmp = nullptr;
        }

        if (sysTabPartTmp != nullptr) {
            delete sysTabPartTmp;
            sysTabPartTmp = nullptr;
        }

        if (sysTabSubPartTmp != nullptr) {
            delete sysTabSubPartTmp;
            sysTabSubPartTmp = nullptr;
        }

        if (sysTsTmp != nullptr) {
            delete sysTsTmp;
            sysTsTmp = nullptr;
        }

        if (sysUserTmp != nullptr) {
            delete sysUserTmp;
            sysUserTmp = nullptr;
        }
    }

    void SystemTransaction::updateNumber16(int16_t& val, int16_t defVal, typeCol column, OracleTable* table, uint64_t offset) {
        if (builder->values[column][VALUE_AFTER] != nullptr && builder->lengths[column][VALUE_AFTER] > 0) {
            char* retPtr;
            if (table->columns[column]->type != 2)
                throw RuntimeException(50019, "ddl: column type mismatch for " + table->owner + "." + table->name + ": column " +
                                       table->columns[column]->name + " type found " + std::to_string(table->columns[column]->type) + " offset: " +
                                       std::to_string(offset));

            builder->parseNumber(builder->values[column][VALUE_AFTER], builder->lengths[column][VALUE_AFTER], offset);
            builder->valueBuffer[builder->valueLength] = 0;
            auto newVal = (int16_t)strtol(builder->valueBuffer, &retPtr, 10);
            if (ctx->trace & TRACE_SYSTEM)
                ctx->logTrace(TRACE_SYSTEM, "set (" + table->columns[column]->name + ": " + std::to_string(val) + " -> " +
                              std::to_string(newVal) + ")");
            val = newVal;
        } else if (builder->values[column][VALUE_AFTER] != nullptr || builder->values[column][VALUE_BEFORE] != nullptr) {
            if (ctx->trace & TRACE_SYSTEM)
                ctx->logTrace(TRACE_SYSTEM, "set (" + table->columns[column]->name + ": " + std::to_string(val) + " -> NULL)");
            val = defVal;
        }
    }

    void SystemTransaction::updateNumber16u(uint16_t& val, uint16_t defVal, typeCol column, OracleTable* table, uint64_t offset) {
        if (builder->values[column][VALUE_AFTER] != nullptr && builder->lengths[column][VALUE_AFTER] > 0) {
            char* retPtr;
            if (table->columns[column]->type != 2)
                throw RuntimeException(50019, "ddl: column type mismatch for " + table->owner + "." + table->name + ": column " +
                                       table->columns[column]->name + " type found " + std::to_string(table->columns[column]->type) + " offset: " +
                                       std::to_string(offset));

            builder->parseNumber(builder->values[column][VALUE_AFTER], builder->lengths[column][VALUE_AFTER], offset);
            builder->valueBuffer[builder->valueLength] = 0;
            if (builder->valueLength == 0 || (builder->valueLength > 0 && builder->valueBuffer[0] == '-'))
                throw RuntimeException(50020, "ddl: column type mismatch for " + table->owner + "." + table->name + ": column " +
                                       table->columns[column]->name + " value found " + builder->valueBuffer + " offset: " + std::to_string(offset));

            uint16_t newVal = strtoul(builder->valueBuffer, &retPtr, 10);
            if (ctx->trace & TRACE_SYSTEM)
                ctx->logTrace(TRACE_SYSTEM, "set (" + table->columns[column]->name + ": " + std::to_string(val) + " -> " +
                              std::to_string(newVal) + ")");
            val = newVal;
        } else if (builder->values[column][VALUE_AFTER] != nullptr || builder->values[column][VALUE_BEFORE] != nullptr) {
            if (ctx->trace & TRACE_SYSTEM)
                ctx->logTrace(TRACE_SYSTEM, "set (" + table->columns[column]->name + ": " + std::to_string(val) + " -> NULL)");
            val = defVal;
        }
    }

    void SystemTransaction::updateNumber32u(uint32_t& val, uint32_t defVal, typeCol column, OracleTable* table, uint64_t offset) {
        if (builder->values[column][VALUE_AFTER] != nullptr && builder->lengths[column][VALUE_AFTER] > 0) {
            char* retPtr;
            if (table->columns[column]->type != 2)
                throw RuntimeException(50019, "ddl: column type mismatch for " + table->owner + "." + table->name + ": column " +
                                       table->columns[column]->name + " type found " + std::to_string(table->columns[column]->type) + " offset: " +
                                       std::to_string(offset));

            builder->parseNumber(builder->values[column][VALUE_AFTER], builder->lengths[column][VALUE_AFTER], offset);
            builder->valueBuffer[builder->valueLength] = 0;
            if (builder->valueLength == 0 || (builder->valueLength > 0 && builder->valueBuffer[0] == '-'))
                throw RuntimeException(50020, "ddl: column type mismatch for " + table->owner + "." + table->name + ": column " +
                                       table->columns[column]->name + " value found " + builder->valueBuffer + " offset: " + std::to_string(offset));

            uint32_t newVal = strtoul(builder->valueBuffer, &retPtr, 10);
            if (ctx->trace & TRACE_SYSTEM)
                ctx->logTrace(TRACE_SYSTEM, "set (" + table->columns[column]->name + ": " + std::to_string(val) + " -> " +
                              std::to_string(newVal) + ")");
            val = newVal;
        } else if (builder->values[column][VALUE_AFTER] != nullptr || builder->values[column][VALUE_BEFORE] != nullptr) {
            if (ctx->trace & TRACE_SYSTEM)
                ctx->logTrace(TRACE_SYSTEM, "set (" + table->columns[column]->name + ": " + std::to_string(val) + " -> NULL)");
            val = defVal;
        }
    }

    void SystemTransaction::updateNumber64(int64_t& val, int64_t defVal, typeCol column, OracleTable* table, uint64_t offset) {
        if (builder->values[column][VALUE_AFTER] != nullptr && builder->lengths[column][VALUE_AFTER] > 0) {
            char* retPtr;
            if (table->columns[column]->type != 2)
                throw RuntimeException(50019, "ddl: column type mismatch for " + table->owner + "." + table->name + ": column " +
                                       table->columns[column]->name + " type found " + std::to_string(table->columns[column]->type) + " offset: " +
                                       std::to_string(offset));

            builder->parseNumber(builder->values[column][VALUE_AFTER], builder->lengths[column][VALUE_AFTER], offset);
            builder->valueBuffer[builder->valueLength] = 0;
            if (builder->valueLength == 0)
                throw RuntimeException(50020, "ddl: column type mismatch for " + table->owner + "." + table->name + ": column " +
                                       table->columns[column]->name + " value found " + builder->valueBuffer + " offset: " + std::to_string(offset));

            int64_t newVal = strtol(builder->valueBuffer, &retPtr, 10);
            if (ctx->trace & TRACE_SYSTEM)
                ctx->logTrace(TRACE_SYSTEM, "set (" + table->columns[column]->name + ": " + std::to_string(val) + " -> " +
                              std::to_string(newVal) + ")");
            val = newVal;
        } else if (builder->values[column][VALUE_AFTER] != nullptr || builder->values[column][VALUE_BEFORE] != nullptr) {
            if (ctx->trace & TRACE_SYSTEM)
                ctx->logTrace(TRACE_SYSTEM, "set (" + table->columns[column]->name + ": " + std::to_string(val) + " -> NULL)");
            val = defVal;
        }
    }

    void SystemTransaction::updateNumber64u(uint64_t& val, uint64_t defVal, typeCol column, OracleTable* table, uint64_t offset) {
        if (builder->values[column][VALUE_AFTER] != nullptr && builder->lengths[column][VALUE_AFTER] > 0) {
            char* retPtr;
            if (table->columns[column]->type != 2)
                throw RuntimeException(50019, "ddl: column type mismatch for " + table->owner + "." + table->name + ": column " +
                                       table->columns[column]->name + " type found " + std::to_string(table->columns[column]->type) + " offset: " +
                                       std::to_string(offset));

            builder->parseNumber(builder->values[column][VALUE_AFTER], builder->lengths[column][VALUE_AFTER], offset);
            builder->valueBuffer[builder->valueLength] = 0;
            if (builder->valueLength == 0 || (builder->valueLength > 0 && builder->valueBuffer[0] == '-'))
                throw RuntimeException(50020, "ddl: column type mismatch for " + table->owner + "." + table->name + ": column " +
                                       table->columns[column]->name + " value found " + builder->valueBuffer + " offset: " + std::to_string(offset));

            uint64_t newVal = strtoul(builder->valueBuffer, &retPtr, 10);
            if (ctx->trace & TRACE_SYSTEM)
                ctx->logTrace(TRACE_SYSTEM, "set (" + table->columns[column]->name + ": " + std::to_string(val) + " -> " +
                              std::to_string(newVal) + ")");
            val = newVal;
        } else if (builder->values[column][VALUE_AFTER] != nullptr || builder->values[column][VALUE_BEFORE] != nullptr) {
            if (ctx->trace & TRACE_SYSTEM)
                ctx->logTrace(TRACE_SYSTEM, "set (" + table->columns[column]->name + ": " + std::to_string(val) + " -> NULL)");
            val = defVal;
        }
    }

    void SystemTransaction::updateNumberXu(typeIntX& val, typeCol column, OracleTable* table, uint64_t offset) {
        if (builder->values[column][VALUE_AFTER] != nullptr && builder->lengths[column][VALUE_AFTER] > 0) {
            if (table->columns[column]->type != 2)
                throw RuntimeException(50019, "ddl: column type mismatch for " + table->owner + "." + table->name + ": column " +
                                       table->columns[column]->name + " type found " + std::to_string(table->columns[column]->type) + " offset: " +
                                       std::to_string(offset));

            builder->parseNumber(builder->values[column][VALUE_AFTER], builder->lengths[column][VALUE_AFTER], offset);
            builder->valueBuffer[builder->valueLength] = 0;
            if (builder->valueLength == 0 || (builder->valueLength > 0 && builder->valueBuffer[0] == '-'))
                throw RuntimeException(50020, "ddl: column type mismatch for " + table->owner + "." + table->name + ": column " +
                                       table->columns[column]->name + " value found " + builder->valueBuffer + " offset: " + std::to_string(offset));

            typeIntX newVal(0);
            std::string err;
            newVal.setStr(builder->valueBuffer, builder->valueLength, err);
            if (err != "")
                ctx->error(50021, err.c_str());

            if (ctx->trace & TRACE_SYSTEM)
                ctx->logTrace(TRACE_SYSTEM, "set (" + table->columns[column]->name + ": " + val.toString() + " -> " + newVal.toString() + ")");
            val = newVal;
        } else if (builder->values[column][VALUE_AFTER] != nullptr || builder->values[column][VALUE_BEFORE] != nullptr) {
            if (ctx->trace & TRACE_SYSTEM)
                ctx->logTrace(TRACE_SYSTEM, "set (" + table->columns[column]->name + ": " + val.toString() + " -> NULL)");
            val.set(0, 0);
        }
    }

    void SystemTransaction::updateString(std::string& val, uint64_t maxLength, typeCol column, OracleTable* table, uint64_t offset) {
        if (builder->values[column][VALUE_AFTER] != nullptr && builder->lengths[column][VALUE_AFTER] > 0) {
            if (table->columns[column]->type != SYS_COL_TYPE_VARCHAR && table->columns[column]->type != SYS_COL_TYPE_CHAR)
                throw RuntimeException(50019, "ddl: column type mismatch for " + table->owner + "." + table->name + ": column " +
                                       table->columns[column]->name + " type found " + std::to_string(table->columns[column]->type) + " offset: " +
                                       std::to_string(offset));

            builder->parseString(builder->values[column][VALUE_AFTER], builder->lengths[column][VALUE_AFTER],
                                 table->columns[column]->charsetId, offset, false, false, false, true);
            std::string newVal(builder->valueBuffer, builder->valueLength);
            if (builder->valueLength > maxLength)
                throw RuntimeException(50020, "ddl: value too long for " + table->owner + "." + table->name + ": column " +
                                       table->columns[column]->name + ", length " + std::to_string(builder->valueLength) + " offset: " + std::to_string(offset));

            if (ctx->trace & TRACE_SYSTEM)
                ctx->logTrace(TRACE_SYSTEM, "set (" + table->columns[column]->name + ": '" + val + "' -> '" + newVal + "')");
            val = newVal;
        } else if (builder->values[column][VALUE_AFTER] != nullptr || builder->values[column][VALUE_BEFORE] != nullptr) {
            if (ctx->trace & TRACE_SYSTEM)
                ctx->logTrace(TRACE_SYSTEM, "set (" + table->columns[column]->name + ": '" + val + "' -> NULL)");
            val.assign("");
        }
    }

    void SystemTransaction::processInsertSysCCol(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysCCol* sysCCol = metadata->schema->dictSysCColFind(rowId);
        if (sysCCol != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.CCOL$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysCColDrop(sysCCol);
            delete sysCCol;
        }
        sysCColTmp = new SysCCol(rowId, 0, 0, 0, 0, 0);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "CON#") {
                    updateNumber32u(sysCColTmp->con, 0, column, table, offset);
                } else if (table->columns[column]->name == "INTCOL#") {
                    updateNumber16(sysCColTmp->intCol, 0, column, table, offset);
                } else if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysCColTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "SPARE1") {
                    updateNumberXu(sysCColTmp->spare1, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysCColAdd(sysCColTmp);
        sysCColTmp = nullptr;
    }

    void SystemTransaction::processInsertSysCDef(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysCDef* sysCDef = metadata->schema->dictSysCDefFind(rowId);
        if (sysCDef != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.CDEF$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysCDefDrop(sysCDef);
            delete sysCDef;
        }
        sysCDefTmp = new SysCDef(rowId, 0, 0, 0);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "CON#") {
                    updateNumber32u(sysCDefTmp->con, 0, column, table, offset);
                } else if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysCDefTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "TYPE#") {
                    updateNumber16u(sysCDefTmp->type, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysCDefAdd(sysCDefTmp);
        sysCDefTmp = nullptr;
    }

    void SystemTransaction::processInsertSysCol(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysCol* sysCol = metadata->schema->dictSysColFind(rowId);
        if (sysCol != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.COL$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysColDrop(sysCol);
            delete sysCol;
        }
        sysColTmp = new SysCol(rowId, 0, 0, 0, 0, "", 0, 0, -1, -1,
                               0, 0, 0, 0, 0);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysColTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "COL#") {
                    updateNumber16(sysColTmp->col, 0, column, table, offset);
                } else if (table->columns[column]->name == "SEGCOL#") {
                    updateNumber16(sysColTmp->segCol, 0, column, table, offset);
                } else if (table->columns[column]->name == "INTCOL#") {
                    updateNumber16(sysColTmp->intCol, 0, column, table, offset);
                } else if (table->columns[column]->name == "NAME") {
                    updateString(sysColTmp->name, SYS_COL_NAME_LENGTH, column, table, offset);
                } else if (table->columns[column]->name == "TYPE#") {
                    updateNumber16u(sysColTmp->type, 0, column, table, offset);
                } else if (table->columns[column]->name == "LENGTH") {
                    updateNumber64u(sysColTmp->length, 0, column, table, offset);
                } else if (table->columns[column]->name == "PRECISION#") {
                    updateNumber64(sysColTmp->precision, -1, column, table, offset);
                } else if (table->columns[column]->name == "SCALE") {
                    updateNumber64(sysColTmp->scale, -1, column, table, offset);
                } else if (table->columns[column]->name == "CHARSETFORM") {
                    updateNumber64u(sysColTmp->charsetForm, 0, column, table, offset);
                } else if (table->columns[column]->name == "CHARSETID") {
                    updateNumber64u(sysColTmp->charsetId, 0, column, table, offset);
                } else if (table->columns[column]->name == "NULL$") {
                    updateNumber64(sysColTmp->null_, 0, column, table, offset);
                } else if (table->columns[column]->name == "PROPERTY") {
                    updateNumberXu(sysColTmp->property, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysColAdd(sysColTmp);
        sysColTmp = nullptr;
    }

    void SystemTransaction::processInsertSysDeferredStg(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysDeferredStg* sysDeferredStg = metadata->schema->dictSysDeferredStgFind(rowId);
        if (sysDeferredStg != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.DEFERRED_STG$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysDeferredStgDrop(sysDeferredStg);
            delete sysDeferredStg;
        }
        sysDeferredStgTmp = new SysDeferredStg(rowId, 0, 0, 0);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysDeferredStgTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "FLAGS_STG") {
                    updateNumberXu(sysDeferredStgTmp->flagsStg, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysDeferredStgAdd(sysDeferredStgTmp);
        sysDeferredStgTmp = nullptr;
    }

    void SystemTransaction::processInsertSysECol(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysECol* sysECol = metadata->schema->dictSysEColFind(rowId);
        if (sysECol != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.ECOL$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysEColDrop(sysECol);
            delete sysECol;
        }
        sysEColTmp = new SysECol(rowId, 0, 0, -1);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "TABOBJ#") {
                    updateNumber32u(sysEColTmp->tabObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "COLNUM") {
                    updateNumber16(sysEColTmp->colNum, 0, column, table, offset);
                } else if (table->columns[column]->name == "GUARD_ID") {
                    updateNumber16(sysEColTmp->guardId, -1, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysEColAdd(sysEColTmp);
        sysEColTmp = nullptr;
    }

    void SystemTransaction::processInsertSysLob(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysLob* sysLob = metadata->schema->dictSysLobFind(rowId);
        if (sysLob != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.LOB$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysLobDrop(sysLob);
            delete sysLob;
        }
        sysLobTmp = new SysLob(rowId, 0, 0, 0, 0, 0);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysLobTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "COL#") {
                    updateNumber16(sysLobTmp->col, 0, column, table, offset);
                } else if (table->columns[column]->name == "INTCOL#") {
                    updateNumber16(sysLobTmp->intCol, 0, column, table, offset);
                } else if (table->columns[column]->name == "LOBJ#") {
                    updateNumber32u(sysLobTmp->lObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "TS#") {
                    updateNumber32u(sysLobTmp->ts, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysLobAdd(sysLobTmp);
        sysLobTmp = nullptr;
    }

    void SystemTransaction::processInsertSysLobCompPart(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysLobCompPart* sysLobCompPart = metadata->schema->dictSysLobCompPartFind(rowId);
        if (sysLobCompPart != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.LOBCOMPPART$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysLobCompPartDrop(sysLobCompPart);
            delete sysLobCompPart;
        }
        sysLobCompPartTmp = new SysLobCompPart(rowId, 0, 0);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "PARTOBJ#") {
                    updateNumber32u(sysLobCompPartTmp->partObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "LOBJ#") {
                    updateNumber32u(sysLobCompPartTmp->lObj, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysLobCompPartAdd(sysLobCompPartTmp);
        sysLobCompPartTmp = nullptr;
    }

    void SystemTransaction::processInsertSysLobFrag(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysLobFrag* sysLobFrag = metadata->schema->dictSysLobFragFind(rowId);
        if (sysLobFrag != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.LOBFRAG$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysLobFragDrop(sysLobFrag);
            delete sysLobFrag;
        }
        sysLobFragTmp = new SysLobFrag(rowId, 0, 0, 0);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "FRAGOBJ#") {
                    updateNumber32u(sysLobFragTmp->fragObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "PARENTOBJ#") {
                    updateNumber32u(sysLobFragTmp->parentObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "TS#") {
                    updateNumber32u(sysLobFragTmp->ts, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysLobFragAdd(sysLobFragTmp);
        sysLobFragTmp = nullptr;
    }

    void SystemTransaction::processInsertSysObj(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysObj* sysObj = metadata->schema->dictSysObjFind(rowId);
        if (sysObj != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.OBJ$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysObjDrop(sysObj);
            delete sysObj;
        }
        sysObjTmp = new SysObj(rowId, 0, 0, 0, 0, "", 0, 0, false);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OWNER#") {
                    updateNumber32u(sysObjTmp->owner, 0, column, table, offset);
                } else if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysObjTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "DATAOBJ#") {
                    updateNumber32u(sysObjTmp->dataObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "NAME") {
                    updateString(sysObjTmp->name, SYS_OBJ_NAME_LENGTH, column, table, offset);
                } else if (table->columns[column]->name == "TYPE#") {
                    updateNumber16u(sysObjTmp->type, 0, column, table, offset);
                } else if (table->columns[column]->name == "FLAGS") {
                    updateNumberXu(sysObjTmp->flags, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysObjAdd(sysObjTmp);
        sysObjTmp = nullptr;
    }

    void SystemTransaction::processInsertSysTab(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysTab* sysTab = metadata->schema->dictSysTabFind(rowId);
        if (sysTab != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.TAB$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysTabDrop(sysTab);
            delete sysTab;
        }
        sysTabTmp = new SysTab(rowId, 0, 0, 0, 0, 0, 0, 0, 0);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysTabTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "DATAOBJ#") {
                    updateNumber32u(sysTabTmp->dataObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "TS#") {
                    updateNumber32u(sysTabTmp->ts, 0, column, table, offset);
                } else if (table->columns[column]->name == "CLUCOLS") {
                    updateNumber16(sysTabTmp->cluCols, 0, column, table, offset);
                } else if (table->columns[column]->name == "FLAGS") {
                    updateNumberXu(sysTabTmp->flags, column, table, offset);
                } else if (table->columns[column]->name == "PROPERTY") {
                    updateNumberXu(sysTabTmp->property, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysTabAdd(sysTabTmp);
        sysTabTmp = nullptr;
    }

    void SystemTransaction::processInsertSysTabComPart(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysTabComPart* sysTabComPart = metadata->schema->dictSysTabComPartFind(rowId);
        if (sysTabComPart != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.TABCOMPART$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysTabComPartDrop(sysTabComPart);
            delete sysTabComPart;
        }
        sysTabComPartTmp = new SysTabComPart(rowId, 0, 0, 0);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysTabComPartTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "DATAOBJ#") {
                    updateNumber32u(sysTabComPartTmp->dataObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "BO#") {
                    updateNumber32u(sysTabComPartTmp->bo, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysTabComPartAdd(sysTabComPartTmp);
        sysTabComPartTmp = nullptr;
    }

    void SystemTransaction::processInsertSysTabPart(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysTabPart* sysTabPart = metadata->schema->dictSysTabPartFind(rowId);
        if (sysTabPart != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.TABPART$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysTabPartDrop(sysTabPart);
            delete sysTabPart;
        }
        sysTabPartTmp = new SysTabPart(rowId, 0, 0, 0);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysTabPartTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "DATAOBJ#") {
                    updateNumber32u(sysTabPartTmp->dataObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "BO#") {
                    updateNumber32u(sysTabPartTmp->bo, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysTabPartAdd(sysTabPartTmp);
        sysTabPartTmp = nullptr;
    }

    void SystemTransaction::processInsertSysTabSubPart(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysTabSubPart* sysTabSubPart = metadata->schema->dictSysTabSubPartFind(rowId);
        if (sysTabSubPart != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.TABSUBPART$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysTabSubPartDrop(sysTabSubPart);
            delete sysTabSubPart;
        }
        sysTabSubPartTmp = new SysTabSubPart(rowId, 0, 0, 0);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysTabSubPartTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "DATAOBJ#") {
                    updateNumber32u(sysTabSubPartTmp->dataObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "POBJ#") {
                    updateNumber32u(sysTabSubPartTmp->pObj, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysTabSubPartAdd(sysTabSubPartTmp);
        sysTabSubPartTmp = nullptr;
    }

    void SystemTransaction::processInsertSysTs(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysTs* sysTs = metadata->schema->dictSysTsFind(rowId);
        if (sysTs != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.TS$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysTsDrop(sysTs);
            delete sysTs;
        }
        sysTsTmp = new SysTs(rowId, 0, "", 0);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "TS#") {
                    updateNumber32u(sysTsTmp->ts, 0, column, table, offset);
                } else if (table->columns[column]->name == "NAME") {
                    updateString(sysTsTmp->name, SYS_TS_NAME_LENGTH, column, table, offset);
                } else if (table->columns[column]->name == "BLOCKSIZE") {
                    updateNumber32u(sysTsTmp->blockSize, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysTsAdd(sysTsTmp);
        sysTsTmp = nullptr;
    }

    void SystemTransaction::processInsertSysUser(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        SysUser* sysUser = metadata->schema->dictSysUserFind(rowId);
        if (sysUser != nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA))
                throw RuntimeException(50022, "ddl: duplicate SYS.USER$: (rowid: " + rowId.toString() + ") for insert at offset: " +
                                       std::to_string(offset));
            metadata->schema->dictSysUserDrop(sysUser);
            delete sysUser;
        }
        sysUserTmp = new SysUser(rowId, 0, "", 0, 0, false);

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "USER#") {
                    updateNumber32u(sysUserTmp->user, 0, column, table, offset);
                } else if (table->columns[column]->name == "NAME") {
                    updateString(sysUserTmp->name, SYS_USER_NAME_LENGTH, column, table, offset);
                } else if (table->columns[column]->name == "SPARE1") {
                    updateNumberXu(sysUserTmp->spare1, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysUserAdd(sysUserTmp);
        sysUserTmp = nullptr;
    }

    void SystemTransaction::processInsert(OracleTable* table, typeDataObj dataObj, typeDba bdba, typeSlot slot, uint64_t offset) {
        typeRowId rowId(dataObj, bdba, slot);
        char str[19];
        rowId.toString(str);
        if (ctx->trace & TRACE_SYSTEM)
            ctx->logTrace(TRACE_SYSTEM, "insert table (name: " + table->owner + "." + table->name + ", rowid: " + rowId.toString() + ")");

        switch (table->systemTable) {
            case TABLE_SYS_CCOL:
                processInsertSysCCol(table, rowId, offset);
                break;

            case TABLE_SYS_CDEF:
                processInsertSysCDef(table, rowId, offset);
                break;

            case TABLE_SYS_COL:
                processInsertSysCol(table, rowId, offset);
                break;

            case TABLE_SYS_DEFERRED_STG:
                processInsertSysDeferredStg(table, rowId, offset);
                break;

            case TABLE_SYS_ECOL:
                processInsertSysECol(table, rowId, offset);
                break;

            case TABLE_SYS_LOB:
                processInsertSysLob(table, rowId, offset);
                break;

            case TABLE_SYS_LOB_COMP_PART:
                processInsertSysLobCompPart(table, rowId, offset);
                break;

            case TABLE_SYS_LOB_FRAG:
                processInsertSysLobFrag(table, rowId, offset);
                break;

            case TABLE_SYS_OBJ:
                processInsertSysObj(table, rowId, offset);
                break;

            case TABLE_SYS_TAB:
                processInsertSysTab(table, rowId, offset);
                break;

            case TABLE_SYS_TABCOMPART:
                processInsertSysTabComPart(table, rowId, offset);
                break;

            case TABLE_SYS_TABPART:
                processInsertSysTabPart(table, rowId, offset);
                break;

            case TABLE_SYS_TABSUBPART:
                processInsertSysTabSubPart(table, rowId, offset);
                break;

            case TABLE_SYS_TS:
                processInsertSysTs(table, rowId, offset);
                break;

            case TABLE_SYS_USER:
                processInsertSysUser(table, rowId, offset);
                break;
        }
    }

    void SystemTransaction::processUpdateSysCCol(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysCColTmp = metadata->schema->dictSysCColFind(rowId);
        if (sysCColTmp != nullptr) {
            metadata->schema->dictSysCColDrop(sysCColTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.CCOL$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysCColTmp = new SysCCol(rowId, 0, 0, 0, 0, 0);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "CON#") {
                    updateNumber32u(sysCColTmp->con, 0, column, table, offset);
                } else if (table->columns[column]->name == "INTCOL#") {
                    updateNumber16(sysCColTmp->intCol, 0, column, table, offset);
                } else if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysCColTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "SPARE1") {
                    updateNumberXu(sysCColTmp->spare1, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysCColAdd(sysCColTmp);
        sysCColTmp = nullptr;
    }

    void SystemTransaction::processUpdateSysCDef(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysCDefTmp = metadata->schema->dictSysCDefFind(rowId);
        if (sysCDefTmp != nullptr) {
            metadata->schema->dictSysCDefDrop(sysCDefTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.CDEF$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysCDefTmp = new SysCDef(rowId, 0, 0, 0);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "CON#") {
                    updateNumber32u(sysCDefTmp->con, 0, column, table, offset);
                } else if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysCDefTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "TYPE#") {
                    updateNumber16u(sysCDefTmp->type, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysCDefAdd(sysCDefTmp);
        sysCDefTmp = nullptr;
    }

    void SystemTransaction::processUpdateSysCol(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysColTmp = metadata->schema->dictSysColFind(rowId);
        if (sysColTmp != nullptr) {
            metadata->schema->dictSysColDrop(sysColTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.COL$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysColTmp = new SysCol(rowId, 0, 0, 0, 0, "", 0, 0, -1, -1,
                                   0, 0, 0, 0, 0);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysColTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "COL#") {
                    updateNumber16(sysColTmp->col, 0, column, table, offset);
                } else if (table->columns[column]->name == "SEGCOL#") {
                    updateNumber16(sysColTmp->segCol, 0, column, table, offset);
                } else if (table->columns[column]->name == "INTCOL#") {
                    updateNumber16(sysColTmp->intCol, 0, column, table, offset);
                } else if (table->columns[column]->name == "NAME") {
                    updateString(sysColTmp->name, SYS_COL_NAME_LENGTH, column, table, offset);
                } else if (table->columns[column]->name == "TYPE#") {
                    updateNumber16u(sysColTmp->type, 0, column, table, offset);
                } else if (table->columns[column]->name == "LENGTH") {
                    updateNumber64u(sysColTmp->length, 0, column, table, offset);
                } else if (table->columns[column]->name == "PRECISION#") {
                    updateNumber64(sysColTmp->precision, -1, column, table, offset);
                } else if (table->columns[column]->name == "SCALE") {
                    updateNumber64(sysColTmp->scale, -1, column, table, offset);
                } else if (table->columns[column]->name == "CHARSETFORM") {
                    updateNumber64u(sysColTmp->charsetForm, 0, column, table, offset);
                } else if (table->columns[column]->name == "CHARSETID") {
                    updateNumber64u(sysColTmp->charsetId, 0, column, table, offset);
                } else if (table->columns[column]->name == "NULL$") {
                    updateNumber64(sysColTmp->null_, 0, column, table, offset);
                } else if (table->columns[column]->name == "PROPERTY") {
                    updateNumberXu(sysColTmp->property, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysColAdd(sysColTmp);
        sysColTmp = nullptr;
    }

    void SystemTransaction::processUpdateSysDeferredStg(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysDeferredStgTmp = metadata->schema->dictSysDeferredStgFind(rowId);
        if (sysDeferredStgTmp != nullptr) {
            metadata->schema->dictSysDeferredStgDrop(sysDeferredStgTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.DEFERRED_STG$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysDeferredStgTmp = new SysDeferredStg(rowId, 0, 0, 0);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysDeferredStgTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "FLAGS_STG") {
                    updateNumberXu(sysDeferredStgTmp->flagsStg, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysDeferredStgAdd(sysDeferredStgTmp);
        sysDeferredStgTmp = nullptr;
    }

    void SystemTransaction::processUpdateSysECol(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysEColTmp = metadata->schema->dictSysEColFind(rowId);
        if (sysEColTmp != nullptr) {
            metadata->schema->dictSysEColDrop(sysEColTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.ECOL$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysEColTmp = new SysECol(rowId, 0, 0, -1);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "TABOBJ#") {
                    updateNumber32u(sysEColTmp->tabObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "COLNUM") {
                    updateNumber16(sysEColTmp->colNum, 0, column, table, offset);
                } else if (table->columns[column]->name == "GUARD_ID") {
                    updateNumber16(sysEColTmp->guardId, -1, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysEColAdd(sysEColTmp);
        sysEColTmp = nullptr;
    }

    void SystemTransaction::processUpdateSysLob(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysLobTmp = metadata->schema->dictSysLobFind(rowId);
        if (sysLobTmp != nullptr) {
            metadata->schema->dictSysLobDrop(sysLobTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.LOB$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysLobTmp = new SysLob(rowId, 0, 0, 0, 0, 0);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysLobTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "COL#") {
                    updateNumber16(sysLobTmp->col, 0, column, table, offset);
                } else if (table->columns[column]->name == "INTCOL#") {
                    updateNumber16(sysLobTmp->intCol, 0, column, table, offset);
                } else if (table->columns[column]->name == "LOBJ#") {
                    updateNumber32u(sysLobTmp->lObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "TS#") {
                    updateNumber32u(sysLobTmp->ts, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysLobAdd(sysLobTmp);
        sysLobTmp = nullptr;
    }

    void SystemTransaction::processUpdateSysLobCompPart(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysLobCompPartTmp = metadata->schema->dictSysLobCompPartFind(rowId);
        if (sysLobCompPartTmp != nullptr) {
            metadata->schema->dictSysLobCompPartDrop(sysLobCompPartTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.LOBCOMPPART$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysLobCompPartTmp = new SysLobCompPart(rowId, 0, 0);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "PARTOBJ#") {
                    updateNumber32u(sysLobCompPartTmp->partObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "LOBJ#") {
                    updateNumber32u(sysLobCompPartTmp->lObj, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysLobCompPartAdd(sysLobCompPartTmp);
        sysLobCompPartTmp = nullptr;
    }

    void SystemTransaction::processUpdateSysLobFrag(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysLobFragTmp = metadata->schema->dictSysLobFragFind(rowId);
        if (sysLobFragTmp != nullptr) {
            metadata->schema->dictSysLobFragDrop(sysLobFragTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.LOBFRAG$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysLobFragTmp = new SysLobFrag(rowId, 0, 0, 0);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "FRAGOBJ#") {
                    updateNumber32u(sysLobFragTmp->fragObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "PARENTOBJ#") {
                    updateNumber32u(sysLobFragTmp->parentObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "TS#") {
                    updateNumber32u(sysLobFragTmp->ts, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysLobFragAdd(sysLobFragTmp);
        sysLobFragTmp = nullptr;
    }

    void SystemTransaction::processUpdateSysObj(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysObjTmp = metadata->schema->dictSysObjFind(rowId);
        if (sysObjTmp != nullptr) {
            metadata->schema->dictSysObjDrop(sysObjTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.OBJ$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysObjTmp = new SysObj(rowId, 0, 0, 0, 0, "", 0, 0, false);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OWNER#") {
                    updateNumber32u(sysObjTmp->owner, 0, column, table, offset);
                } else if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysObjTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "DATAOBJ#") {
                    updateNumber32u(sysObjTmp->dataObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "NAME") {
                    updateString(sysObjTmp->name, SYS_OBJ_NAME_LENGTH, column, table, offset);
                } else if (table->columns[column]->name == "TYPE#") {
                    updateNumber16u(sysObjTmp->type, 0, column, table, offset);
                } else if (table->columns[column]->name == "FLAGS") {
                    updateNumberXu(sysObjTmp->flags, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysObjAdd(sysObjTmp);
        sysObjTmp = nullptr;
    }

    void SystemTransaction::processUpdateSysTab(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysTabTmp = metadata->schema->dictSysTabFind(rowId);
        if (sysTabTmp != nullptr) {
            metadata->schema->dictSysTabDrop(sysTabTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.TAB$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysTabTmp = new SysTab(rowId, 0, 0, 0, 0, 0, 0, 0, 0);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysTabTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "DATAOBJ#") {
                    updateNumber32u(sysTabTmp->dataObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "TS#") {
                    updateNumber32u(sysTabTmp->ts, 0, column, table, offset);
                } else if (table->columns[column]->name == "CLUCOLS") {
                    updateNumber16(sysTabTmp->cluCols, 0, column, table, offset);
                } else if (table->columns[column]->name == "FLAGS") {
                    updateNumberXu(sysTabTmp->flags, column, table, offset);
                } else if (table->columns[column]->name == "PROPERTY") {
                    updateNumberXu(sysTabTmp->property, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysTabAdd(sysTabTmp);
        sysTabTmp = nullptr;
    }

    void SystemTransaction::processUpdateSysTabComPart(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysTabComPartTmp = metadata->schema->dictSysTabComPartFind(rowId);
        if (sysTabComPartTmp != nullptr) {
            metadata->schema->dictSysTabComPartDrop(sysTabComPartTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.TABCOMPART$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysTabComPartTmp = new SysTabComPart(rowId, 0, 0, 0);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysTabComPartTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "DATAOBJ#") {
                    updateNumber32u(sysTabComPartTmp->dataObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "BO#") {
                    updateNumber32u(sysTabComPartTmp->bo, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysTabComPartAdd(sysTabComPartTmp);
        sysTabComPartTmp = nullptr;
    }

    void SystemTransaction::processUpdateSysTabPart(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysTabPartTmp = metadata->schema->dictSysTabPartFind(rowId);
        if (sysTabPartTmp != nullptr) {
            metadata->schema->dictSysTabPartDrop(sysTabPartTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.TABPART$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysTabPartTmp = new SysTabPart(rowId, 0, 0, 0);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysTabPartTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "DATAOBJ#") {
                    updateNumber32u(sysTabPartTmp->dataObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "BO#") {
                    updateNumber32u(sysTabPartTmp->bo, 0, column, table, offset);
                }
            }
        }
        metadata->schema->dictSysTabPartAdd(sysTabPartTmp);
        sysTabPartTmp = nullptr;
    }

    void SystemTransaction::processUpdateSysTabSubPart(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysTabSubPartTmp = metadata->schema->dictSysTabSubPartFind(rowId);
        if (sysTabSubPartTmp != nullptr) {
            metadata->schema->dictSysTabSubPartDrop(sysTabSubPartTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.TABSUBPART$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysTabSubPartTmp = new SysTabSubPart(rowId, 0, 0, 0);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "OBJ#") {
                    updateNumber32u(sysTabSubPartTmp->obj, 0, column, table, offset);
                } else if (table->columns[column]->name == "DATAOBJ#") {
                    updateNumber32u(sysTabSubPartTmp->dataObj, 0, column, table, offset);
                } else if (table->columns[column]->name == "POBJ#") {
                    updateNumber32u(sysTabSubPartTmp->pObj, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysTabSubPartAdd(sysTabSubPartTmp);
        sysTabSubPartTmp = nullptr;
    }

    void SystemTransaction::processUpdateSysTs(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysTsTmp = metadata->schema->dictSysTsFind(rowId);
        if (sysTsTmp != nullptr) {
            metadata->schema->dictSysTsDrop(sysTsTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.TS$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysTsTmp = new SysTs(rowId, 0, "", 0);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "TS#") {
                    updateNumber32u(sysTsTmp->ts, 0, column, table, offset);
                } else if (table->columns[column]->name == "NAME") {
                    updateString(sysTsTmp->name, SYS_TS_NAME_LENGTH, column, table, offset);
                } else if (table->columns[column]->name == "BLOCKSIZE") {
                    updateNumber32u(sysTsTmp->blockSize, 0, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysTsAdd(sysTsTmp);
        sysTsTmp = nullptr;
    }

    void SystemTransaction::processUpdateSysUser(OracleTable* table, typeRowId& rowId, uint64_t offset) {
        sysUserTmp = metadata->schema->dictSysUserFind(rowId);
        if (sysUserTmp != nullptr) {
            metadata->schema->dictSysUserDrop(sysUserTmp);
        } else {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.USER$: (rowid: " + rowId.toString() + ") for update");
                return;
            }
            sysUserTmp = new SysUser(rowId, 0, "", 0, 0, false);
        }

        uint64_t baseMax = builder->valuesMax >> 6;
        for (uint64_t base = 0; base <= baseMax; ++base) {
            auto column = static_cast<typeCol>(base << 6);
            for (uint64_t mask = 1; mask != 0; mask <<= 1, ++column) {
                if (builder->valuesSet[base] < mask)
                    break;
                if ((builder->valuesSet[base] & mask) == 0)
                    continue;

                if (table->columns[column]->name == "USER#") {
                    updateNumber32u(sysUserTmp->user, 0, column, table, offset);
                } else if (table->columns[column]->name == "NAME") {
                    updateString(sysUserTmp->name, SYS_USER_NAME_LENGTH, column, table, offset);
                } else if (table->columns[column]->name == "SPARE1") {
                    updateNumberXu(sysUserTmp->spare1, column, table, offset);
                }
            }
        }

        metadata->schema->dictSysUserAdd(sysUserTmp);
        sysUserTmp = nullptr;
    }

    void SystemTransaction::processUpdate(OracleTable* table, typeDataObj dataObj, typeDba bdba, typeSlot slot, uint64_t offset) {
        typeRowId rowId(dataObj, bdba, slot);
        char str[19];
        rowId.toString(str);
        if (ctx->trace & TRACE_SYSTEM)
            ctx->logTrace(TRACE_SYSTEM, "update table (name: " + table->owner + "." + table->name + ", rowid: " + rowId.toString() + ")");

        switch (table->systemTable) {
            case TABLE_SYS_CCOL:
                processUpdateSysCCol(table, rowId, offset);
                break;

            case TABLE_SYS_CDEF:
                processUpdateSysCDef(table, rowId, offset);
                break;

            case TABLE_SYS_COL:
                processUpdateSysCol(table, rowId, offset);
                break;

            case TABLE_SYS_DEFERRED_STG:
                processUpdateSysDeferredStg(table, rowId, offset);
                break;

            case TABLE_SYS_ECOL:
                processUpdateSysECol(table, rowId, offset);
                break;

            case TABLE_SYS_LOB:
                processUpdateSysLob(table, rowId, offset);
                break;

            case TABLE_SYS_LOB_COMP_PART:
                processUpdateSysLobCompPart(table, rowId, offset);
                break;

            case TABLE_SYS_LOB_FRAG:
                processUpdateSysLobFrag(table, rowId, offset);
                break;

            case TABLE_SYS_OBJ:
                processUpdateSysObj(table, rowId, offset);
                break;

            case TABLE_SYS_TAB:
                processUpdateSysTab(table, rowId, offset);
                break;

            case TABLE_SYS_TABCOMPART:
                processUpdateSysTabComPart(table, rowId, offset);
                break;

            case TABLE_SYS_TABPART:
                processUpdateSysTabPart(table, rowId, offset);
                break;

            case TABLE_SYS_TABSUBPART:
                processUpdateSysTabSubPart(table, rowId, offset);
                break;

            case TABLE_SYS_TS:
                processUpdateSysTs(table, rowId, offset);
                break;

            case TABLE_SYS_USER:
                processUpdateSysUser(table, rowId, offset);
                break;
        }
    }

    void SystemTransaction::processDeleteSysCCol(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysCColTmp = metadata->schema->dictSysCColFind(rowId);
        if (sysCColTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.CCOL$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysCColDrop(sysCColTmp);
        metadata->schema->sysCColSetTouched.erase(sysCColTmp);
        delete sysCColTmp;
        sysCColTmp = nullptr;
    }

    void SystemTransaction::processDeleteSysCDef(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysCDefTmp = metadata->schema->dictSysCDefFind(rowId);
        if (sysCDefTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.CDEF$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysCDefDrop(sysCDefTmp);
        metadata->schema->sysCDefSetTouched.erase(sysCDefTmp);
        delete sysCDefTmp;
        sysCDefTmp = nullptr;
    }

    void SystemTransaction::processDeleteSysCol(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysColTmp = metadata->schema->dictSysColFind(rowId);
        if (sysColTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.COL$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysColDrop(sysColTmp);
        metadata->schema->sysColSetTouched.erase(sysColTmp);
        delete sysColTmp;
        sysColTmp = nullptr;
    }

    void SystemTransaction::processDeleteSysDeferredStg(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysDeferredStgTmp = metadata->schema->dictSysDeferredStgFind(rowId);
        if (sysDeferredStgTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.DEFERRED_STG$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysDeferredStgDrop(sysDeferredStgTmp);
        metadata->schema->sysDeferredStgSetTouched.erase(sysDeferredStgTmp);
        delete sysDeferredStgTmp;
        sysDeferredStgTmp = nullptr;
    }

    void SystemTransaction::processDeleteSysECol(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysEColTmp = metadata->schema->dictSysEColFind(rowId);
        if (sysEColTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.ECOL$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysEColDrop(sysEColTmp);
        metadata->schema->sysEColSetTouched.erase(sysEColTmp);
        delete sysEColTmp;
        sysEColTmp = nullptr;
    }

    void SystemTransaction::processDeleteSysLob(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysLobTmp = metadata->schema->dictSysLobFind(rowId);
        if (sysLobTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.LOB$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysLobDrop(sysLobTmp);
        metadata->schema->sysLobSetTouched.erase(sysLobTmp);
        delete sysLobTmp;
        sysLobTmp = nullptr;
    }

    void SystemTransaction::processDeleteSysLobCompPart(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysLobCompPartTmp = metadata->schema->dictSysLobCompPartFind(rowId);
        if (sysLobCompPartTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.LOBCOMPPART$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysLobCompPartDrop(sysLobCompPartTmp);
        metadata->schema->sysLobCompPartSetTouched.erase(sysLobCompPartTmp);
        delete sysLobCompPartTmp;
        sysLobCompPartTmp = nullptr;
    }

    void SystemTransaction::processDeleteSysLobFrag(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysLobFragTmp = metadata->schema->dictSysLobFragFind(rowId);
        if (sysLobFragTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.LOBFRAG$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysLobFragDrop(sysLobFragTmp);
        metadata->schema->sysLobFragSetTouched.erase(sysLobFragTmp);
        delete sysLobFragTmp;
        sysLobFragTmp = nullptr;
    }

    void SystemTransaction::processDeleteSysObj(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysObjTmp = metadata->schema->dictSysObjFind(rowId);
        if (sysObjTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.OBJ$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysObjDrop(sysObjTmp);
        metadata->schema->sysObjSetTouched.erase(sysObjTmp);
        delete sysObjTmp;
        sysObjTmp = nullptr;
    }

    void SystemTransaction::processDeleteSysTab(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysTabTmp = metadata->schema->dictSysTabFind(rowId);
        if (sysTabTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.TAB$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysTabDrop(sysTabTmp);
        metadata->schema->sysTabSetTouched.erase(sysTabTmp);
        delete sysTabTmp;
        sysTabTmp = nullptr;
    }

    void SystemTransaction::processDeleteSysTabComPart(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysTabComPartTmp = metadata->schema->dictSysTabComPartFind(rowId);
        if (sysTabComPartTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.TABCOMPART$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysTabComPartDrop(sysTabComPartTmp);
        metadata->schema->sysTabComPartSetTouched.erase(sysTabComPartTmp);
        delete sysTabComPartTmp;
        sysTabComPartTmp = nullptr;
    }

    void SystemTransaction::processDeleteSysTabPart(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysTabPartTmp = metadata->schema->dictSysTabPartFind(rowId);
        if (sysTabPartTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.TABPART$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysTabPartDrop(sysTabPartTmp);
        metadata->schema->sysTabPartSetTouched.erase(sysTabPartTmp);
        delete sysTabPartTmp;
        sysTabPartTmp = nullptr;
    }

    void SystemTransaction::processDeleteSysTabSubPart(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysTabSubPartTmp = metadata->schema->dictSysTabSubPartFind(rowId);
        if (sysTabSubPartTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.TABSUBPART$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysTabSubPartDrop(sysTabSubPartTmp);
        metadata->schema->sysTabSubPartSetTouched.erase(sysTabSubPartTmp);
        delete sysTabSubPartTmp;
        sysTabSubPartTmp = nullptr;
    }

    void SystemTransaction::processDeleteSysTs(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysTsTmp = metadata->schema->dictSysTsFind(rowId);
        if (sysTsTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.TS$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysTsDrop(sysTsTmp);
        delete sysTsTmp;
        sysTsTmp = nullptr;
    }

    void SystemTransaction::processDeleteSysUser(typeRowId& rowId, uint64_t offset __attribute__((unused))) {
        sysUserTmp = metadata->schema->dictSysUserFind(rowId);
        if (sysUserTmp == nullptr) {
            if (!FLAG(REDO_FLAGS_ADAPTIVE_SCHEMA)) {
                if (ctx->trace & TRACE_SYSTEM)
                    ctx->logTrace(TRACE_SYSTEM, "missing SYS.USER$: (rowid: " + rowId.toString() + ") for delete");
                return;
            }
        }

        metadata->schema->dictSysUserDrop(sysUserTmp);
        metadata->schema->sysUserSetTouched.erase(sysUserTmp);
        delete sysUserTmp;
        sysUserTmp = nullptr;
    }

    void SystemTransaction::processDelete(OracleTable* table, typeDataObj dataObj, typeDba bdba, typeSlot slot, uint64_t offset) {
            typeRowId rowId(dataObj, bdba, slot);
        char str[19];
        rowId.toString(str);
        if (ctx->trace & TRACE_SYSTEM)
            ctx->logTrace(TRACE_SYSTEM, "delete table (name: " + table->owner + "." + table->name + ", rowid: " + rowId.toString() + ")");

        switch (table->systemTable) {
            case TABLE_SYS_CCOL:
                processDeleteSysCCol(rowId, offset);
                break;

            case TABLE_SYS_CDEF:
                processDeleteSysCDef(rowId, offset);
                break;

            case TABLE_SYS_COL:
                processDeleteSysCol(rowId, offset);
                break;

            case TABLE_SYS_DEFERRED_STG:
                processDeleteSysDeferredStg(rowId, offset);
                break;

            case TABLE_SYS_ECOL:
                processDeleteSysECol(rowId, offset);
                break;

            case TABLE_SYS_LOB:
                processDeleteSysLob(rowId, offset);
                break;

            case TABLE_SYS_LOB_COMP_PART:
                processDeleteSysLobCompPart(rowId, offset);
                break;

            case TABLE_SYS_LOB_FRAG:
                processDeleteSysLobFrag(rowId, offset);
                break;

            case TABLE_SYS_OBJ:
                processDeleteSysObj(rowId, offset);
                break;

            case TABLE_SYS_TAB:
                processDeleteSysTab(rowId, offset);
                break;

            case TABLE_SYS_TABCOMPART:
                processDeleteSysTabComPart(rowId, offset);
                break;

            case TABLE_SYS_TABPART:
                processDeleteSysTabPart(rowId, offset);
                break;

            case TABLE_SYS_TABSUBPART:
                processDeleteSysTabSubPart(rowId, offset);
                break;

            case TABLE_SYS_TS:
                processDeleteSysTs(rowId, offset);
                break;

            case TABLE_SYS_USER:
                processDeleteSysUser(rowId, offset);
                break;
        }
    }

    void SystemTransaction::commit(typeScn scn) {
        if (ctx->trace & TRACE_SYSTEM)
            ctx->logTrace(TRACE_SYSTEM, "commit");

        if (!metadata->schema->touched)
            return;

        std::list<std::string> msgsDropped;
        std::list<std::string> msgsUpdated;
        metadata->schema->scn = scn;
        metadata->schema->dropUnusedMetadata(metadata->users, msgsDropped);

        for (SchemaElement* element: metadata->schemaElements)
            metadata->schema->buildMaps(element->owner, element->table, element->keys, element->keysStr, element->options,
                                        msgsUpdated, metadata->suppLogDbPrimary, metadata->suppLogDbAll,
                                        metadata->defaultCharacterMapId, metadata->defaultCharacterNcharMapId);
        metadata->schema->resetTouched();

        for (const auto& msg: msgsDropped) {
            ctx->info(0, "dropped metadata: " + msg);
        }
        for (const auto& msg: msgsUpdated) {
            ctx->info(0, "updated metadata: " + msg);
        }
    }
}
