package com.zu.ffmpegaudioplayer

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.zu.ffmpegaudioplayer.data.AudioFile
import java.text.SimpleDateFormat

fun formatDuration(duration: Int): String
{
    val totalSeconds: Int = (duration / 1000)
    val totalMinutes = totalSeconds / 60
    val seconds = totalSeconds % 60
    val minutes = totalMinutes % 60
    val hours = totalMinutes / 60


    return if(hours != 0) "$minutes:$seconds" else "$hours:$minutes:$seconds"
}

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
        if(data == null || position >= data!!.size)
        {
            return
        }

        holder.tvName.text = data!![position].audioName
        holder.tvDuration.text = formatDuration(data!![position].duration!!)

    }
}

class SongListItem(itemView: View): RecyclerView.ViewHolder(itemView)
{
    var tvName: TextView
    var tvDuration: TextView
    init {
        tvName = itemView.findViewById(R.id.tv_name)
        tvDuration = itemView.findViewById(R.id.tv_duration)
    }
}