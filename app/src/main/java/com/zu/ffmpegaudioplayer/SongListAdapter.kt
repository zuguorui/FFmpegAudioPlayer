package com.zu.ffmpegaudioplayer

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.zu.ffmpegaudioplayer.data.AudioFile

class SongListAdapter: RecyclerView.Adapter<SongListItem>()
{
    var data: ArrayList<AudioFile>? = null
        set(value) {
            field = value
            notifyDataSetChanged()
        }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): SongListItem {
        val view: View = LayoutInflater.from(parent.context).inflate(R.layout.list_item_song, null, false)
        return SongListItem(view)
    }

    override fun getItemCount(): Int {
        return data?.size ?: 0
    }

    override fun onBindViewHolder(holder: SongListItem, position: Int) {

    }
}

class SongListItem(itemView: View): RecyclerView.ViewHolder(itemView)
{
    lateinit var tvName: TextView
    lateinit var tvDuration: TextView
    init {
        tvName = itemView.findViewById(R.id.tv_name)
        tvDuration = itemView.findViewById(R.id.tv_duration)
    }
}