/* Memory buffer for handling output buffer in JSON format
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
#include "../common/SysCol.h"
#include "../common/typeRowId.h"
#include "../metadata/Metadata.h"
#include "../metadata/Schema.h"
#include "BuilderProtobuf.h"

namespace OpenLogReplicator {
    BuilderProtobuf::BuilderProtobuf(Ctx* newCtx, Locales* newLocales, Metadata* newMetadata, uint64_t newDbFormat, uint64_t newAttributesFormat,
                                     uint64_t newIntervalDtsFormat, uint64_t newIntervalYtmFormat, uint64_t newMessageFormat, uint64_t newRidFormat,
                                     uint64_t newXidFormat, uint64_t newTimestampFormat, uint64_t newTimestampTzFormat, uint64_t newTimestampAll,
                                     uint64_t newCharFormat, uint64_t newScnFormat, uint64_t newScnAll, uint64_t newUnknownFormat, uint64_t newSchemaFormat,
                                     uint64_t newColumnFormat, uint64_t newUnknownType, uint64_t newFlushBuffer) :
            Builder(newCtx, newLocales, newMetadata, newDbFormat, newAttributesFormat, newIntervalDtsFormat, newIntervalYtmFormat, newMessageFormat,
                    newRidFormat, newXidFormat, newTimestampFormat, newTimestampTzFormat, newTimestampAll, newCharFormat, newScnFormat, newScnAll,
                    newUnknownFormat, newSchemaFormat, newColumnFormat, newUnknownType, newFlushBuffer),
            redoResponsePB(nullptr),
            valuePB(nullptr),
            payloadPB(nullptr),
            schemaPB(nullptr) {
    }

    BuilderProtobuf::~BuilderProtobuf() {
        if (redoResponsePB != nullptr) {
            delete redoResponsePB;
            redoResponsePB = nullptr;
        }
        google::protobuf::ShutdownProtobufLibrary();
    }

    void BuilderProtobuf::columnNull(OracleTable* table, typeCol col, bool after) {
        if (table != nullptr && unknownType == UNKNOWN_TYPE_HIDE) {
            OracleColumn* column = table->columns[col];
            if (column->storedAsLob)
                return;
            if (column->guard && !FLAG(REDO_FLAGS_SHOW_GUARD_COLUMNS))
                return;
            if (column->nested && !FLAG(REDO_FLAGS_SHOW_NESTED_COLUMNS))
                return;
            if (column->hidden && !FLAG(REDO_FLAGS_SHOW_HIDDEN_COLUMNS))
                return;
            if (column->unused && !FLAG(REDO_FLAGS_SHOW_UNUSED_COLUMNS))
                return;

            uint64_t typeNo = table->columns[col]->type;
            if (typeNo != SYS_COL_TYPE_VARCHAR
                    && typeNo != SYS_COL_TYPE_NUMBER
                    && typeNo != SYS_COL_TYPE_DATE
                    && typeNo != SYS_COL_TYPE_RAW
                    && typeNo != SYS_COL_TYPE_CHAR
                    && (typeNo != SYS_COL_TYPE_XMLTYPE || !after)
                    && typeNo != SYS_COL_TYPE_FLOAT
                    && typeNo != SYS_COL_TYPE_DOUBLE
                    && (typeNo != SYS_COL_TYPE_CLOB || !after)
                    && (typeNo != SYS_COL_TYPE_BLOB || !after)
                    && typeNo != SYS_COL_TYPE_TIMESTAMP
                    && typeNo != SYS_COL_TYPE_INTERVAL_YEAR_TO_MONTH
                    && typeNo != SYS_COL_TYPE_INTERVAL_DAY_TO_SECOND
                    && typeNo != SYS_COL_TYPE_UROWID
                    && typeNo != SYS_COL_TYPE_TIMESTAMP_WITH_LOCAL_TZ)
                return;
        }

        if (table == nullptr || FLAG(REDO_FLAGS_RAW_COLUMN_DATA)) {
            std::string columnName("COL_" + std::to_string(col));
            valuePB->set_name(columnName);
            return;
        }

        valuePB->set_name(table->columns[col]->name);
    }

    void BuilderProtobuf::columnFloat(const std::string& columnName, double value) {
        valuePB->set_name(columnName);
        valuePB->set_value_double(value);
    }

    // TODO: possible precision loss
    void BuilderProtobuf::columnDouble(const std::string& columnName, long double value) {
        valuePB->set_name(columnName);
        valuePB->set_value_double(value);
    }

    void BuilderProtobuf::columnString(const std::string& columnName) {
        valuePB->set_name(columnName);
        valuePB->set_value_string(valueBuffer, valueLength);
    }

    void BuilderProtobuf::columnNumber(const std::string& columnName, uint64_t precision, uint64_t scale) {
        valuePB->set_name(columnName);
        valueBuffer[valueLength] = 0;
        char* retPtr;

        if (scale == 0 && precision <= 17) {
            int64_t value = strtol(valueBuffer, &retPtr, 10);
            valuePB->set_value_int(value);
        } else if (precision <= 6 && scale < 38) {
            float value = strtof(valueBuffer, &retPtr);
            valuePB->set_value_float(value);
        } else if (precision <= 15 && scale <= 307) {
            double value = strtod(valueBuffer, &retPtr);
            valuePB->set_value_double(value);
        } else {
            valuePB->set_value_string(valueBuffer, valueLength);
        }
    }

    void BuilderProtobuf::columnRowId(const std::string& columnName, typeRowId rowId) {
        char str[19];
        rowId.toHex(str);
        valuePB->set_name(columnName);
        valuePB->set_value_string(str, 18);
    }

    void BuilderProtobuf::columnRaw(const std::string& columnName, const uint8_t* data __attribute__((unused)), uint64_t length __attribute__((unused))) {
        valuePB->set_name(columnName);
        // TODO: implement
    }

    void BuilderProtobuf::columnTimestamp(const std::string& columnName, struct tm& epochTime __attribute__((unused)), uint64_t fraction __attribute__((unused))) {
        valuePB->set_name(columnName);
        // TODO: implement
    }

    void BuilderProtobuf::columnTimestampTz(const std::string& columnName, struct tm& epochTime __attribute__((unused)), uint64_t fraction __attribute__((unused)),
                                          const char* tz __attribute__((unused))) {
        valuePB->set_name(columnName);
        // TODO: implement
    }

    void BuilderProtobuf::appendRowid(typeDataObj dataObj, typeDba bdba, typeSlot slot) {
        if ((messageFormat & MESSAGE_FORMAT_ADD_SEQUENCES) != 0)
            payloadPB->set_num(num);

        if (ridFormat == RID_FORMAT_SKIP)
            return;
        else if (ridFormat == RID_FORMAT_TEXT) {
            typeRowId rowId(dataObj, bdba, slot);
            char str[19];
            rowId.toString(str);
            payloadPB->set_rid(str, 18);
        }
    }

    void BuilderProtobuf::appendHeader(typeScn scn, typeTime time_, bool first, bool showDb, bool showXid) {
        redoResponsePB->set_code(pb::ResponseCode::PAYLOAD);
        if (first || (scnAll & SCN_ALL_PAYLOADS) != 0) {
            if ((scnFormat & SCN_FORMAT_TEXT_HEX) != 0) {
                char buf[17];
                numToString(scn, buf, 16);
                redoResponsePB->set_scns(buf);
            } else {
                redoResponsePB->set_scn(scn);
            }
        }

        if (first || (timestampAll & TIMESTAMP_ALL_PAYLOADS) != 0) {
            std::string str;
            switch (timestampFormat) {
                case TIMESTAMP_FORMAT_UNIX_NANO:
                    redoResponsePB->set_tm(time_.toTime() * 1000000000L);
                    break;

                case TIMESTAMP_FORMAT_UNIX_MICRO:
                    redoResponsePB->set_tm(time_.toTime() * 1000000L);
                    break;

                case TIMESTAMP_FORMAT_UNIX_MILLI:
                    redoResponsePB->set_tm(time_.toTime() * 1000L);
                    break;

                case TIMESTAMP_FORMAT_UNIX:
                    redoResponsePB->set_tm(time_.toTime());
                    break;

                case TIMESTAMP_FORMAT_UNIX_NANO_STRING:
                    str = std::to_string(time_.toTime() * 1000000000L);
                    redoResponsePB->set_tms(str);
                    break;

                case TIMESTAMP_FORMAT_UNIX_MICRO_STRING:
                    str = std::to_string(time_.toTime() * 1000000L);
                    redoResponsePB->set_tms(str);
                    break;

                case TIMESTAMP_FORMAT_UNIX_MILLI_STRING:
                    str = std::to_string(time_.toTime() * 1000L);
                    redoResponsePB->set_tms(str);
                    break;

                case TIMESTAMP_FORMAT_UNIX_STRING:
                    str = std::to_string(time_.toTime());
                    redoResponsePB->set_tms(str);
                    break;

                case TIMESTAMP_FORMAT_ISO8601:
                    char iso[21];
                    time_.toIso8601(iso);
                    redoResponsePB->set_tms(iso);
                    break;
            }
        }

        redoResponsePB->set_c_scn(lwnScn);
        redoResponsePB->set_c_idx(lwnIdx);

        if (showXid) {
            if (xidFormat == XID_FORMAT_TEXT_HEX) {
                std::ostringstream ss;
                ss << "0x";
                ss << std::setfill('0') << std::setw(4) << std::hex << static_cast<uint64_t>(lastXid.usn());
                ss << '.';
                ss << std::setfill('0') << std::setw(3) << std::hex << static_cast<uint64_t>(lastXid.slt());
                ss << '.';
                ss << std::setfill('0') << std::setw(8) << std::hex << static_cast<uint64_t>(lastXid.sqn());
                redoResponsePB->set_xid(ss.str());
            } else if (xidFormat == XID_FORMAT_TEXT_DEC) {
                std::ostringstream ss;
                ss << static_cast<uint64_t>(lastXid.usn());
                ss << '.';
                ss << static_cast<uint64_t>(lastXid.slt());
                ss << '.';
                ss << static_cast<uint64_t>(lastXid.sqn());
                redoResponsePB->set_xid(ss.str());
            } else if (xidFormat == XID_FORMAT_NUMERIC) {
                redoResponsePB->set_xidn(lastXid.getData());
            }
        }

        if (showDb)
            redoResponsePB->set_db(metadata->conName);
    }

    void BuilderProtobuf::appendSchema(OracleTable* table, typeObj obj) {
        if (table == nullptr) {
            std::string ownerName;
            std::string tableName;
            // try to read object name from ongoing uncommitted transaction data
            if (metadata->schema->checkTableDictUncommitted(obj, ownerName, tableName)) {
                schemaPB->set_owner(ownerName);
                schemaPB->set_name(tableName);
            } else {
                tableName = "OBJ_" + std::to_string(obj);
                schemaPB->set_name(tableName);
            }

            if ((schemaFormat & SCHEMA_FORMAT_OBJ) != 0)
                schemaPB->set_obj(obj);

            return;
        }

        schemaPB->set_owner(table->owner);
        schemaPB->set_name(table->name);

        if ((schemaFormat & SCHEMA_FORMAT_OBJ) != 0)
            schemaPB->set_obj(obj);

        if ((schemaFormat & SCHEMA_FORMAT_FULL) != 0) {
            if ((schemaFormat & SCHEMA_FORMAT_REPEATED) == 0) {
                if (tables.count(table) > 0)
                    return;
                else
                    tables.insert(table);
            }

            schemaPB->add_column();
            pb::Column* columnPB = schemaPB->mutable_column(schemaPB->column_size() - 1);

            for (typeCol column = 0; column < static_cast<typeCol>(table->columns.size()); ++column) {
                if (table->columns[column] == nullptr)
                    continue;

                columnPB->set_name(table->columns[column]->name);

                switch (table->columns[column]->type) {
                    case SYS_COL_TYPE_VARCHAR:
                        columnPB->set_type(pb::VARCHAR2);
                        columnPB->set_length(static_cast<int32_t>(table->columns[column]->length));
                        break;

                    case SYS_COL_TYPE_NUMBER:
                        columnPB->set_type(pb::NUMBER);
                        columnPB->set_precision(static_cast<int32_t>(table->columns[column]->precision));
                        columnPB->set_scale(static_cast<int32_t>(table->columns[column]->scale));
                        break;

                    // Long, not supported
                    case SYS_COL_TYPE_LONG:
                        columnPB->set_type(pb::LONG);
                        break;

                    case SYS_COL_TYPE_DATE:
                        columnPB->set_type(pb::DATE);
                        break;

                    case SYS_COL_TYPE_RAW:
                        columnPB->set_type(pb::RAW);
                        columnPB->set_length(static_cast<int32_t>(table->columns[column]->length));
                        break;

                    case SYS_COL_TYPE_LONG_RAW: // Not supported
                        columnPB->set_type(pb::LONG_RAW);
                        break;

                    case SYS_COL_TYPE_CHAR:
                        columnPB->set_type(pb::CHAR);
                        columnPB->set_length(static_cast<int32_t>(table->columns[column]->length));
                        break;

                    case SYS_COL_TYPE_FLOAT:
                        columnPB->set_type(pb::BINARY_FLOAT);
                        break;

                    case SYS_COL_TYPE_DOUBLE:
                        columnPB->set_type(pb::BINARY_DOUBLE);
                        break;

                    case SYS_COL_TYPE_CLOB:
                        columnPB->set_type(pb::CLOB);
                        break;

                    case SYS_COL_TYPE_BLOB:
                        columnPB->set_type(pb::BLOB);
                        break;

                    case SYS_COL_TYPE_TIMESTAMP:
                        columnPB->set_type(pb::TIMESTAMP);
                        columnPB->set_length(static_cast<int32_t>(table->columns[column]->length));
                        break;

                    case SYS_COL_TYPE_TIMESTAMP_WITH_TZ:
                        columnPB->set_type(pb::TIMESTAMP_WITH_TZ);
                        columnPB->set_length(static_cast<int32_t>(table->columns[column]->length));
                        break;

                    case SYS_COL_TYPE_INTERVAL_YEAR_TO_MONTH:
                        columnPB->set_type(pb::INTERVAL_YEAR_TO_MONTH);
                        columnPB->set_length(static_cast<int32_t>(table->columns[column]->length));
                        break;

                    case SYS_COL_TYPE_INTERVAL_DAY_TO_SECOND:
                        columnPB->set_type(pb::INTERVAL_DAY_TO_SECOND);
                        columnPB->set_length(static_cast<int32_t>(table->columns[column]->length));
                        break;

                    case SYS_COL_TYPE_UROWID:
                        columnPB->set_type(pb::UROWID);
                        columnPB->set_length(static_cast<int32_t>(table->columns[column]->length));
                        break;

                    case SYS_COL_TYPE_TIMESTAMP_WITH_LOCAL_TZ:
                        columnPB->set_type(pb::TIMESTAMP_WITH_LOCAL_TZ);
                        columnPB->set_length(static_cast<int32_t>(table->columns[column]->length));
                        break;

                    default:
                        columnPB->set_type(pb::UNKNOWN);
                        break;
                }

                columnPB->set_nullable(table->columns[column]->nullable);
            }
        }
    }

    void BuilderProtobuf::numToString(uint64_t value, char* buf, uint64_t length) {
        uint64_t j = (length - 1) * 4;
        for (uint64_t i = 0; i < length; ++i) {
            buf[i] = ctx->map16[(value >> j) & 0xF];
            j -= 4;
        }
        buf[length] = 0;
    }

    void BuilderProtobuf::processBeginMessage(typeScn scn, typeSeq sequence, typeTime time_) {
        newTran = false;
        builderBegin(scn, sequence, 0, 0);
        createResponse();
        appendHeader(scn, time_, true, (dbFormat & DB_FORMAT_ADD_DML) != 0, true);

        if ((messageFormat & MESSAGE_FORMAT_FULL) == 0) {
            redoResponsePB->add_payload();
            payloadPB = redoResponsePB->mutable_payload(redoResponsePB->payload_size() - 1);
            payloadPB->set_op(pb::BEGIN);

            std::string output;
            bool ret = redoResponsePB->SerializeToString(&output);
            delete redoResponsePB;
            redoResponsePB = nullptr;

            if (!ret)
                throw RuntimeException(50017, "PB begin processing failed, error serializing to string");
            append(output);
            builderCommit(false);
        }
    }

    void BuilderProtobuf::processInsert(typeScn scn, typeSeq sequence, typeTime time_, LobCtx* lobCtx, OracleTable* table, typeObj obj, typeDataObj dataObj,
                                        typeDba bdba, typeSlot slot, typeXid xid __attribute__((unused)), uint64_t offset) {
        if (newTran)
            processBeginMessage(scn, sequence, time_);

        if ((messageFormat & MESSAGE_FORMAT_FULL) != 0) {
            if (redoResponsePB == nullptr)
                throw RuntimeException(50018, "PB insert processing failed, a message is missing");
        } else {
            builderBegin(scn, sequence, obj, 0);
            createResponse();
            appendHeader(scn, time_, true, (dbFormat & DB_FORMAT_ADD_DML) != 0, true);
        }

        redoResponsePB->add_payload();
        payloadPB = redoResponsePB->mutable_payload(redoResponsePB->payload_size() - 1);
        payloadPB->set_op(pb::INSERT);

        schemaPB = payloadPB->mutable_schema();
        appendSchema(table, obj);
        appendRowid(dataObj, bdba, slot);
        appendAfter(lobCtx, table, offset);

        if ((messageFormat & MESSAGE_FORMAT_FULL) == 0) {
            std::string output;
            bool ret = redoResponsePB->SerializeToString(&output);
            delete redoResponsePB;
            redoResponsePB = nullptr;

            if (!ret)
                throw RuntimeException(50017, "PB insert processing failed, error serializing to string");
            append(output);
            builderCommit(false);
        }
        ++num;
    }

    void BuilderProtobuf::processUpdate(typeScn scn, typeSeq sequence, typeTime time_, LobCtx* lobCtx, OracleTable* table, typeObj obj, typeDataObj dataObj,
                                        typeDba bdba, typeSlot slot, typeXid xid __attribute__((unused)), uint64_t offset) {
        if (newTran)
            processBeginMessage(scn, sequence, time_);

        if ((messageFormat & MESSAGE_FORMAT_FULL) != 0) {
            if (redoResponsePB == nullptr)
                throw RuntimeException(50018, "PB update processing failed, a message is missing");
        } else {
            builderBegin(scn, sequence, obj, 0);
            createResponse();
            appendHeader(scn, time_, true, (dbFormat & DB_FORMAT_ADD_DML) != 0, true);
        }

        redoResponsePB->add_payload();
        payloadPB = redoResponsePB->mutable_payload(redoResponsePB->payload_size() - 1);
        payloadPB->set_op(pb::UPDATE);

        schemaPB = payloadPB->mutable_schema();
        appendSchema(table, obj);
        appendRowid(dataObj, bdba, slot);
        appendBefore(lobCtx, table, offset);
        appendAfter(lobCtx, table, offset);

        if ((messageFormat & MESSAGE_FORMAT_FULL) == 0) {
            std::string output;
            bool ret = redoResponsePB->SerializeToString(&output);
            delete redoResponsePB;
            redoResponsePB = nullptr;

            if (!ret)
                throw RuntimeException(50017, "PB update processing failed, error serializing to string");
            append(output);
            builderCommit(false);
        }
        ++num;
    }

    void BuilderProtobuf::processDelete(typeScn scn, typeSeq sequence, typeTime time_, LobCtx* lobCtx, OracleTable* table, typeObj obj, typeDataObj dataObj,
                                        typeDba bdba, typeSlot slot, typeXid xid __attribute__((unused)), uint64_t offset) {
        if (newTran)
            processBeginMessage(scn, sequence, time_);

        if ((messageFormat & MESSAGE_FORMAT_FULL) != 0) {
            if (redoResponsePB == nullptr)
                throw RuntimeException(50018, "PB delete processing failed, a message is missing");
        } else {
            builderBegin(scn, sequence, obj, 0);
            createResponse();
            appendHeader(scn, time_, true, (dbFormat & DB_FORMAT_ADD_DML) != 0, true);
        }

        redoResponsePB->add_payload();
        payloadPB = redoResponsePB->mutable_payload(redoResponsePB->payload_size() - 1);
        payloadPB->set_op(pb::DELETE);

        schemaPB = payloadPB->mutable_schema();
        appendSchema(table, obj);
        appendRowid(dataObj, bdba, slot);
        appendBefore(lobCtx, table, offset);

        if ((messageFormat & MESSAGE_FORMAT_FULL) == 0) {
            std::string output;
            bool ret = redoResponsePB->SerializeToString(&output);
            delete redoResponsePB;
            redoResponsePB = nullptr;

            if (!ret)
                throw RuntimeException(50017, "PB delete processing failed, error serializing to string");
            append(output);
            builderCommit(false);
        }
        ++num;
    }

    void BuilderProtobuf::processDdl(typeScn scn, typeSeq sequence, typeTime time_, OracleTable* table __attribute__((unused)), typeObj obj,
                                     typeDataObj dataObj __attribute__((unused)), uint16_t type __attribute__((unused)), uint16_t seq __attribute__((unused)),
                                     const char* operation __attribute__((unused)), const char* sql, uint64_t sqlLength) {
        if (newTran)
            processBeginMessage(scn, sequence, time_);

        if ((messageFormat & MESSAGE_FORMAT_FULL) != 0) {
            if (redoResponsePB == nullptr)
                throw RuntimeException(50018, "PB commit processing failed, a message is missing");
        } else {
            builderBegin(scn, sequence, obj, 0);
            createResponse();
            appendHeader(scn, time_, true, (dbFormat & DB_FORMAT_ADD_DDL) != 0, true);

            redoResponsePB->add_payload();
            payloadPB = redoResponsePB->mutable_payload(redoResponsePB->payload_size() - 1);
            payloadPB->set_op(pb::DDL);
            appendSchema(table, obj);
            payloadPB->set_ddl(sql, sqlLength);
        }

        if ((messageFormat & MESSAGE_FORMAT_FULL) == 0) {
            std::string output;
            bool ret = redoResponsePB->SerializeToString(&output);
            delete redoResponsePB;
            redoResponsePB = nullptr;

            if (!ret)
                throw RuntimeException(50017, "PB commit processing failed, error serializing to string");
            append(output);
            builderCommit(true);
        }
        ++num;
    }

    void BuilderProtobuf::initialize() {
        Builder::initialize();

        GOOGLE_PROTOBUF_VERIFY_VERSION;
    }

    void BuilderProtobuf::processCommit(typeScn scn, typeSeq sequence, typeTime time_) {
        // Skip empty transaction
        if (newTran) {
            newTran = false;
            return;
        }

        if ((messageFormat & MESSAGE_FORMAT_FULL) != 0) {
            if (redoResponsePB == nullptr)
                throw RuntimeException(50018, "PB commit processing failed, a message is missing");
        } else {
            builderBegin(scn, sequence, 0, 0);
            createResponse();
            appendHeader(scn, time_, true, (dbFormat & DB_FORMAT_ADD_DML) != 0, true);

            redoResponsePB->add_payload();
            payloadPB = redoResponsePB->mutable_payload(redoResponsePB->payload_size() - 1);
            payloadPB->set_op(pb::COMMIT);
        }

        std::string output;
        bool ret = redoResponsePB->SerializeToString(&output);
        delete redoResponsePB;
        redoResponsePB = nullptr;

        if (!ret)
            throw RuntimeException(50017, "PB commit processing failed, error serializing to string");
        append(output);
        builderCommit(true);

        num = 0;
    }

    void BuilderProtobuf::processCheckpoint(typeScn scn, typeSeq sequence, typeTime time_ __attribute__((unused)), uint64_t offset, bool redo) {
        if (lwnScn != scn) {
            lwnScn = scn;
            lwnIdx = 0;
        }

        builderBegin(scn, sequence, 0, OUTPUT_BUFFER_MESSAGE_CHECKPOINT);
        createResponse();
        appendHeader(scn, time_, true, false, false);

        redoResponsePB->add_payload();
        payloadPB = redoResponsePB->mutable_payload(redoResponsePB->payload_size() - 1);
        payloadPB->set_op(pb::CHKPT);
        payloadPB->set_seq(sequence);
        payloadPB->set_offset(offset);
        payloadPB->set_redo(redo);

        std::string output;
        bool ret = redoResponsePB->SerializeToString(&output);
        delete redoResponsePB;
        redoResponsePB = nullptr;

        if (!ret)
            throw RuntimeException(50017, "PB commit processing failed, error serializing to string");
        append(output);
        builderCommit(true);
    }
}
