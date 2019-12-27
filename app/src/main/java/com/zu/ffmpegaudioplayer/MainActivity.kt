package com.zu.ffmpegaudioplayer

import android.Manifest
import android.content.pm.PackageManager
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.MotionEvent
import android.widget.AdapterView
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

        nCreatePlayer()

        adapter.itemClickListener = {
            position: Int ->
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
        nReleasePlayer()
        super.onDestroy()
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
