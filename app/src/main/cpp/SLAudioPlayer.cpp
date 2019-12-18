//
// Created by zu on 2019-12-10.
//

#include "SLAudioPlayer.h"

#define MODULE_NAME  "SLAudioPlayer"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, MODULE_NAME, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, MODULE_NAME, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, MODULE_NAME, __VA_ARGS__)

void audio_callback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    SLAudioPlayer *player = (SLAudioPlayer *)context;
    player->processAudio();
}

void SLAudioPlayer::processAudio() {

    if(spareDataProvider != NULL)
    {
        dataProvider = spareDataProvider;
        spareDataProvider = NULL;
    }
    if(removeAudioDataProviderFlag)
    {
        removeAudioDataProviderFlag = false;
        dataProvider = NULL;
    }
    if(dataProvider != NULL)
    {
        int16_t *data = NULL;
        int num_samples = 0;
        dataProvider->getAudioData(data, &num_samples);
        (*playerBufferQueue)->Enqueue(playerBufferQueue, data, num_samples * 2 * sizeof(int16_t));
    }
}

SLAudioPlayer::SLAudioPlayer() {

}

SLAudioPlayer::~SLAudioPlayer() {

}

bool SLAudioPlayer::createPlayer() {

    SLresult result;

    //create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }
    (void*)result;

    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }
    (void*)result;

    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }
    (void*)result;

    SLInterfaceID ids1[1] = {SL_IID_OUTPUTMIX};
    SLboolean reqs1[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, ids1, reqs1);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }
    (void*)result;

    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }
    (void*)result;

    // Create player
    SLDataLocator_AndroidSimpleBufferQueue bufferQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM pcmFormat = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                  SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};

    SLDataSource audioSrc = {&bufferQueue, &pcmFormat};

    SLDataLocator_OutputMix locOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSink = {&locOutputMix, NULL};

    SLInterfaceID ids2[2] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    SLboolean reqs2[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_FALSE};

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &audioSrc, &audioSink, 2, ids2, reqs2);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }
    (void*)result;

    result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }
    (void*)result;

    result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }
    (void*)result;

    result = (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &playerBufferQueue);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }
    (void*)result;

    result = (*playerBufferQueue)->RegisterCallback(playerBufferQueue, audio_callback, this);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }
    (void*)result;

//    result = (*playerObject)->GetInterface(playerObject, SL_IID_VOLUME, &bqPlayerVolume);
//    if(result != SL_RESULT_SUCCESS)
//    {
//        return false;
//    }
//    (void*)result;

    result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PAUSED);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }
    (void*)result;

    int fakeBufferLen = 10;
    int16_t *fakeBuffer = (int16_t *)malloc(fakeBufferLen * sizeof(int16_t));
    memset(fakeBuffer, 0, fakeBufferLen * sizeof(int16_t));
    (*playerBufferQueue)->Enqueue(playerBufferQueue, fakeBuffer, fakeBufferLen * sizeof(int16_t));
    free(fakeBuffer);

    return true;
}

void SLAudioPlayer::releasePlayer() {
    if(playerPlay != NULL)
    {
        (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
        playerPlay = NULL;
        (*playerObject)->Destroy(playerObject);
        playerObject = NULL;
        playerBufferQueue = NULL;

    }
    if(engineObject != NULL)
    {
        (*engineObject)->Destroy(engineObject);
    }
}

bool SLAudioPlayer::play() {
    if(playerPlay == NULL)
    {
        return false;
    }

    SLresult result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);

    return result == SL_RESULT_SUCCESS;
}

bool SLAudioPlayer::stop() {
    if(playerPlay == NULL)
    {
        return false;
    }
    SLresult result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
    return result == SL_RESULT_SUCCESS;
}

void SLAudioPlayer::setDataProvider(IAudioDataProvider *provider) {
    this->spareDataProvider = provider;
}

void SLAudioPlayer::removeDataProvider(IAudioDataProvider *provider) {
    if(provider == NULL || this->dataProvider == NULL)
    {
        return;
    }
    if(this->dataProvider == provider)
    {
        removeAudioDataProviderFlag = true;
    }
}





