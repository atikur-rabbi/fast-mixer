/*
 * Copyright 2018 The Android Open Source Project
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

#include "Player.h"
#include "../logging_macros.h"

void Player::addSource(string key, shared_ptr<DataSource> source) {
    mSourceMap.insert(pair<string, shared_ptr<DataSource>>(key, source));
}

void Player::renderAudio(float *targetData, int32_t numFrames) {
    const AudioProperties properties = mSourceMap.begin()->second->getProperties();

    if (mIsPlaying){

        int64_t framesToRenderFromData = numFrames;
        int64_t totalSourceFrames = mSourceMap.begin()->second->getSize() / properties.channelCount;
        const float *data = mSourceMap.begin()->second->getData();

        // Check whether we're about to reach the end of the recording
        if (!mIsLooping && mReadFrameIndex + numFrames >= totalSourceFrames){
            framesToRenderFromData = totalSourceFrames - mReadFrameIndex;
            mIsPlaying = false;
            if (mStopPlaybackCallback) {
                mStopPlaybackCallback();
            }
        }

        for (int i = 0; i < framesToRenderFromData; ++i) {
            for (int j = 0; j < properties.channelCount; ++j) {
                targetData[(i*properties.channelCount)+j] = data[(mReadFrameIndex*properties.channelCount)+j];
            }

            // Increment and handle wraparound
            if (++mReadFrameIndex >= totalSourceFrames) mReadFrameIndex = 0;
        }

        if (framesToRenderFromData < numFrames) {
            // fill the rest of the buffer with silence
            renderSilence(&targetData[framesToRenderFromData], numFrames * properties.channelCount);
        }

    } else {
        renderSilence(targetData, numFrames * properties.channelCount);
    }
}

void Player::renderSilence(float *start, int32_t numSamples){
    for (int i = 0; i < numSamples; ++i) {
        start[i] = 0;
    }
}

void Player::setPlayHead(int32_t playHead) {
    const AudioProperties properties = mSourceMap.begin()->second->getProperties();
    int64_t totalSourceFrames = mSourceMap.begin()->second->getSize() / properties.channelCount;
    if (playHead < totalSourceFrames) {
        mReadFrameIndex = playHead;
    }
}

int64_t Player::getTotalSampleFrames() {
    auto source = mSourceMap.begin()->second;
    const AudioProperties properties = source->getProperties();
    int64_t totalSourceFrames = source->getSize() / properties.channelCount;
    return totalSourceFrames;
}
