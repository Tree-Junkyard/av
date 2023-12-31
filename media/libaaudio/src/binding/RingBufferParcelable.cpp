/*
 * Copyright 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "RingBufferParcelable"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <stdint.h>

#include <binder/Parcelable.h>
#include <utility/AAudioUtilities.h>

#include "binding/AAudioServiceDefinitions.h"
#include "binding/SharedRegionParcelable.h"
#include "binding/RingBufferParcelable.h"

using namespace aaudio;

RingBufferParcelable::RingBufferParcelable(const RingBuffer& parcelable)
        : mReadCounterParcelable(parcelable.readCounterParcelable),
          mWriteCounterParcelable(parcelable.writeCounterParcelable),
          mDataParcelable(parcelable.dataParcelable),
          mBytesPerFrame(parcelable.bytesPerFrame),
          mFramesPerBurst(parcelable.framesPerBurst),
          mCapacityInFrames(parcelable.capacityInFrames),
          mFlags(static_cast<RingbufferFlags>(parcelable.flags)) {
    static_assert(sizeof(mFlags) == sizeof(parcelable.flags));
}

RingBuffer RingBufferParcelable::parcelable() const {
    RingBuffer result;
    result.readCounterParcelable = mReadCounterParcelable.parcelable();
    result.writeCounterParcelable = mWriteCounterParcelable.parcelable();
    result.dataParcelable = mDataParcelable.parcelable();
    result.bytesPerFrame = mBytesPerFrame;
    result.framesPerBurst = mFramesPerBurst;
    result.capacityInFrames = mCapacityInFrames;
    static_assert(sizeof(mFlags) == sizeof(result.flags));
    result.flags = static_cast<int32_t>(mFlags);
    return result;
}

// TODO This assumes that all three use the same SharedMemoryParcelable
void RingBufferParcelable::setupMemory(int32_t sharedMemoryIndex,
                 int32_t dataMemoryOffset,
                 int32_t dataSizeInBytes,
                 int32_t readCounterOffset,
                 int32_t writeCounterOffset,
                 int32_t counterSizeBytes) {
    mReadCounterParcelable.setup({sharedMemoryIndex, readCounterOffset, counterSizeBytes});
    mWriteCounterParcelable.setup({sharedMemoryIndex, writeCounterOffset, counterSizeBytes});
    mDataParcelable.setup({sharedMemoryIndex, dataMemoryOffset, dataSizeInBytes});
}

void RingBufferParcelable::setupMemory(int32_t sharedMemoryIndex,
                 int32_t dataMemoryOffset,
                 int32_t dataSizeInBytes) {
    mReadCounterParcelable.setup({sharedMemoryIndex, 0, 0});
    mWriteCounterParcelable.setup({sharedMemoryIndex, 0, 0});
    mDataParcelable.setup({sharedMemoryIndex, dataMemoryOffset, dataSizeInBytes});
}

void RingBufferParcelable::setupMemory(
        const SharedRegionParcelable::MemoryInfoTuple& dataMemoryInfo,
        const SharedRegionParcelable::MemoryInfoTuple& readCounterInfo,
        const SharedRegionParcelable::MemoryInfoTuple& writeCounterInfo) {
    mReadCounterParcelable.setup(readCounterInfo);
    mWriteCounterParcelable.setup(writeCounterInfo);
    mDataParcelable.setup(dataMemoryInfo);
}

int32_t RingBufferParcelable::getBytesPerFrame() const {
    return mBytesPerFrame;
}

void RingBufferParcelable::setBytesPerFrame(int32_t bytesPerFrame) {
    mBytesPerFrame = bytesPerFrame;
}

int32_t RingBufferParcelable::getFramesPerBurst() const {
    return mFramesPerBurst;
}

void RingBufferParcelable::setFramesPerBurst(int32_t framesPerBurst) {
    mFramesPerBurst = framesPerBurst;
}

int32_t RingBufferParcelable::getCapacityInFrames() const {
    return mCapacityInFrames;
}

void RingBufferParcelable::setCapacityInFrames(int32_t capacityInFrames) {
    mCapacityInFrames = capacityInFrames;
}

aaudio_result_t RingBufferParcelable::resolve(SharedMemoryParcelable *memoryParcels, RingBufferDescriptor *descriptor) {
    aaudio_result_t result;

    result = mReadCounterParcelable.resolve(memoryParcels,
                                            (void **) &descriptor->readCounterAddress);
    if (result != AAUDIO_OK) {
        return result;
    }

    result = mWriteCounterParcelable.resolve(memoryParcels,
                                             (void **) &descriptor->writeCounterAddress);
    if (result != AAUDIO_OK) {
        return result;
    }

    result = mDataParcelable.resolve(memoryParcels, (void **) &descriptor->dataAddress);
    if (result != AAUDIO_OK) {
        return result;
    }

    descriptor->bytesPerFrame = mBytesPerFrame;
    descriptor->framesPerBurst = mFramesPerBurst;
    descriptor->capacityInFrames = mCapacityInFrames;
    descriptor->flags = mFlags;
    return AAUDIO_OK;
}

void RingBufferParcelable::updateMemory(const RingBufferParcelable& parcelable,
                                        const std::map<int32_t, int32_t>& memoryIndexMap) {
    setupMemory(parcelable.mDataParcelable.getMemoryInfo(&memoryIndexMap),
                parcelable.mReadCounterParcelable.getMemoryInfo(&memoryIndexMap),
                parcelable.mWriteCounterParcelable.getMemoryInfo(&memoryIndexMap));
    setBytesPerFrame(parcelable.getBytesPerFrame());
    setFramesPerBurst(parcelable.getFramesPerBurst());
    setCapacityInFrames(parcelable.getCapacityInFrames());
}

aaudio_result_t RingBufferParcelable::validate() const {
    if (mCapacityInFrames < 0 || mCapacityInFrames >= 32 * 1024) {
        ALOGE("invalid mCapacityInFrames = %d", mCapacityInFrames);
        return AAUDIO_ERROR_INTERNAL;
    }
    if (mBytesPerFrame < 0 || mBytesPerFrame >= 256) {
        ALOGE("invalid mBytesPerFrame = %d", mBytesPerFrame);
        return AAUDIO_ERROR_INTERNAL;
    }
    if (mFramesPerBurst < 0 || mFramesPerBurst >= 16 * 1024) {
        ALOGE("invalid mFramesPerBurst = %d", mFramesPerBurst);
        return AAUDIO_ERROR_INTERNAL;
    }
    return AAUDIO_OK;
}

void RingBufferParcelable::dump() {
    ALOGD("mCapacityInFrames = %d ---------", mCapacityInFrames);
    if (mCapacityInFrames > 0) {
        ALOGD("mBytesPerFrame = %d", mBytesPerFrame);
        ALOGD("mFramesPerBurst = %d", mFramesPerBurst);
        ALOGD("mFlags = %u", mFlags);
        mReadCounterParcelable.dump();
        mWriteCounterParcelable.dump();
        mDataParcelable.dump();
    }
}
