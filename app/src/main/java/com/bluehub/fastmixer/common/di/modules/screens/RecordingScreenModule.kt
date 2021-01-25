package com.bluehub.fastmixer.common.di.modules.screens

import androidx.lifecycle.ViewModel
import com.bluehub.fastmixer.common.di.FragmentScope
import com.bluehub.fastmixer.common.di.ViewModelKey
import com.bluehub.fastmixer.screens.recording.RecordingScreenControlViewModel
import dagger.Binds
import dagger.Module
import dagger.multibindings.IntoMap

@Module
interface RecordingScreenModule {
    @Binds
    @IntoMap
    @FragmentScope
    @ViewModelKey(RecordingScreenControlViewModel::class)
    fun recordingScreenViewModel(viewModel: RecordingScreenControlViewModel): ViewModel
}
