package com.zu.ffmpegaudioplayer.data

import android.content.ContentResolver
import android.content.Context
import android.database.Cursor
import android.provider.MediaStore

data class AudioFile(val path: String, val id: Long)
{
    var audioName: String? = null
    var artist: String? = null
    var album: String? = null
    var size: Int? = null
    var duration: Int? = null
    var fileName: String? = null
    var albumId: Long? = null

}

fun loadAudioFromMediaStore(context: Context): ArrayList<AudioFile>
{
    var list: ArrayList<AudioFile> = ArrayList()
    var contentResolver: ContentResolver = context.contentResolver
    var cursor: Cursor? = contentResolver.query(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, null, null, null, MediaStore.Audio.Media.DEFAULT_SORT_ORDER)
    if(cursor?.moveToFirst() ?: false)
    {
        do{
            if (cursor!!.getLong(cursor!!.getColumnIndexOrThrow(MediaStore.Audio.Media.DURATION)) > 3000)
            {
                var path = cursor!!.getString(cursor!!.getColumnIndexOrThrow(MediaStore.Audio.Media.DATA))
                var id = cursor!!.getLong(cursor!!.getColumnIndexOrThrow(MediaStore.Audio.Media._ID))
                val song = AudioFile(path, id).apply {
                    this.audioName = cursor.getString(cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.TITLE))
                    this.artist = cursor.getString(cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.ARTIST))
                    this.album = cursor.getString(cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.ALBUM))
                    this.fileName = cursor.getString(cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.DISPLAY_NAME))
                    this.duration = cursor.getInt(cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.DURATION))
                    this.size = cursor.getInt(cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.SIZE))
                    this.albumId = cursor.getLong(cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.ALBUM_ID))
                }

                list.add(song)

            }
        }while (cursor!!.moveToNext())
    }
    return list
}