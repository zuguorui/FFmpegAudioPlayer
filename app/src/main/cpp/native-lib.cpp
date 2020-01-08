#include <jni.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <android/log.h>
#include <sstream>
#include "AudioFilePlayer.h"
#include "JavaStateListener.h"

#define MODULE_NAME  "native-lib"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)

using namespace std;

AudioFilePlayer *player = NULL;

JavaStateListener *javaStateListener = NULL;

void stateChangedCallback(bool isPlay)
{
    if(javaStateListener != NULL)
    {
        javaStateListener->playStateChanged(isPlay);
    }
}

void infoGetCallback(int64_t duration, int32_t picSize)
{
    if(javaStateListener != NULL)
    {
        javaStateListener->infoGet(duration, picSize);
    }
}

void progressChangedCallback(int64_t position, bool isPlayFinished)
{
    if(javaStateListener != NULL)
    {
        javaStateListener->progressChanged(position, isPlayFinished);
    }
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nCreatePlayer(JNIEnv *env, jobject instance)
{
    player = new AudioFilePlayer();
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nSetStateListener(JNIEnv *env, jobject instance, jobject listener)
{
    if(player == NULL)
    {
        LOGE("player is NULL when set listener");
    }

    if(javaStateListener != NULL)
    {
        delete(javaStateListener);
    }
    javaStateListener = new JavaStateListener(env, listener);
//    javaStateListener->infoGet(1000, 0);
//    javaStateListener->progressChanged(100);
//    javaStateListener->playStateChanged(true);
    //TODO: Add callback functions to AudioFilePlayer
    player->setStateCallback(stateChangedCallback);
    player->setInfoGetCallback(infoGetCallback);
    player->setProgressChangedCallback(progressChangedCallback);
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nRemoveStateListener(JNIEnv *env, jobject instance)
{

    if(player == NULL)
    {
        LOGE("player is NULL when set listener");
    }
    player->removeStateCallback();
    player->removeInfoGetCallback();
    player->removeProgressChangedCallback();
    if(javaStateListener != NULL)
    {
        delete(javaStateListener);
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nOpenFile(JNIEnv *env, jobject instance, jstring filePath)
{
    if(player == NULL)
    {
        LOGE("player is NULL");
        return false;
    }
    const char *pathChars = env->GetStringUTFChars(filePath, NULL);

    bool result =  player->openFile(pathChars);

    env->ReleaseStringUTFChars(filePath, pathChars);

    return result;

}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nSeekTo(JNIEnv *env, jobject instance, jlong position)
{
    if(player == NULL)
    {
        LOGE("player is NULL");
        return;
    }
    player->seekTo(position);
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nCloseInput(JNIEnv *env, jobject instance)
{
    if(player == NULL)
    {
        LOGE("player is NULL");
        return;
    }

    player->closeInput();
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nReleasePlayer(JNIEnv *env, jobject instance)
{
    if(player == NULL)
    {
        LOGE("player is NULL");
        return;
    }
    delete(player);
}


extern "C" JNIEXPORT void JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nStartPlay(JNIEnv *env, jobject instance)
{
    if(player == NULL)
    {
        LOGE("player is NULL");
        return;
    }
    player->startPlay();
}


extern "C" JNIEXPORT void JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nStopPlay(JNIEnv *env, jobject instance)
{
    if(player == NULL)
    {
        LOGE("player is NULL");
        return;
    }
    player->stopPlay();
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nGetPicData(JNIEnv *env, jobject instance, jbyteArray buffer)
{
    if(player == NULL)
    {
        LOGE("player is NULL");
        return;
    }
    int bufferLen = player->getPicBufferLen();
    const int8_t *picBuffer = player->getPicData();
    if(picBuffer != NULL)
    {
        env->SetByteArrayRegion(buffer, 0, bufferLen, picBuffer);
    }

}

