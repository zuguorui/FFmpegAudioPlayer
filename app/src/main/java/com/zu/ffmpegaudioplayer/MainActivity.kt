package com.zu.ffmpegaudioplayer

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.zu.ffmpegaudioplayer.data.AudioFile
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    private var songList: ArrayList<AudioFile>? = null
        set(value) {
            field = value
            if(value != null)
            {

            }
        }

    private var adapter:




    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)


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
