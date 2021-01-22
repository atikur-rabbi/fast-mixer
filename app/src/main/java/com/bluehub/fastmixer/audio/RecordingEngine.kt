package com.bluehub.fastmixer.audio


class RecordingEngine {
    companion object {
        init {
            System.loadLibrary("audioEngine")
        }

        @JvmStatic external fun create(
            appPathStr: String,
            recordingSessionIdStr: String,
            recordingScreenViewModelPassed: Boolean = false): Boolean

        @JvmStatic external fun delete()

        @JvmStatic external fun startRecording()

        @JvmStatic external fun stopRecording()

        @JvmStatic external fun startLivePlayback()

        @JvmStatic external fun stopLivePlayback()

        @JvmStatic external fun startPlayback(): Boolean

        @JvmStatic external fun stopPlayback()

        @JvmStatic external fun pausePlayback()

        @JvmStatic external fun flushWriteBuffer()

        @JvmStatic external fun restartPlayback()

        @JvmStatic external fun getCurrentMax(): Int

        @JvmStatic external fun resetCurrentMax()

        @JvmStatic external fun getTotalRecordedFrames(): Int

        @JvmStatic external fun getCurrentPlaybackProgress(): Int

        @JvmStatic external fun setPlayHead(position: Int)

        @JvmStatic external fun getDurationInSeconds(): Int

        @JvmStatic external fun addSources(filePaths: Array<String>)

        @JvmStatic external fun resetRecordingEngine()
    }
}
