//
// Created by asalehin on 7/11/20.
//

#include <cstdint>
#include <cstdio>
#include <string>
#include <unistd.h>
#include "RecordingIO.h"
#include "logging_macros.h"
#include "Utils.h"
#include <mutex>
#include <condition_variable>
#include <sys/stat.h>
#include "Constants.h"
#include "AudioEngine.h"

std::mutex RecordingIO::mtx;
std::condition_variable RecordingIO::reallocated;
bool RecordingIO::is_reallocated = false;

bool RecordingIO::check_if_reallocated() {
    return is_reallocated;
}

bool RecordingIO::setup_audio_source() {
    if (!validate_audio_file()) {
        return false;
    }

    AudioProperties targetProperties{
            .channelCount = StreamConstants::mOutputChannelCount,
            .sampleRate = StreamConstants::mSampleRate
    };

    std::shared_ptr<FileDataSource> audioSource{
            FileDataSource::newFromCompressedFile(mRecordingFilePath.c_str(), targetProperties)
    };

    if (audioSource == nullptr) {
        return false;
    }

    int32_t playHead = 0;
    if (mRecordedTrack != nullptr) {
        playHead = mRecordedTrack->getPlayHead();
        if (playHead >= mRecordedTrack->getTotalSampleFrames()) {
            playHead = 0;
        }
    }

    mRecordedTrack = std::make_unique<Player>(audioSource, mStopPlaybackCallback);
    mRecordedTrack->setPlayHead(playHead);
    mRecordedTrack->setPlaying(true);

    return true;
}

void RecordingIO::pause_audio_source() {
    if (mRecordedTrack == nullptr) {
        return;
    }
    mRecordedTrack->setPlaying(false);
}

void RecordingIO::stop_audio_source() {
    pause_audio_source();
    mRecordedTrack = nullptr;
}

bool RecordingIO::validate_audio_file() {
    return !(mRecordingFilePath.empty() || (access(mRecordingFilePath.c_str(), F_OK) == -1));
}

void RecordingIO::read_playback(float *targetData, int32_t numFrames, int32_t channelCount) {
    if (!validate_audio_file()) {
        return;
    }

    if (mRecordedTrack == nullptr) {
        return;
    }

    mRecordedTrack->renderAudio(targetData, numFrames);
}

void RecordingIO::flush_to_file(int16_t* buffer, int32_t length, const std::string& recordingFilePath, std::unique_ptr<SndfileHandle>& recordingFile) {
    if (recordingFile == nullptr) {
        int format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        SndfileHandle file = SndfileHandle(recordingFilePath, SFM_WRITE, format, StreamConstants::mInputChannelCount, StreamConstants::mSampleRate);
        recordingFile = std::make_unique<SndfileHandle>(file);
    }
    recordingFile->write(buffer, length);

    std::unique_lock<std::mutex> lck(mtx);
    reallocated.wait(lck, check_if_reallocated);
    delete[] buffer;
    is_reallocated = false;
    lck.unlock();
}

void RecordingIO::perform_flush(int flushIndex) {
    int16_t* oldBuffer = mData;
    is_reallocated = false;
    taskQueue->enqueue([&]() {
        flush_to_file(oldBuffer, flushIndex, mRecordingFilePath, mRecordingFile);
    });
    auto * newData = new int16_t[kMaxSamples]{0};
    std::copy(mData + flushIndex, mData + mWriteIndex, newData);
    mData = newData;
    is_reallocated = true;
    reallocated.notify_all();
    mWriteIndex -= flushIndex;
    mLivePlaybackReadIndex -= flushIndex;
    readyToFlush = false;
    toFlush = false;
    mIteration = 1;
}

int32_t RecordingIO::write(const int16_t *sourceData, int32_t numSamples) {
    if (mWriteIndex + numSamples > kMaxSamples) {
        readyToFlush = true;
    }

    int flushIndex = 0;
    if (readyToFlush) {
        int upperBound  = 0;
        if (mWriteIndex < kMaxSamples) {
            upperBound = mWriteIndex;
        } else {
            upperBound = kMaxSamples;
        }
        if (mLivePlaybackEnabled && mLivePlaybackReadIndex >= upperBound) {
            flushIndex = upperBound;
            toFlush = true;
        } else if (!mLivePlaybackEnabled) {
            flushIndex = mWriteIndex;
            toFlush = true;
        }
    }

    if (toFlush) {
        perform_flush(flushIndex);
    }

    if (mWriteIndex + numSamples > mIteration * kMaxSamples) {
        readyToFlush = true;
        mIteration++;
        int32_t newSize = mIteration * kMaxSamples;
        auto * newData = new int16_t[newSize]{0};
        std::copy(mData, mData + mWriteIndex, newData);
        delete[] mData;
        mData = newData;
    }

    for(int i = 0; i < numSamples; i++) {
        mData[mWriteIndex++] = sourceData[i] * gain_factor;
        if (currentMax < abs(sourceData[i])) {
            currentMax = abs(sourceData[i]);
        }
    }
    mTotalSamples += numSamples;

    return numSamples;
}

void RecordingIO::flush_buffer() {
    if (mWriteIndex > 0) {
        int16_t* oldBuffer = mData;
        is_reallocated = false;
        int32_t flushIndex = mWriteIndex;
        taskQueue->enqueue([&]() {
            flush_to_file(oldBuffer, flushIndex, mRecordingFilePath, mRecordingFile);
        });
        mIteration = 1;
        mWriteIndex = 0;
        mLivePlaybackReadIndex = 0;
        mData = new int16_t[kMaxSamples]{0};
        is_reallocated = true;
        reallocated.notify_all();
        readyToFlush = false;
        toFlush = false;
    }
}

int32_t RecordingIO::read_live_playback(int16_t *targetData, int32_t numSamples) {
    if (mLivePlaybackReadIndex > mWriteIndex) {
        return 0;
    }
    int32_t framesRead = 0;
    auto framesToCopy = numSamples;
    if (mLivePlaybackReadIndex + numSamples >= mTotalSamples) {
        framesToCopy = mTotalSamples - mLivePlaybackReadIndex;
    }
    if (framesToCopy > 0) {
        memcpy(targetData, mData + mLivePlaybackReadIndex, framesToCopy * sizeof(int16_t));
        mLivePlaybackReadIndex += framesToCopy;
    }
    return framesRead;
}

void RecordingIO::sync_live_playback() {
    mLivePlaybackReadIndex = mWriteIndex;
}

void RecordingIO::setLivePlaybackEnabled(bool livePlaybackEnabled) {
    mLivePlaybackEnabled = livePlaybackEnabled;
}

int RecordingIO::getCurrentMax() {
    int temp = currentMax;
    currentMax = 0;
    return temp;
}

void RecordingIO::resetCurrentMax() {
    currentMax = 0;
}

void RecordingIO::setTogglePlaybackCallback(std::function<void()> stopPlaybackCallback) {
    mStopPlaybackCallback = stopPlaybackCallback;
}
