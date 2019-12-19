//
// Created by rikson on 2019/12/16.
//

#include <android/log.h>
#include "AudioFilePlayer.h"

#define MODULE_NAME  "AudioFilePlayer"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, MODULE_NAME, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, MODULE_NAME, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, MODULE_NAME, __VA_ARGS__)

AudioFilePlayer::AudioFilePlayer() {
    decoder = new AudioFileDecoder();
    audioPlayer = new SLAudioPlayer();
    audioPlayer->setDataProvider(decoder);
    audioPlayer->createPlayer();
}

AudioFilePlayer::~AudioFilePlayer() {

    if(audioPlayer != NULL)
    {
        audioPlayer->releasePlayer();
        audioPlayer->removeDataProvider(decoder);
        delete(audioPlayer);
    }

    if(decoder != NULL)
    {
        delete(decoder);
    }
}

bool AudioFilePlayer::openFile(const char *filePath) {
    if(audioPlayer == NULL)
    {
        LOGE("audioPlayer is NULL");
        return false;
    }
    if(decoder != NULL)
    {
        return decoder->openFile(filePath);
    }else{
        LOGE("decoder is NULL");
        return false;
    }
}

void AudioFilePlayer::closeInput() {
    if(decoder != NULL)
    {
        decoder->closeInput();
    }else{
        LOGE("decoder is NULL");
    }
}

bool AudioFilePlayer::startPlay() {
    if(audioPlayer != NULL && decoder != NULL)
    {
        return audioPlayer->play();
    }else if(audioPlayer == NULL){
        LOGE("audioPlayer is NULL");
        return false;
    }else{
        LOGE("decoder is NULL");
        return false;
    }
}

void AudioFilePlayer::stopPlay() {
    if(audioPlayer != NULL && decoder != NULL)
    {
        audioPlayer->stop();
    }else if(audioPlayer == NULL){
        LOGE("audioPlayer is NULL");
    }else{
        LOGE("decoder is NULL");
    }
}

void AudioFilePlayer::seekTo(int64_t position) {
    if(audioPlayer != NULL && decoder != NULL)
    {
        decoder->seekTo(position);
    }else if(audioPlayer == NULL){
        LOGE("audioPlayer is NULL");
    }else{
        LOGE("decoder is NULL");
    }
}
