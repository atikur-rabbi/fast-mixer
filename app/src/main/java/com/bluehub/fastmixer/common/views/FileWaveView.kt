package com.bluehub.fastmixer.common.views

import android.content.Context
import android.graphics.Canvas
import android.graphics.Paint
import android.graphics.Typeface
import android.util.AttributeSet
import android.view.View
import androidx.databinding.BindingMethod
import androidx.databinding.BindingMethods


@BindingMethods(value = [
    BindingMethod(type = FileWaveView::class, attribute = "allSamplesReader", method = "setAllSamplesReader"),
    BindingMethod(type = FileWaveView::class, attribute = "audioFilePath", method = "setAudioFilePath"),
    BindingMethod(type = FileWaveView::class, attribute = "totalSampleCountReader", method = "setTotalSampleCountReader")
])
class FileWaveView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null
) : View(context, attrs) {
    private lateinit var mAudioFilePath: String

    lateinit var mAllSamplesReader: () -> Array<Float>
    lateinit var mTotalSampleCountReader: () -> Int

    private var fileLoaded = false
    private var totalSampleCount: Int = 0

    private val paint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        style = Paint.Style.FILL
        textAlign = Paint.Align.CENTER
        textSize = 15.0f
        typeface = Typeface.create("", Typeface.BOLD)
    }

    fun setAudioFilePath(audioFilePath: String) {
        mAudioFilePath = audioFilePath
        checkAndSetupAudioFileSource()
    }

    fun setAllSamplesReader(allSamplesReader: () -> Array<Float>) {
        mAllSamplesReader = allSamplesReader
    }

    fun setTotalSampleCountReader(totalSampleCountReader: () -> Int) {
        mTotalSampleCountReader = totalSampleCountReader
        checkAndSetupAudioFileSource()
    }

    private fun checkAndSetupAudioFileSource() {
        if (::mAudioFilePath.isInitialized
            && ::mTotalSampleCountReader.isInitialized) {
            totalSampleCount = mTotalSampleCountReader()
        }
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        if(::mAudioFilePath.isInitialized) {
            canvas.drawText(mAudioFilePath, 10F, 10F, paint)
        }
    }
}
