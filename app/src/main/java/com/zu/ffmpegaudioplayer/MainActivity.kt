package com.zu.ffmpegaudioplayer

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
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

    private var songList: ArrayList<AudioFile>? = null
        set(value) {
            field = value
            adapter.data = value
        }

    private var adapter: SongListAdapter = SongListAdapter()




    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val layoutManager = LinearLayoutManager(this,RecyclerView.VERTICAL, false)
        rv_song.adapter = adapter
        rv_song.layoutManager = layoutManager

        Observable.fromCallable {
            loadAudioFromMediaStore(this)
        }
            .subscribeOn(Schedulers.io())
            .observeOn(AndroidSchedulers.mainThread())
            .subscribe {
                songList = it
            }
    }



    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String
    external fun nCreatePlayer()
    external fun nOpenFile(filePath: String): Boolean
    external fun nCloseInput()
    external fun nReleasePlayer()
    external fun nStartPlay()
    external fun nStopPlay()


    companion object {

        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
}
