package com.zu.ffmpegaudioplayer

import android.Manifest
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Handler
import android.os.Message
import android.util.Log
import android.view.MotionEvent
import android.widget.AdapterView
import android.widget.SeekBar
import androidx.core.app.ActivityCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.zu.ffmpegaudioplayer.data.AudioFile
import com.zu.ffmpegaudioplayer.data.loadAudioFromMediaStore
import io.reactivex.Observable
import io.reactivex.Scheduler
import io.reactivex.SingleObserver
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.schedulers.Schedulers
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    private val TAG = "MainActivity"

    private var pic: Bitmap? = null

    private var songList: ArrayList<AudioFile>? = null
        set(value) {
            field = value
            adapter.data = value
        }


    private var adapter: SongListAdapter = SongListAdapter()

    private var songIndex = -1

    private var currentSongFinished = false

    private var isPlay: Boolean = false

    private var handler: Handler = Handler{
        when(it.what)
        {
            PROGRESS_UPDATE -> {
                val ms: Long = it.obj as Long
                val s: Int = (ms / 1000).toInt()
                sb_progress.progress = s
                tv_position.text = formatDuration(ms)
            }
            PLAY_STATE_UPDATE -> {
                btn_play.text = if(it.obj as Boolean) "停止" else "开始"
                isPlay = it.obj as Boolean
            }
            INFO_GET -> {
                val ms: Long = it.obj as Long
                val s: Int = (ms / 1000).toInt()
                tv_duration1.text = formatDuration(ms)
                sb_progress.max = s
                if(pic != null)
                {
                    iv_pic.setImageBitmap(pic!!)
                }

            }
        }
        return@Handler true
    }


    private var duration: Long = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        songIndex = -1
        isPlay = false
        val layoutManager = LinearLayoutManager(this,RecyclerView.VERTICAL, false)
        rv_song.adapter = adapter
        rv_song.layoutManager = layoutManager

        btn_play.setOnClickListener {
            if(isPlay)
            {
                nStopPlay()
            }else{
                if(songIndex == -1)
                {
                    if((songList?.size ?: 0) == 0)
                    {
                        return@setOnClickListener
                    }else{
                        songIndex = 0
                        nOpenFile(songList!![songIndex].path)
                    }
                }
                nStartPlay()
            }
        }

        btn_next.setOnClickListener {
            playNext()
        }

        btn_previous.setOnClickListener {
            playPrevious()
        }

        sb_progress.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener{
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                Log.d(TAG, "progress changed to $progress")
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
                Log.d(TAG, "start track")
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                Log.d(TAG, "stop tracking, progress is ${seekBar!!.progress}")
                nSeekTo(seekBar!!.progress.toLong() * 1000)
            }
        })

        nCreatePlayer()
        nSetStateListener(this)
        adapter.itemClickListener = {
            position: Int ->
            songIndex = position
            nOpenFile(songList!![position].path)
            nStartPlay()
        }

        Observable.fromCallable {
            loadAudioFromMediaStore(this)
        }
            .subscribeOn(Schedulers.io())
            .observeOn(AndroidSchedulers.mainThread())
            .subscribe {
                songList = it
            }
        checkPermission()
    }

    override fun onDestroy() {
        nStopPlay()
        nCloseInput()
        nRemoveStateListener()
        nReleasePlayer()
        super.onDestroy()
    }

    private fun playNext()
    {
        if((songList?.size ?: 0) == 0)
        {
            return
        }
        if(songIndex == -1)
        {
            songIndex = 0
        }else{

            songIndex++
            if(songIndex >= songList!!.size)
            {
                songIndex = 0
            }
        }
        nOpenFile(songList!![songIndex].path)
        nStartPlay()
    }

    private fun playPrevious()
    {
        if((songList?.size ?: 0) == 0)
        {
            return
        }
        if(songIndex == -1)
        {
            songIndex = songList!!.size - 1
        }else{

            songIndex--
            if(songIndex < 0)
            {
                songIndex = songList!!.size - 1
            }
        }
        nOpenFile(songList!![songIndex].path)
        nStartPlay()
    }

    fun listPermissions(): ArrayList<String>
    {
        var result = ArrayList<String>()
        result.add(Manifest.permission.READ_EXTERNAL_STORAGE)
        result.add(Manifest.permission.WRITE_EXTERNAL_STORAGE)
        return result
    }

    fun checkPermission()
    {
        val permissions = listPermissions()
        var allGet = true
        for(permission in permissions)
        {
            if(ActivityCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED)
            {
                Log.e(TAG, "permission $permission not granted")
                allGet = false
            }else{
                Log.d(TAG, "permission $permission granted")
            }
        }

        if(!allGet)
        {
            var permissionArray: Array<String> = Array(permissions.size){i: Int -> permissions[i] }

            ActivityCompat.requestPermissions(this, permissionArray, 33)
        }
    }



    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        when(requestCode){
            33 -> {
                for(i in grantResults.indices)
                {
                    if (grantResults[i] != PackageManager.PERMISSION_GRANTED)
                    {
                        Log.e(TAG, "permission ${permissions[i]} not granted")
                    }else{
                        Log.d(TAG, "permission ${permissions[i]} granted")
                    }
                }
            }
        }
    }

    fun onInfoGet(duration: Long, picBufferLen: Int)
    {
        pic?.recycle()
        pic = null
        if(picBufferLen > 0)
        {
            var rawArray: ByteArray = ByteArray(picBufferLen)
            nGetPicData(rawArray)
            pic = BitmapFactory.decodeByteArray(rawArray, 0, picBufferLen)
//            iv_pic.setImageBitmap(pic)
        }
        this.duration = duration
        handler.sendMessage(Message().apply {
            what = INFO_GET
            obj = duration
        })

    }

    fun onProgressChanged(progress: Long, isPlayFinished: Boolean)
    {
        handler.sendMessage(Message().apply {
            what = PROGRESS_UPDATE
            obj = progress
        })

        if(isPlayFinished)
        {
            playNext()
        }
    }

    fun onPlayStateChanged(isPlay: Boolean)
    {
        handler.sendMessage(Message().apply {
            what = PLAY_STATE_UPDATE
            obj = isPlay
        })
    }


    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    private external fun stringFromJNI(): String
    private external fun nCreatePlayer()
    private external fun nOpenFile(filePath: String): Boolean
    private external fun nCloseInput()
    private external fun nReleasePlayer()
    private external fun nStartPlay()
    private external fun nStopPlay()
    private external fun nSetStateListener(listener: Any)
    private external fun nRemoveStateListener()
    private external fun nSeekTo(position: Long)
    private external fun nGetPicData(picBuffer: ByteArray)


    companion object {
        const val PROGRESS_UPDATE: Int = 1
        const val PLAY_STATE_UPDATE: Int = 2
        const val INFO_GET: Int = 3
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
}
