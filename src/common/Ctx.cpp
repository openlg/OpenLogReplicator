/* Context of little/big-endian data
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

#define GLOBALS 1

#include <cstdlib>
#include <csignal>
#include <execinfo.h>
#include <iostream>
#include <set>
#include <string>
#include <unistd.h>

#include "Ctx.h"
#include "DataException.h"
#include "RuntimeException.h"
#include "Thread.h"
#include "typeIntX.h"

uint64_t OLR_LOCALES = OLR_LOCALES_TIMESTAMP;

namespace OpenLogReplicator {
    const char Ctx::map10[11] = "0123456789";

    const char Ctx::map16[17] = "0123456789abcdef";

    const char Ctx::map64[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    const char Ctx::map64R[256] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 63,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0,
            0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0,
            0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    typeIntX typeIntX::BASE10[TYPE_INTX_DIGITS][10];

    Ctx::Ctx() :
            bigEndian(false),
            memoryMinMb(0),
            memoryMaxMb(0),
            memoryChunks(nullptr),
            memoryChunksMin(0),
            memoryChunksAllocated(0),
            memoryChunksFree(0),
            memoryChunksMax(0),
            memoryChunksHWM(0),
            memoryChunksReusable(0),
            version12(false),
            version(0),
            dumpRedoLog(0),
            dumpRawData(0),
            readBufferMax(0),
            buffersFree(0),
            bufferSizeMax(0),
            buffersMaxUsed(0),
            suppLogSize(0),
            checkpointIntervalS(600),
            checkpointIntervalMb(500),
            checkpointKeep(100),
            schemaForceInterval(20),
            redoReadSleepUs(50000),
            redoVerifyDelayUs(0),
            archReadSleepUs(10000000),
            archReadTries(10),
            refreshIntervalUs(10000000),
            pollIntervalUs(100000),
            queueSize(65536),
            dumpPath("."),
            stopLogSwitches(0),
            stopCheckpoints(0),
            stopTransactions(0),
            transactionSizeMax(0),
            logLevel(3),
            trace(0),
            flags(0),
            disableChecks(0),
            hardShutdown(false),
            softShutdown(false),
            replicatorFinished(false),
            read16(read16Little),
            read32(read32Little),
            read56(read56Little),
            read64(read64Little),
            readScn(readScnLittle),
            readScnR(readScnRLittle),
            write16(write16Little),
            write32(write32Little),
            write56(write56Little),
            write64(write64Little),
            writeScn(writeScnLittle) {
        mainThread = pthread_self();
    }

    Ctx::~Ctx() {
        lobIdToXidMap.clear();

        while (memoryChunksAllocated > 0) {
            --memoryChunksAllocated;
            free(memoryChunks[memoryChunksAllocated]);
            memoryChunks[memoryChunksAllocated] = nullptr;
        }

        if (memoryChunks != nullptr) {
            delete[] memoryChunks;
            memoryChunks = nullptr;
        }
    }

    void Ctx::setBigEndian() {
        bigEndian = true;
        read16 = read16Big;
        read32 = read32Big;
        read56 = read56Big;
        read64 = read64Big;
        readScn = readScnBig;
        readScnR = readScnRBig;
        write16 = write16Big;
        write32 = write32Big;
        write56 = write56Big;
        write64 = write64Big;
        writeScn = writeScnBig;
    }

    bool Ctx::isBigEndian() const {
        return bigEndian;
    }

    uint16_t Ctx::read16Little(const uint8_t* buf) {
        return static_cast<uint16_t>(buf[0]) | (static_cast<uint16_t>(buf[1]) << 8);
    }

    uint16_t Ctx::read16Big(const uint8_t* buf) {
        return (static_cast<uint16_t>(buf[0]) << 8) | static_cast<uint16_t>(buf[1]);
    }

    uint32_t Ctx::read24Big(const uint8_t* buf) {
        return (static_cast<uint32_t>(buf[0]) << 16) |
               (static_cast<uint32_t>(buf[1]) << 8) | static_cast<uint32_t>(buf[2]);
    }

    uint32_t Ctx::read32Little(const uint8_t* buf) {
        return static_cast<uint32_t>(buf[0]) | (static_cast<uint32_t>(buf[1]) << 8) |
               (static_cast<uint32_t>(buf[2]) << 16) | (static_cast<uint32_t>(buf[3]) << 24);
    }

    uint32_t Ctx::read32Big(const uint8_t* buf) {
        return (static_cast<uint32_t>(buf[0]) << 24) | (static_cast<uint32_t>(buf[1]) << 16) |
               (static_cast<uint32_t>(buf[2]) << 8) | static_cast<uint32_t>(buf[3]);
    }

    uint64_t Ctx::read56Little(const uint8_t* buf) {
        return static_cast<uint64_t>(buf[0]) | (static_cast<uint64_t>(buf[1]) << 8) |
               (static_cast<uint64_t>(buf[2]) << 16) | (static_cast<uint64_t>(buf[3]) << 24) |
               (static_cast<uint64_t>(buf[4]) << 32) | (static_cast<uint64_t>(buf[5]) << 40) |
               (static_cast<uint64_t>(buf[6]) << 48);
    }

    uint64_t Ctx::read56Big(const uint8_t* buf) {
        return (static_cast<uint64_t>(buf[0]) << 24) | (static_cast<uint64_t>(buf[1]) << 16) |
                (static_cast<uint64_t>(buf[2]) << 8) | (static_cast<uint64_t>(buf[3])) |
                (static_cast<uint64_t>(buf[4]) << 40) | (static_cast<uint64_t>(buf[5]) << 32) |
                (static_cast<uint64_t>(buf[6]) << 48);
    }

    uint64_t Ctx::read64Little(const uint8_t* buf) {
        return static_cast<uint64_t>(buf[0]) | (static_cast<uint64_t>(buf[1]) << 8) |
               (static_cast<uint64_t>(buf[2]) << 16) | (static_cast<uint64_t>(buf[3]) << 24) |
               (static_cast<uint64_t>(buf[4]) << 32) | (static_cast<uint64_t>(buf[5]) << 40) |
               (static_cast<uint64_t>(buf[6]) << 48) | (static_cast<uint64_t>(buf[7]) << 56);
    }

    uint64_t Ctx::read64Big(const uint8_t* buf) {
        return (static_cast<uint64_t>(buf[0]) << 56) | (static_cast<uint64_t>(buf[1]) << 48) |
               (static_cast<uint64_t>(buf[2]) << 40) | (static_cast<uint64_t>(buf[3]) << 32) |
               (static_cast<uint64_t>(buf[4]) << 24) | (static_cast<uint64_t>(buf[5]) << 16) |
               (static_cast<uint64_t>(buf[6]) << 8) | static_cast<uint64_t>(buf[7]);
    }

    typeScn Ctx::readScnLittle(const uint8_t* buf) {
        if (buf[0] == 0xFF && buf[1] == 0xFF && buf[2] == 0xFF && buf[3] == 0xFF && buf[4] == 0xFF && buf[5] == 0xFF)
            return ZERO_SCN;
        if ((buf[5] & 0x80) == 0x80)
            return static_cast<uint64_t>(buf[0]) | (static_cast<uint64_t>(buf[1]) << 8) |
                   (static_cast<uint64_t>(buf[2]) << 16) | (static_cast<uint64_t>(buf[3]) << 24) |
                   (static_cast<uint64_t>(buf[6]) << 32) | (static_cast<uint64_t>(buf[7]) << 40) |
                   (static_cast<uint64_t>(buf[4]) << 48) | (static_cast<uint64_t>(buf[5] & 0x7F) << 56);
        else
            return static_cast<uint64_t>(buf[0]) | (static_cast<uint64_t>(buf[1]) << 8) |
                   (static_cast<uint64_t>(buf[2]) << 16) | (static_cast<uint64_t>(buf[3]) << 24) |
                   (static_cast<uint64_t>(buf[4]) << 32) | (static_cast<uint64_t>(buf[5]) << 40);
    }

    typeScn Ctx::readScnBig(const uint8_t* buf) {
        if (buf[0] == 0xFF && buf[1] == 0xFF && buf[2] == 0xFF && buf[3] == 0xFF && buf[4] == 0xFF && buf[5] == 0xFF)
            return ZERO_SCN;
        if ((buf[4] & 0x80) == 0x80)
            return static_cast<uint64_t>(buf[3]) | (static_cast<uint64_t>(buf[2]) << 8) |
                   (static_cast<uint64_t>(buf[1]) << 16) | (static_cast<uint64_t>(buf[0]) << 24) |
                   (static_cast<uint64_t>(buf[7]) << 32) | (static_cast<uint64_t>(buf[6]) << 40) |
                   (static_cast<uint64_t>(buf[5]) << 48) | (static_cast<uint64_t>(buf[4] & 0x7F) << 56);
        else
            return static_cast<uint64_t>(buf[3]) | (static_cast<uint64_t>(buf[2]) << 8) |
                   (static_cast<uint64_t>(buf[1]) << 16) | (static_cast<uint64_t>(buf[0]) << 24) |
                   (static_cast<uint64_t>(buf[5]) << 32) | (static_cast<uint64_t>(buf[4]) << 40);
    }

    typeScn Ctx::readScnRLittle(const uint8_t* buf) {
        if (buf[0] == 0xFF && buf[1] == 0xFF && buf[2] == 0xFF && buf[3] == 0xFF && buf[4] == 0xFF && buf[5] == 0xFF)
            return ZERO_SCN;
        if ((buf[1] & 0x80) == 0x80)
            return static_cast<uint64_t>(buf[2]) | (static_cast<uint64_t>(buf[3]) << 8) |
                   (static_cast<uint64_t>(buf[4]) << 16) | (static_cast<uint64_t>(buf[5]) << 24) |
                   // (static_cast<uint64_t>(buf[6]) << 32) | (static_cast<uint64_t>(buf[7]) << 40) |
                   (static_cast<uint64_t>(buf[0]) << 48) | (static_cast<uint64_t>(buf[1] & 0x7F) << 56);
        else
            return static_cast<uint64_t>(buf[2]) | (static_cast<uint64_t>(buf[3]) << 8) |
                   (static_cast<uint64_t>(buf[4]) << 16) | (static_cast<uint64_t>(buf[5]) << 24) |
                   (static_cast<uint64_t>(buf[0]) << 32) | (static_cast<uint64_t>(buf[1]) << 40);
    }

    typeScn Ctx::readScnRBig(const uint8_t* buf) {
        if (buf[0] == 0xFF && buf[1] == 0xFF && buf[2] == 0xFF && buf[3] == 0xFF && buf[4] == 0xFF && buf[5] == 0xFF)
            return ZERO_SCN;
        if ((buf[0] & 0x80) == 0x80)
            return static_cast<uint64_t>(buf[5]) | (static_cast<uint64_t>(buf[4]) << 8) |
                   (static_cast<uint64_t>(buf[3]) << 16) | (static_cast<uint64_t>(buf[2]) << 24) |
                   // (static_cast<uint64_t>(buf[7]) << 32) | (static_cast<uint64_t>(buf[6]) << 40) |
                   (static_cast<uint64_t>(buf[1]) << 48) | (static_cast<uint64_t>(buf[0] & 0x7F) << 56);
        else
            return static_cast<uint64_t>(buf[5]) | (static_cast<uint64_t>(buf[4]) << 8) |
                   (static_cast<uint64_t>(buf[3]) << 16) | (static_cast<uint64_t>(buf[2]) << 24) |
                   (static_cast<uint64_t>(buf[1]) << 32) | (static_cast<uint64_t>(buf[0]) << 40);
    }

    void Ctx::write16Little(uint8_t* buf, uint16_t val) {
        buf[0] = val & 0xFF;
        buf[1] = (val >> 8) & 0xFF;
    }

    void Ctx::write16Big(uint8_t* buf, uint16_t val) {
        buf[0] = (val >> 8) & 0xFF;
        buf[1] = val & 0xFF;
    }

    void Ctx::write32Little(uint8_t* buf, uint32_t val) {
        buf[0] = val & 0xFF;
        buf[1] = (val >> 8) & 0xFF;
        buf[2] = (val >> 16) & 0xFF;
        buf[3] = (val >> 24) & 0xFF;
    }

    void Ctx::write32Big(uint8_t* buf, uint32_t val) {
        buf[0] = (val >> 24) & 0xFF;
        buf[1] = (val >> 16) & 0xFF;
        buf[2] = (val >> 8) & 0xFF;
        buf[3] = val & 0xFF;
    }

    void Ctx::write56Little(uint8_t* buf, uint64_t val) {
        buf[0] = val & 0xFF;
        buf[1] = (val >> 8) & 0xFF;
        buf[2] = (val >> 16) & 0xFF;
        buf[3] = (val >> 24) & 0xFF;
        buf[4] = (val >> 32) & 0xFF;
        buf[5] = (val >> 40) & 0xFF;
        buf[6] = (val >> 48) & 0xFF;
    }

    void Ctx::write56Big(uint8_t* buf, uint64_t val) {
        buf[0] = (val >> 24) & 0xFF;
        buf[1] = (val >> 16) & 0xFF;
        buf[2] = (val >> 8) & 0xFF;
        buf[3] = val & 0xFF;
        buf[4] = (val >> 40) & 0xFF;
        buf[5] = (val >> 32) & 0xFF;
        buf[6] = (val >> 48) & 0xFF;
    }

    void Ctx::write64Little(uint8_t* buf, uint64_t val) {
        buf[0] = val & 0xFF;
        buf[1] = (val >> 8) & 0xFF;
        buf[2] = (val >> 16) & 0xFF;
        buf[3] = (val >> 24) & 0xFF;
        buf[4] = (val >> 32) & 0xFF;
        buf[5] = (val >> 40) & 0xFF;
        buf[6] = (val >> 48) & 0xFF;
        buf[7] = (val >> 56) & 0xFF;
    }

    void Ctx::write64Big(uint8_t* buf, uint64_t val) {
        buf[0] = (val >> 56) & 0xFF;
        buf[1] = (val >> 48) & 0xFF;
        buf[2] = (val >> 40) & 0xFF;
        buf[3] = (val >> 32) & 0xFF;
        buf[4] = (val >> 24) & 0xFF;
        buf[5] = (val >> 16) & 0xFF;
        buf[6] = (val >> 8) & 0xFF;
        buf[7] = val & 0xFF;
    }

    void Ctx::writeScnLittle(uint8_t* buf, typeScn val) {
        if (val < 0x800000000000) {
            buf[0] = val & 0xFF;
            buf[1] = (val >> 8) & 0xFF;
            buf[2] = (val >> 16) & 0xFF;
            buf[3] = (val >> 24) & 0xFF;
            buf[4] = (val >> 32) & 0xFF;
            buf[5] = (val >> 40) & 0xFF;
        } else {
            buf[0] = val & 0xFF;
            buf[1] = (val >> 8) & 0xFF;
            buf[2] = (val >> 16) & 0xFF;
            buf[3] = (val >> 24) & 0xFF;
            buf[4] = (val >> 48) & 0xFF;
            buf[5] = ((val >> 56) & 0x7F) | 0x80;
            buf[6] = (val >> 32) & 0xFF;
            buf[7] = (val >> 40) & 0xFF;
        }
    }

    void Ctx::writeScnBig(uint8_t* buf, typeScn val) {
        if (val < 0x800000000000) {
            buf[0] = (val >> 24) & 0xFF;
            buf[1] = (val >> 16) & 0xFF;
            buf[2] = (val >> 8) & 0xFF;
            buf[3] = val & 0xFF;
            buf[4] = (val >> 40) & 0xFF;
            buf[5] = (val >> 32) & 0xFF;
        } else {
            buf[0] = (val >> 24) & 0xFF;
            buf[1] = (val >> 16) & 0xFF;
            buf[2] = (val >> 8) & 0xFF;
            buf[3] = val & 0xFF;
            buf[4] = ((val >> 56) & 0x7F) | 0x80;
            buf[5] = (val >> 48) & 0xFF;
            buf[6] = (val >> 40) & 0xFF;
            buf[7] = (val >> 32) & 0xFF;
        }
    }

    const rapidjson::Value& Ctx::getJsonFieldA(const std::string& fileName, const rapidjson::Value& value, const char* field) {
        if (!value.HasMember(field))
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " not found");
        const rapidjson::Value& ret = value[field];
        if (!ret.IsArray())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " is not an array");
        return ret;
    }

    uint16_t Ctx::getJsonFieldU16(const std::string& fileName, const rapidjson::Value& value, const char* field) {
        if (!value.HasMember(field))
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " not found");
        const rapidjson::Value& ret = value[field];
        if (!ret.IsUint64())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " is not an unsigned 64-bit number");
        uint64_t val = ret.GetUint64();
        if (val > 0xFFFF)
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " is too big (" + std::to_string(val) + ")");
        return val;
    }

    int16_t Ctx::getJsonFieldI16(const std::string& fileName, const rapidjson::Value& value, const char* field) {
        if (!value.HasMember(field))
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " not found");
        const rapidjson::Value& ret = value[field];
        if (!ret.IsInt64())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " is not a signed 64-bit number");
        int64_t val = ret.GetInt64();
        if ((val > static_cast<int64_t>(0x7FFF)) || (val < -static_cast<int64_t>(0x8000)))
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " is too big (" + std::to_string(val) + ")");
        return (int16_t)val;
    }

    uint32_t Ctx::getJsonFieldU32(const std::string& fileName, const rapidjson::Value& value, const char* field) {
        if (!value.HasMember(field))
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " not found");
        const rapidjson::Value& ret = value[field];
        if (!ret.IsUint64())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " is not an unsigned 64-bit number");
        uint64_t val = ret.GetUint64();
        if (val > 0xFFFFFFFF)
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " is too big (" + std::to_string(val) + ")");
        return static_cast<uint32_t>(val);
    }

    int32_t Ctx::getJsonFieldI32(const std::string& fileName, const rapidjson::Value& value, const char* field) {
        if (!value.HasMember(field))
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " not found");
        const rapidjson::Value& ret = value[field];
        if (!ret.IsInt64())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " is not a signed 64-bit number");
        int64_t val = ret.GetInt64();
        if ((val > static_cast<int64_t>(0x7FFFFFFF)) || (val < -static_cast<int64_t>(0x80000000)))
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " is too big (" + std::to_string(val) + ")");
        return static_cast<int32_t>(val);
    }

    uint64_t Ctx::getJsonFieldU64(const std::string& fileName, const rapidjson::Value& value, const char* field) {
        if (!value.HasMember(field))
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " not found");
        const rapidjson::Value& ret = value[field];
        if (!ret.IsUint64())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " is not an unsigned 64-bit number");
        return ret.GetUint64();
    }

    int64_t Ctx::getJsonFieldI64(const std::string& fileName, const rapidjson::Value& value, const char* field) {
        if (!value.HasMember(field))
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " not found");
        const rapidjson::Value& ret = value[field];
        if (!ret.IsInt64())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " is not a signed 64-bit number");
        return ret.GetInt64();
    }

    const rapidjson::Value& Ctx::getJsonFieldO(const std::string& fileName, const rapidjson::Value& value, const char* field) {
        if (!value.HasMember(field))
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " not found");
        const rapidjson::Value& ret = value[field];
        if (!ret.IsObject())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " is not an object");
        return ret;
    }

    const char* Ctx::getJsonFieldS(const std::string& fileName, uint64_t maxLength, const rapidjson::Value& value, const char* field) {
        if (!value.HasMember(field))
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " not found");
        const rapidjson::Value& ret = value[field];
        if (!ret.IsString())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " is not a string");
        if (ret.GetStringLength() > maxLength)
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + " is too long (" +
                                std::to_string(ret.GetStringLength()) + ", max: " + std::to_string(maxLength) + ")");
        return ret.GetString();
    }

    const rapidjson::Value& Ctx::getJsonFieldA(const std::string& fileName, const rapidjson::Value& value, const char* field, uint64_t num) {
        const rapidjson::Value& ret = value[num];
        if (!ret.IsArray())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + "[" + std::to_string(num) +
                                "] is not an array");
        return ret;
    }

    uint16_t Ctx::getJsonFieldU16(const std::string& fileName, const rapidjson::Value& value, const char* field, uint64_t num) {
        const rapidjson::Value& ret = value[num];
        if (!ret.IsUint64())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + "[" + std::to_string(num) +
                                "] is not an unsigned 64-bit number");
        uint64_t val = ret.GetUint64();
        if (val > 0xFFFF)
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + "[" + std::to_string(num) +
                                "] is too big (" + std::to_string(val) + ")");
        return val;
    }

    int16_t Ctx::getJsonFieldI16(const std::string& fileName, const rapidjson::Value& value, const char* field, uint64_t num) {
        const rapidjson::Value& ret = value[num];
        if (!ret.IsInt64())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + "[" + std::to_string(num) +
                                "] is not a signed 64-bit number");
        int64_t val = ret.GetInt64();
        if ((val > static_cast<int64_t>(0x7FFF)) || (val < -static_cast<int64_t>(0x8000)))
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + "[" + std::to_string(num) +
                                "] is too big (" + std::to_string(val) + ")");
        return (int16_t)val;
    }

    uint32_t Ctx::getJsonFieldU32(const std::string& fileName, const rapidjson::Value& value, const char* field, uint64_t num) {
        const rapidjson::Value& ret = value[num];
        if (!ret.IsUint64())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + "[" + std::to_string(num) +
                                "] is not an unsigned 64-bit number");
        uint64_t val = ret.GetUint64();
        if (val > 0xFFFFFFFF)
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + "[" + std::to_string(num) +
                                "] is too big (" + std::to_string(val) + ")");
        return static_cast<uint32_t>(val);
    }

    int32_t Ctx::getJsonFieldI32(const std::string& fileName, const rapidjson::Value& value, const char* field, uint64_t num) {
        const rapidjson::Value& ret = value[num];
        if (!ret.IsInt64())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + "[" + std::to_string(num) +
                                "] is not a signed 64-bit number");
        int64_t val = ret.GetInt64();
        if ((val > static_cast<int64_t>(0x7FFFFFFF)) || (val < -static_cast<int64_t>(0x80000000)))
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + "[" + std::to_string(num) +
                                "] is too big (" + std::to_string(val) + ")");
        return static_cast<int32_t>(val);
    }

    uint64_t Ctx::getJsonFieldU64(const std::string& fileName, const rapidjson::Value& value, const char* field, uint64_t num) {
        const rapidjson::Value& ret = value[num];
        if (!ret.IsUint64())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + "[" + std::to_string(num) +
                                "] is not an unsigned 64-bit number");
        return ret.GetUint64();
    }

    int64_t Ctx::getJsonFieldI64(const std::string& fileName, const rapidjson::Value& value, const char* field, uint64_t num) {
        const rapidjson::Value& ret = value[num];
        if (!ret.IsInt64())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + "[" + std::to_string(num) +
                                "] is not a signed 64-bit number");
        return ret.GetInt64();
    }

    const rapidjson::Value& Ctx::getJsonFieldO(const std::string& fileName, const rapidjson::Value& value, const char* field, uint64_t num) {
        const rapidjson::Value& ret = value[num];
        if (!ret.IsObject())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + "[" + std::to_string(num) +
                                "] is not an object");
        return ret;
    }

    const char* Ctx::getJsonFieldS(const std::string& fileName, uint64_t maxLength, const rapidjson::Value& value, const char* field, uint64_t num) {
        const rapidjson::Value& ret = value[num];
        if (!ret.IsString())
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + "[" + std::to_string(num) +
                                "] is not a string");
        if (ret.GetStringLength() > maxLength)
            throw DataException(20003, "file: " + fileName + " - parse error, field " + field + "[" + std::to_string(num) +
                                "] is too long (" +
                    std::to_string(ret.GetStringLength()) + ", max: " + std::to_string(maxLength) + ")");
        return ret.GetString();
    }

    void Ctx::initialize(uint64_t newMemoryMinMb, uint64_t newMemoryMaxMb, uint64_t newReadBufferMax) {
        memoryMinMb = newMemoryMinMb;
        memoryMaxMb = newMemoryMaxMb;
        memoryChunksMin = (memoryMinMb / MEMORY_CHUNK_SIZE_MB);
        memoryChunksMax = memoryMaxMb / MEMORY_CHUNK_SIZE_MB;
        readBufferMax = newReadBufferMax;
        buffersFree = newReadBufferMax;
        bufferSizeMax = readBufferMax * MEMORY_CHUNK_SIZE;

        memoryChunks = new uint8_t*[memoryMaxMb / MEMORY_CHUNK_SIZE_MB];
        for (uint64_t i = 0; i < memoryChunksMin; ++i) {
            memoryChunks[i] = reinterpret_cast<uint8_t*>(aligned_alloc(MEMORY_ALIGNMENT, MEMORY_CHUNK_SIZE));
            if (memoryChunks[i] == nullptr)
                throw RuntimeException(10016, "couldn't allocate " + std::to_string(MEMORY_CHUNK_SIZE_MB) +
                                       " bytes memory for: memory chunks#2");
            ++memoryChunksAllocated;
            ++memoryChunksFree;
        }
        memoryChunksHWM = static_cast<uint64_t>(memoryChunksMin);
    }

    void Ctx::wakeAllOutOfMemory() {
        std::unique_lock<std::mutex> lck(memoryMtx);
        condOutOfMemory.notify_all();
    }

    uint64_t Ctx::getMaxUsedMemory() const {
        return memoryChunksHWM * MEMORY_CHUNK_SIZE_MB;
    }

    uint64_t Ctx::getFreeMemory() {
        return memoryChunksFree * MEMORY_CHUNK_SIZE_MB;
    }

    uint64_t Ctx::getAllocatedMemory() const {
        return memoryChunksAllocated * MEMORY_CHUNK_SIZE_MB;
    }

    uint8_t* Ctx::getMemoryChunk(const char* module, bool reusable) {
        std::unique_lock<std::mutex> lck(memoryMtx);

        if (memoryChunksFree == 0) {
            while (memoryChunksAllocated == memoryChunksMax && !softShutdown) {
                if (memoryChunksReusable > 1) {
                    warning(10067, "out of memory, but there are reusable memory chunks, trying to reuse some memory");

                    if (trace & TRACE_SLEEP)
                        logTrace(TRACE_SLEEP, "Ctx:getMemoryChunk");
                    condOutOfMemory.wait(lck);
                } else {
                    hint("try to restart with higher value of 'memory-max-mb' parameter or if big transaction - add to 'skip-xid' list; "
                         "transaction would be skipped");
                    throw RuntimeException(10017, "out of memory");
                }
            }

            if (memoryChunksFree == 0) {
                memoryChunks[0] = reinterpret_cast<uint8_t*>(aligned_alloc(MEMORY_ALIGNMENT, MEMORY_CHUNK_SIZE));
                if (memoryChunks[0] == nullptr) {
                    throw RuntimeException(10016, "couldn't allocate " + std::to_string(MEMORY_CHUNK_SIZE_MB) +
                                           " bytes memory for: " + module);
                }
                ++memoryChunksFree;
                ++memoryChunksAllocated;
            }

            if (memoryChunksAllocated > memoryChunksHWM)
                memoryChunksHWM = static_cast<uint64_t>(memoryChunksAllocated);
        }

        --memoryChunksFree;
        if (reusable)
            ++memoryChunksReusable;
        return memoryChunks[memoryChunksFree];
    }

    void Ctx::freeMemoryChunk(const char* module, uint8_t* chunk, bool reusable) {
        std::unique_lock<std::mutex> lck(memoryMtx);

        if (memoryChunksFree == memoryChunksAllocated)
            throw RuntimeException(50001, "trying to free unknown memory block for: " + std::string(module));

        // Keep memoryChunksMin reserved
        if (memoryChunksFree >= memoryChunksMin) {
            free(chunk);
            --memoryChunksAllocated;
        } else {
            memoryChunks[memoryChunksFree] = chunk;
            ++memoryChunksFree;
        }
        if (reusable)
            --memoryChunksReusable;

        condOutOfMemory.notify_all();
    }

    void Ctx::stopHard() {
        logTrace(TRACE_THREADS, "stop hard");

        {
            std::unique_lock<std::mutex> lck(mtx);

            if (hardShutdown)
                return;
            hardShutdown = true;
            softShutdown = true;

            condMainLoop.notify_all();
        }
        {
            std::unique_lock<std::mutex> lck(memoryMtx);
            condOutOfMemory.notify_all();
        }
    }

    void Ctx::stopSoft() {
        logTrace(TRACE_THREADS, "stop soft");

        std::unique_lock<std::mutex> lck(mtx);
        if (softShutdown)
            return;

        softShutdown = true;
        condMainLoop.notify_all();
    }

    void Ctx::mainFinish() {
        logTrace(TRACE_THREADS, "main finish start");

        while (wakeThreads()) {
            usleep(10000);
            wakeAllOutOfMemory();
        }

        while (!threads.empty()) {
            Thread* thread;
            {
                std::unique_lock<std::mutex> lck(mtx);
                thread = *(threads.begin());
            }
            finishThread(thread);
        }

        logTrace(TRACE_THREADS, "main finish end");
    }

    void Ctx::mainLoop() {
        logTrace(TRACE_THREADS, "main loop start");

        {
            std::unique_lock<std::mutex> lck(mtx);
            if (!hardShutdown) {
                if (trace & TRACE_SLEEP)
                    logTrace(TRACE_SLEEP, "Ctx:mainLoop");
                condMainLoop.wait(lck);
            }
        }

        logTrace(TRACE_THREADS, "main loop end");
    }

    void Ctx::printStacktrace() {
        void* array[128];
        int size;
        error(10014, "stacktrace for thread: " + std::to_string(reinterpret_cast<uint64_t>(pthread_self())));
        {
            std::unique_lock<std::mutex> lck(mtx);
            size = backtrace(array, 128);
        }
        backtrace_symbols_fd(array, size, STDERR_FILENO);
        error(10014, "stacktrace for thread: completed");
    }

    void Ctx::signalHandler(int s) {
        if (!hardShutdown) {
            error(10015, "caught signal: " + s);
            stopHard();
        }
    }

    bool Ctx::wakeThreads() {
        logTrace(TRACE_THREADS, "wake threads");

        bool wakingUp = false;
        {
            std::unique_lock<std::mutex> lck(mtx);
            for (Thread* thread: threads) {
                if (!thread->finished) {
                    logTrace(TRACE_THREADS, "waking up thread: " + thread->alias);
                    thread->wakeUp();
                    wakingUp = true;
                }
            }
        }
        wakeAllOutOfMemory();

        return wakingUp;
    }

    void Ctx::spawnThread(Thread* thread) {
        logTrace(TRACE_THREADS, "spawn: " + thread->alias);

        if (pthread_create(&thread->pthread, nullptr, &Thread::runStatic, reinterpret_cast<void*>(thread)))
            throw RuntimeException(10013, "spawning thread: " + thread->alias);
        {
            std::unique_lock<std::mutex> lck(mtx);
            threads.insert(thread);
        }
    }

    void Ctx::finishThread(Thread* thread) {
        logTrace(TRACE_THREADS, "finish: " + thread->alias);

        std::unique_lock<std::mutex> lck(mtx);
        if (threads.find(thread) == threads.end())
            return;
        threads.erase(thread);
        pthread_join(thread->pthread, nullptr);
    }

    std::ostringstream& Ctx::writeEscapeValue(std::ostringstream& ss, const std::string& str) {
        const char* c_str = str.c_str();
        for (uint64_t i = 0; i < str.length(); ++i) {
            if (*c_str == '\t') {
                ss << "\\t";
            } else if (*c_str == '\r') {
                ss << "\\r";
            } else if (*c_str == '\n') {
                ss << "\\n";
            } else if (*c_str == '\b') {
                ss << "\\b";
            } else if (*c_str == '\f') {
                ss << "\\f";
            } else if (*c_str == '"' || *c_str == '\\') {
                ss << '\\' << *c_str;
            } else if (*c_str < 32) {
                ss << "\\u00" << map16[(*c_str >> 4) & 0x0F] << map16[*c_str & 0x0F];
            } else {
                ss << *c_str;
            }
            ++c_str;
        }
        return ss;
    }

    bool Ctx::checkNameCase(const char* name) {
        uint64_t num = 0;
        while (*(name + num) != 0) {
            if (islower((unsigned char)*(name + num)))
                return false;

            if (num == 1024)
                throw DataException(20004, "identifier '" + std::string(name) + "' is too long");
            ++num;
        }

        return true;
    }

    void Ctx::releaseBuffer() {
        std::unique_lock<std::mutex> lck(memoryMtx);
        ++buffersFree;
    }

    void Ctx::allocateBuffer() {
        std::unique_lock<std::mutex> lck(memoryMtx);
        --buffersFree;
        if (readBufferMax - buffersFree > buffersMaxUsed)
            buffersMaxUsed = readBufferMax - buffersFree;
    }

    void Ctx::signalDump() {
        if (mainThread == pthread_self()) {
            std::unique_lock<std::mutex> lck(mtx);
            for (Thread* thread : threads)
                pthread_kill(thread->pthread, SIGUSR1);
        }
    }

    void Ctx::welcome(const std::string& message) {
        int code = 0;
        if (OLR_LOCALES == OLR_LOCALES_TIMESTAMP) {
            std::ostringstream s;
            time_t now = time(nullptr);
            tm nowTm = *localtime(&now);
            char str[50];
            strftime(str, sizeof(str), "%F %T", &nowTm);
            s << str << " INFO  " << std::setw(5) << std::setfill('0') << std::dec << code << " " << message << std::endl;
            std::cerr << s.str();
        } else {
            std::ostringstream s;
            s << " INFO  " << std::setw(5) << std::setfill('0') << std::dec << code << " " << message << std::endl;
            std::cerr << s.str();
        }
    }

    void Ctx::hint(const std::string& message) {
        if (logLevel < LOG_LEVEL_ERROR)
            return;

        if (OLR_LOCALES == OLR_LOCALES_TIMESTAMP) {
            std::ostringstream s;
            time_t now = time(nullptr);
            tm nowTm = *localtime(&now);
            char str[50];
            strftime(str, sizeof(str), "%F %T", &nowTm);
            s << str << " HINT  " << message << std::endl;
            std::cerr << s.str();
        } else {
            std::ostringstream s;
            s << "HINT  " << message << std::endl;
            std::cerr << s.str();
        }
    }

    void Ctx::error(int code, const std::string& message) {
        if (logLevel < LOG_LEVEL_ERROR)
            return;

        if (OLR_LOCALES == OLR_LOCALES_TIMESTAMP) {
            std::ostringstream s;
            time_t now = time(nullptr);
            tm nowTm = *localtime(&now);
            char str[50];
            strftime(str, sizeof(str), "%F %T", &nowTm);
            s << str << " ERROR " << std::setw(5) << std::setfill('0') << std::dec << code << " " << message << std::endl;
            std::cerr << s.str();
        } else {
            std::ostringstream s;
            s << "ERROR " << std::setw(5) << std::setfill('0') << std::dec << code << " " << message << std::endl;
            std::cerr << s.str();
        }
    }

    void Ctx::warning(int code, const std::string& message) {
        if (logLevel < LOG_LEVEL_WARNING)
            return;

        if (OLR_LOCALES == OLR_LOCALES_TIMESTAMP) {
            std::ostringstream s;
            time_t now = time(nullptr);
            tm nowTm = *localtime(&now);
            char str[50];
            strftime(str, sizeof(str), "%F %T", &nowTm);
            s << str << " WARN  " << std::setw(5) << std::setfill('0') << std::dec << code << " " << message << std::endl;
            std::cerr << s.str();
        } else {
            std::ostringstream s;
            s << "WARN  " << std::setw(5) << std::setfill('0') << std::dec << code << " " << message << std::endl;
            std::cerr << s.str();
        }
    }

    void Ctx::info(int code, const std::string& message) {
        if (logLevel < LOG_LEVEL_INFO)
            return;

        if (OLR_LOCALES == OLR_LOCALES_TIMESTAMP) {
            std::ostringstream s;
            time_t now = time(nullptr);
            tm nowTm = *localtime(&now);
            char str[50];
            strftime(str, sizeof(str), "%F %T", &nowTm);
            s << str << " INFO  " << std::setw(5) << std::setfill('0') << std::dec << code << " " << message << std::endl;
            std::cerr << s.str();
        } else {
            std::ostringstream s;
            s << "INFO  " << std::setw(5) << std::setfill('0') << std::dec << code << " " << message << std::endl;
            std::cerr << s.str();
        }
    }

    void Ctx::debug(int code, const std::string& message) {
        if (logLevel < LOG_LEVEL_DEBUG)
            return;

        if (OLR_LOCALES == OLR_LOCALES_TIMESTAMP) {
            std::ostringstream s;
            time_t now = time(nullptr);
            tm nowTm = *localtime(&now);
            char str[50];
            strftime(str, sizeof(str), "%F %T", &nowTm);
            s << str << " DEBUG " << std::setw(5) << std::setfill('0') << std::dec << code << " " << message << std::endl;
            std::cerr << s.str();
        } else {
            std::ostringstream s;
            s << "DEBUG " << std::setw(5) << std::setfill('0') << std::dec << code << " " << message << std::endl;
            std::cerr << s.str();
        }
    }

    void Ctx::logTrace(int mask, const std::string& message) {
        const char* code = "XXXXX";
        if ((trace & mask) == 0)
            return;

        switch (mask) {
            case TRACE_DML:
                code = "DML  ";
                break;

            case TRACE_DUMP:
                code = "DUMP ";
                break;

            case TRACE_LOB:
                code = "LOB  ";
                break;

            case TRACE_LWN:
                code = "LWN  ";
                break;

            case TRACE_THREADS:
                code = "THRD ";
                break;

            case TRACE_SQL:
                code = "SQL  ";
                break;

            case TRACE_FILE:
                code = "FILE ";
                break;

            case TRACE_DISK:
                code = "DISK ";
                break;

            case TRACE_PERFORMANCE:
                code = "PERFM";
                break;

            case TRACE_TRANSACTION:
                code = "TRANX";
                break;

            case TRACE_REDO:
                code = "REDO ";
                break;

            case TRACE_ARCHIVE_LIST:
                code = "ARCHL";
                break;

            case TRACE_SCHEMA_LIST:
                code = "SCHEM";
                break;

            case TRACE_WRITER:
                code = "WRITR";
                break;

            case TRACE_CHECKPOINT:
                code = "CHKPT";
                break;

            case TRACE_SYSTEM:
                code = "SYSTM";
                break;

            case TRACE_LOB_DATA:
                code = "LOBDT";
                break;

            case TRACE_SLEEP:
                code = "SLEEP";
                break;
        }

        if (OLR_LOCALES == OLR_LOCALES_TIMESTAMP) {
            std::ostringstream s;
            time_t now = time(nullptr);
            tm nowTm = *localtime(&now);
            char str[50];
            strftime(str, sizeof(str), "%F %T", &nowTm);
            s << str << " TRACE " << code << " " << message << '\n';
            std::cerr << s.str();
        } else {
            std::ostringstream s;
            s << "TRACE " << code << " " << message << '\n';
            std::cerr << s.str();
        }
    }
}
