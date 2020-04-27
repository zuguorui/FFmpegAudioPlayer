//
// Created by zu on 2019-12-10.
//

#include "SLAudioPlayer.h"

#define MODULE_NAME  "SLAudioPlayer"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)

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

        int num_samples = 0;
        memset(buffer, 0, MAX_SAMPLE_COUNT * 2 * sizeof(int16_t));
        dataProvider->getAudioData(buffer, &num_samples);
        (*playerBufferQueue)->Enqueue(playerBufferQueue, buffer, num_samples * 2 * sizeof(int16_t));
    }
}

SLAudioPlayer::SLAudioPlayer() {
    buffer = (int16_t *)malloc(MAX_SAMPLE_COUNT * 2 * sizeof(int16_t));
}

SLAudioPlayer::~SLAudioPlayer() {
    if(buffer != NULL)
    {
        free(buffer);
    }
}

bool SLAudioPlayer::createPlayer() {

    SLresult result;

    //create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    SLInterfaceID ids1[1] = {SL_IID_OUTPUTMIX};
    SLboolean reqs1[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, ids1, reqs1);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


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


    result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    result = (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &playerBufferQueue);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    result = (*playerBufferQueue)->RegisterCallback(playerBufferQueue, audio_callback, this);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


//    result = (*playerObject)->GetInterface(playerObject, SL_IID_VOLUME, &bqPlayerVolume);
//    if(result != SL_RESULT_SUCCESS)
//    {
//        return false;
//    }


    result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PAUSED);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    //主动Enqueue一次buffer，OpenSLES才会主动向我们请求数据
    (*playerBufferQueue)->Enqueue(playerBufferQueue, buffer, MAX_SAMPLE_COUNT * 2 * sizeof(int16_t));


    return true;
}

void SLAudioPlayer::releasePlayer() {
    if(playerPlay != NULL)
    {
        (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
        if(stateCallback != NULL)
        {
            stateCallback(false);
        }
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

void SLAudioPlayer::setPlayStateChangedCallback(PlayStateChangedCallback callback) {
    stateCallback = callback;
}

void SLAudioPlayer::removePlayStateChangedCallback() {
    stateCallback = NULL;
}

bool SLAudioPlayer::play() {
    if(playerPlay == NULL)
    {
        return false;
    }

    SLresult result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);

    if(stateCallback != NULL)
    {
        stateCallback(true);
    }
    return result == SL_RESULT_SUCCESS;
}

bool SLAudioPlayer::stop() {
    if(playerPlay == NULL)
    {
        return false;
    }
    SLresult result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
    if(stateCallback != NULL)
    {
        stateCallback(false);
    }
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





