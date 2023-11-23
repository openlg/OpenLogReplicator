/* Base class for thread to write output
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
<http:////www.gnu.org/licenses/>.  */

#include <fstream>
#include <thread>
#include <unistd.h>

#include "../builder/Builder.h"
#include "../common/Ctx.h"
#include "../common/DataException.h"
#include "../common/NetworkException.h"
#include "../common/RuntimeException.h"
#include "../metadata/Metadata.h"
#include "Writer.h"

namespace OpenLogReplicator {
    Writer::Writer(Ctx* newCtx, const std::string& newAlias, const std::string& newDatabase, Builder* newBuilder, Metadata* newMetadata) :
            Thread(newCtx, newAlias),
            database(newDatabase),
            builder(newBuilder),
            metadata(newMetadata),
            builderQueue(nullptr),
            checkpointScn(ZERO_SCN),
            checkpointIdx(0),
            checkpointTime(time(nullptr)),
            sentMessages(0),
            oldLength(0),
            currentQueueSize(0),
            maxQueueSize(0),
            streaming(false),
            confirmedScn(ZERO_SCN),
            confirmedIdx(0),
            queue(nullptr) {
    }

    Writer::~Writer() {
        if (queue != nullptr) {
            delete[] queue;
            queue = nullptr;
        }
    }

    void Writer::initialize() {
        if (queue != nullptr)
            return;
        queue = new BuilderMsg*[ctx->queueSize];
    }

    void Writer::createMessage(BuilderMsg* msg) {
        ++sentMessages;

        queue[currentQueueSize++] = msg;
        if (currentQueueSize > maxQueueSize)
            maxQueueSize = currentQueueSize;
    }

    void Writer::sortQueue() {
        if (currentQueueSize == 0)
            return;

        BuilderMsg** oldQueue = queue;
        queue = new BuilderMsg*[ctx->queueSize];
        uint64_t oldQueueSize = currentQueueSize;

        for (uint64_t newId = 0 ; newId < currentQueueSize; ++newId) {
            queue[newId] = oldQueue[0];
            uint64_t i = 0;
            --oldQueueSize;
            while (i < oldQueueSize) {
                if (i * 2 + 2 < oldQueueSize && oldQueue[i * 2 + 2]->id < oldQueue[oldQueueSize]->id) {
                    if (oldQueue[i * 2 + 1]->id < oldQueue[i * 2 + 2]->id) {
                        oldQueue[i] = oldQueue[i * 2 + 1];
                        i = i * 2 + 1;
                    } else {
                        oldQueue[i] = oldQueue[i * 2 + 2];
                        i = i * 2 + 2;
                    }
                } else if (i * 2 + 1 < oldQueueSize && oldQueue[i * 2 + 1]->id < oldQueue[oldQueueSize]->id) {
                    oldQueue[i] = oldQueue[i * 2 + 1];
                    i = i * 2 + 1;
                } else
                    break;
            }
            oldQueue[i] = oldQueue[oldQueueSize];
        }

        if (oldQueue != nullptr)
            delete[] oldQueue;
    }

    void Writer::resetMessageQueue() {
        for (uint64_t i = 0; i < currentQueueSize; ++i) {
            BuilderMsg* msg = queue[i];
            if ((msg->flags & OUTPUT_BUFFER_MESSAGE_ALLOCATED) != 0)
                delete[] msg->data;
        }
        currentQueueSize = 0;

        oldLength = builderQueue->start;
    }

    void Writer::confirmMessage(BuilderMsg* msg) {
        std::unique_lock<std::mutex> lck(mtx);

        if (msg == nullptr) {
            if (currentQueueSize == 0) {
                ctx->warning(70007, "trying to confirm an empty message");
                return;
            }
            msg = queue[0];
        }

        msg->flags |= OUTPUT_BUFFER_MESSAGE_CONFIRMED;
        if (msg->flags & OUTPUT_BUFFER_MESSAGE_ALLOCATED) {
            delete[] msg->data;
            msg->flags &= ~OUTPUT_BUFFER_MESSAGE_ALLOCATED;
        }

        uint64_t maxId = 0;
        {
            while (currentQueueSize > 0 && (queue[0]->flags & OUTPUT_BUFFER_MESSAGE_CONFIRMED) != 0) {
                maxId = queue[0]->queueId;
                if (confirmedScn == ZERO_SCN || msg->lwnScn > confirmedScn) {
                    confirmedScn = msg->lwnScn;
                    confirmedIdx = msg->lwnIdx;
                } else if (msg->lwnScn == confirmedScn && msg->lwnIdx > confirmedIdx)
                    confirmedIdx = msg->lwnIdx;

                if (--currentQueueSize == 0)
                    break;

                uint64_t i = 0;
                while (i < currentQueueSize) {
                    if (i * 2 + 2 < currentQueueSize && queue[i * 2 + 2]->id < queue[currentQueueSize]->id) {
                        if (queue[i * 2 + 1]->id < queue[i * 2 + 2]->id) {
                            queue[i] = queue[i * 2 + 1];
                            i = i * 2 + 1;
                        } else {
                            queue[i] = queue[i * 2 + 2];
                            i = i * 2 + 2;
                        }
                    } else if (i * 2 + 1 < currentQueueSize && queue[i * 2 + 1]->id < queue[currentQueueSize]->id) {
                        queue[i] = queue[i * 2 + 1];
                        i = i * 2 + 1;
                    } else
                        break;
                }
                queue[i] = queue[currentQueueSize];
            }
        }

        builder->releaseBuffers(maxId);
    }

    void Writer::run() {
        if (ctx->trace & TRACE_THREADS) {
            std::ostringstream ss;
            ss << std::this_thread::get_id();
            ctx->logTrace(TRACE_THREADS, "writer (" + ss.str() + ") start");
        }

        ctx->info(0, "writer is starting with " + getName());

        try {
            // Before anything, read the latest checkpoint
            readCheckpoint();
            builderQueue = builder->firstBuilderQueue;
            oldLength = 0;
            currentQueueSize = 0;

            // External loop for client disconnection
            while (!ctx->hardShutdown) {
                try {
                    mainLoop();

                // Client disconnected
                } catch (NetworkException& ex) {
                    ctx->warning(ex.code, ex.msg);
                    streaming = false;
                }

                if (ctx->softShutdown && ctx->replicatorFinished)
                    break;
            }
        } catch (DataException& ex) {
            ctx->error(ex.code, ex.msg);
            ctx->stopHard();
        } catch (RuntimeException& ex) {
            ctx->error(ex.code, ex.msg);
            ctx->stopHard();
        }

        ctx->info(0, "writer is stopping: " + getName() + ", max queue size: " + std::to_string(maxQueueSize));
        if (ctx->trace & TRACE_THREADS) {
            std::ostringstream ss;
            ss << std::this_thread::get_id();
            ctx->logTrace(TRACE_THREADS, "writer (" + ss.str() + ") stop");
        }
    }

    void Writer::mainLoop() {
        BuilderMsg* msg;
        uint64_t newLength = 0;
        currentQueueSize = 0;

        // Start streaming
        while (!ctx->hardShutdown) {
            // Check if the writer has a receiver of data which defined starting point of replication
            while (!ctx->hardShutdown) {
                pollQueue();

                if (streaming && metadata->status == METADATA_STATUS_REPLICATE)
                    break;

                if (ctx->trace & TRACE_WRITER)
                    ctx->logTrace(TRACE_WRITER, "waiting for client");
                usleep(ctx->pollIntervalUs);
            }

            // Get a message to send
            while (!ctx->hardShutdown) {
                // Verify sent messages, check what is received by client
                pollQueue();

                // Update checkpoint
                writeCheckpoint(false);

                // Next buffer
                if (builderQueue->next != nullptr)
                    if (builderQueue->length == oldLength) {
                        builderQueue = builderQueue->next;
                        oldLength = 0;
                    }

                // Found something
                msg = reinterpret_cast<BuilderMsg*>(builderQueue->data + oldLength);
                if (builderQueue->length > oldLength + sizeof(struct BuilderMsg) && msg->length > 0) {
                    newLength = builderQueue->length;
                    break;
                }

                if (ctx->softShutdown && ctx->replicatorFinished)
                    break;
                builder->sleepForWriterWork(currentQueueSize, ctx->pollIntervalUs);
            }

            // Send the message
            while (oldLength + sizeof(struct BuilderMsg) < newLength && !ctx->hardShutdown) {
                msg = reinterpret_cast<BuilderMsg*>(builderQueue->data + oldLength);
                if (msg->length == 0)
                    break;

                // The queue is full
                pollQueue();
                while (currentQueueSize >= ctx->queueSize && !ctx->hardShutdown) {
                    if (ctx->trace & TRACE_WRITER)
                        ctx->logTrace(TRACE_WRITER, "output queue is full (" + std::to_string(currentQueueSize) +
                                      " elements), sleeping " + std::to_string(ctx->pollIntervalUs) + "us");
                    usleep(ctx->pollIntervalUs);
                    pollQueue();
                }

                writeCheckpoint(false);
                if (ctx->hardShutdown)
                    break;

                uint64_t length8 = (msg->length + 7) & 0xFFFFFFFFFFFFFFF8;
                oldLength += sizeof(struct BuilderMsg);

                // Message in one part - send directly from buffer
                if (oldLength + length8 <= OUTPUT_BUFFER_DATA_SIZE) {
                    createMessage(msg);
                    // Send the message to the client in one part
                    if (((msg->flags & OUTPUT_BUFFER_MESSAGE_CHECKPOINT) && !FLAG(REDO_FLAGS_SHOW_CHECKPOINT)) ||
                            !metadata->isNewData(msg->lwnScn, msg->lwnIdx))
                        confirmMessage(msg);
                    else
                        sendMessage(msg);
                    oldLength += length8;

                // The message is split to many parts - merge & copy
                } else {
                    msg->data = new uint8_t[msg->length];
                    if (msg->data == nullptr)
                        throw RuntimeException(10016, "couldn't allocate " + std::to_string(msg->length) +
                                               " bytes memory for: temporary buffer for JSON message");
                    msg->flags |= OUTPUT_BUFFER_MESSAGE_ALLOCATED;

                    uint64_t copied = 0;
                    while (msg->length > copied) {
                        uint64_t toCopy = msg->length - copied;
                        if (toCopy > newLength - oldLength) {
                            toCopy = newLength - oldLength;
                            memcpy(reinterpret_cast<void*>(msg->data + copied),
                                   reinterpret_cast<const void*>(builderQueue->data + oldLength), toCopy);
                            builderQueue = builderQueue->next;
                            newLength = OUTPUT_BUFFER_DATA_SIZE;
                            oldLength = 0;
                        } else {
                            memcpy(reinterpret_cast<void*>(msg->data + copied),
                                   reinterpret_cast<const void*>(builderQueue->data + oldLength), toCopy);
                            oldLength += (toCopy + 7) & 0xFFFFFFFFFFFFFFF8;
                        }
                        copied += toCopy;
                    }

                    createMessage(msg);
                    // Send only new messages to the client
                    if (((msg->flags & OUTPUT_BUFFER_MESSAGE_CHECKPOINT) && !FLAG(REDO_FLAGS_SHOW_CHECKPOINT)) ||
                            !metadata->isNewData(msg->lwnScn, msg->lwnIdx))
                        confirmMessage(msg);
                    else
                        sendMessage(msg);
                    break;
                }
            }

            // All work done?
            if (ctx->softShutdown && ctx->replicatorFinished) {
                // Is there still some data to send?
                if (builderQueue->length != oldLength || builderQueue->next != nullptr)
                    continue;
                break;
            }
        }

        writeCheckpoint(true);
    }

    void Writer::writeCheckpoint(bool force) {
        // Nothing changed
        if ((checkpointScn == confirmedScn && checkpointIdx == confirmedIdx) || confirmedScn == ZERO_SCN)
            return;

        // Force first checkpoint
        if (checkpointScn == ZERO_SCN)
            force = true;

        // Not yet
        time_t now = time(nullptr);
        uint64_t timeSinceCheckpoint = (now - checkpointTime);
        if (timeSinceCheckpoint < ctx->checkpointIntervalS && !force)
            return;

        if (ctx->trace & TRACE_CHECKPOINT) {
            if (checkpointScn == ZERO_SCN)
                ctx->logTrace(TRACE_CHECKPOINT, "writer confirmed scn: " + std::to_string(confirmedScn) + " idx: " +
                        std::to_string(confirmedIdx));
            else
                ctx->logTrace(TRACE_CHECKPOINT, "writer confirmed scn: " + std::to_string(confirmedScn) + " idx: " +
                        std::to_string(confirmedIdx) + " checkpoint scn: " + std::to_string(checkpointScn) + " idx: " +
                        std::to_string(checkpointIdx));
        }
        std::string name(database + "-chkpt");
        std::ostringstream ss;
        ss << R"({"database":")" << database
                << R"(","scn":)" << std::dec << confirmedScn
                << R"(,"idx":)" << std::dec << confirmedIdx
                << R"(,"resetlogs":)" << std::dec << metadata->resetlogs
                << R"(,"activation":)" << std::dec << metadata->activation << "}";

        if (metadata->stateWrite(name, confirmedScn, ss)) {
            checkpointScn = confirmedScn;
            checkpointIdx = confirmedIdx;
            checkpointTime = now;
        }
    }

    void Writer::readCheckpoint() {
        std::ifstream infile;
        std::string name(database + "-chkpt");

        // Checkpoint is present - read it
        std::string checkpoint;
        rapidjson::Document document;
        if (!metadata->stateRead(name, CHECKPOINT_FILE_MAX_SIZE, checkpoint))
            return;

        if (checkpoint.length() == 0 || document.Parse(checkpoint.c_str()).HasParseError())
            throw DataException(20001, "file: " + name + " offset: " + std::to_string(document.GetErrorOffset()) +
                                " - parse error: " + GetParseError_En(document.GetParseError()));

        const char* databaseJson = Ctx::getJsonFieldS(name, JSON_PARAMETER_LENGTH, document, "database");
        if (database != databaseJson)
            throw DataException(20001, "file: " + name + " - invalid database name: " + databaseJson);

        metadata->setResetlogs(Ctx::getJsonFieldU32(name, document, "resetlogs"));
        metadata->setActivation(Ctx::getJsonFieldU32(name, document, "activation"));

        // Started earlier - continue work & ignore default startup parameters
        checkpointScn = Ctx::getJsonFieldU64(name, document, "scn");
        metadata->clientScn = checkpointScn;
        if (document.HasMember("idx"))
            checkpointIdx = Ctx::getJsonFieldU64(name, document, "idx");
        else
            checkpointIdx = 0;
        metadata->clientIdx = checkpointIdx;
        metadata->startScn = checkpointScn;
        metadata->startSequence = ZERO_SEQ;
        metadata->startTime.clear();
        metadata->startTimeRel = 0;

        ctx->info(0, "checkpoint - all confirmed till scn: " + std::to_string(checkpointScn) + ", idx: " +
                std::to_string(checkpointIdx));
        metadata->setStatusReplicate();
    }

    void Writer::wakeUp() {
        Thread::wakeUp();
        builder->wakeUp();
    }
}
