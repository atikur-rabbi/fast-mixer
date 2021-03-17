package com.bluehub.fastmixer.screens.mixing

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.*
import com.bluehub.fastmixer.common.models.AudioFileUiState
import com.bluehub.fastmixer.databinding.ListItemAudioFileBinding
import kotlinx.coroutines.Deferred

class AudioFileListAdapter(
    private val items: MutableList<AudioFileUiState>,
    private val clickListener: AudioFileEventListeners,
    private val fileWaveViewStore: FileWaveViewStore)
    : RecyclerView.Adapter<AudioFileListAdapter.ViewHolder>() {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        return ViewHolder.from(parent)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.bind(items[position], clickListener, fileWaveViewStore)
    }

    class ViewHolder private constructor(val binding: ListItemAudioFileBinding): RecyclerView.ViewHolder(binding.root) {
        fun bind(item: AudioFileUiState, clickListener: AudioFileEventListeners, fileWaveViewStore: FileWaveViewStore) {
            binding.fileWaveViewWidget.apply {
                setAudioFileUiState(item)
                setAudioFileEventListeners(clickListener)
                setFileWaveViewStore(fileWaveViewStore)
            }

            binding.executePendingBindings()
        }

        companion object {
            fun from(parent: ViewGroup): ViewHolder {
                val layoutInflater = LayoutInflater.from(parent.context)
                val binding = ListItemAudioFileBinding.inflate(layoutInflater, parent, false)
                return ViewHolder(binding)
            }
        }
    }

    override fun getItemCount(): Int {
        return items.size
    }

    fun addAtIndex(addIdx: Int) {
        notifyItemInserted(addIdx)
    }

    fun removeAtIndex(removeIdx: Int) {
        notifyItemRemoved(removeIdx)
    }
}

class AudioFileEventListeners(
    var readSamplesCallback: (String) -> (Int) -> Deferred<Array<Float>>,
    var deleteFileCallback: (String) -> Unit,
    var togglePlayCallback: (String) -> Unit
) {
    fun readSamplesCallbackWithIndex(filePath: String): (Int)->Deferred<Array<Float>> = readSamplesCallback(filePath)
}
