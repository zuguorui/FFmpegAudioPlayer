#include <jni.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <android/log.h>
#include <sstream>
#include "AudioFilePlayer.h"

#define MODULE_NAME  "native-lib"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, MODULE_NAME, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, MODULE_NAME, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, MODULE_NAME, __VA_ARGS__)

using namespace std;

AudioFilePlayer *player = NULL;

extern "C" JNIEXPORT jstring JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nCreatePlayer(JNIEnv *env, jobject *instance)
{
    player = new AudioFilePlayer();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nOpenFile(JNIEnv *env, jobject *instance, jstring filePath)
{
    if(player == NULL)
    {
        LOGE("player is NULL");
        return false;
    }
    const char *pathChars = env->GetStringUTFChars(filePath, NULL);


    return player->openFile(pathChars);


}


extern "C" JNIEXPORT void JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nCloseInput(JNIEnv *env, jobject *instance)
{
    if(player == NULL)
    {
        LOGE("player is NULL");
        return;
    }

    player->closeInput();
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nReleasePlayer(JNIEnv *env, jobject *instance)
{
    if(player == NULL)
    {
        LOGE("player is NULL");
        return;
    }
    delete(player);
}


extern "C" JNIEXPORT void JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nStartPlay(JNIEnv *env, jobject *instance)
{
    if(player == NULL)
    {
        LOGE("player is NULL");
        return;
    }
    player->startPlay();
}


extern "C" JNIEXPORT void JNICALL
Java_com_zu_ffmpegaudioplayer_MainActivity_nStopPlay(JNIEnv *env, jobject *instance)
{
    if(player == NULL)
    {
        LOGE("player is NULL");
        return;
    }
    player->stopPlay();
}

