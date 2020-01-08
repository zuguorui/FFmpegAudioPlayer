//
// Created by rikson on 2019/12/16.
//

#include <android/log.h>
#include "AudioFilePlayer.h"

#define MODULE_NAME  "AudioFilePlayer"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)



AudioFilePlayer::AudioFilePlayer() {
    decoder = new AudioFileDecoder2();
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
        if(currentFile != NULL)
        {
            free(currentFile);
        }
        int nameLen = strlen(filePath);
        currentFile = (char *)malloc(nameLen * sizeof(char));
        memset(currentFile, 0, nameLen * sizeof(char));
        strcpy(currentFile, filePath);
        decoder->openFile(currentFile);
        return true;
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

void AudioFilePlayer::setStateCallback(PlayStateChangedCallback callback) {
    if(audioPlayer == NULL)
    {
        LOGE("audio player is NULL when set state callback");
        return;
    }
    audioPlayer->setPlayStateChangedCallback(callback);
}

void AudioFilePlayer::setInfoGetCallback(InfoGetCallback callback) {
    if(decoder == NULL)
    {
        LOGE("file decoder is NULL when set information get callback");
        return;
    }
    decoder->setInfoGetCallback(callback);
}

void AudioFilePlayer::setProgressChangedCallback(ProgressChangedCallback callback) {
    if(decoder == NULL)
    {
        LOGE("file decoder is NULL when set progress changed callback");
        return;
    }
    decoder->setProgressChangedCallback(callback);
}

void AudioFilePlayer::removeStateCallback() {
    if(audioPlayer == NULL)
    {
        LOGE("audio player is NULL when remove state callback");
        return;
    }
    audioPlayer->removePlayStateChangedCallback();
}

void AudioFilePlayer::removeInfoGetCallback() {
    if(decoder == NULL)
    {
        LOGE("file decoder is NULL when remove information get callback");
        return;
    }
    decoder->removeInfoGetCallback();
}

void AudioFilePlayer::removeProgressChangedCallback() {
    if(decoder == NULL)
    {
        LOGE("file decoder is NULL when remove progress changed callback");
        return;
    }
    decoder->removeProgressChangedCallback();
}

const int8_t* AudioFilePlayer::getPicData() {
    if(decoder == NULL)
    {
        LOGE("file decoder is NULL when get pic buffer");
        return NULL;
    }

    return decoder->getPicData();
}

int AudioFilePlayer::getPicBufferLen() {
    if(decoder == NULL)
    {
        LOGE("file decoder is NULL when get pic buffer len");
        return NULL;
    }

    return decoder->getPicBufferLen();
}
