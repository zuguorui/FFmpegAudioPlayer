//
// Created by zu on 2019-12-10.
//

#ifndef FFMPEGAUDIOPLAYER_SLAUDIOPLAYER_H
#define FFMPEGAUDIOPLAYER_SLAUDIOPLAYER_H

#include <iostream>
#include <stdlib.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "IAudioDataProvider.h"
#include "Commons.h"

using namespace std;

typedef void (*PlayStateChangedCallback)(bool);

class SLAudioPlayer {
public:
    SLAudioPlayer();
    ~SLAudioPlayer();
    bool createPlayer();
    void releasePlayer();
    bool play();
    bool stop();

    void setDataProvider(IAudioDataProvider *provider);
    void removeDataProvider(IAudioDataProvider *provider);

    void processAudio();

    void setPlayStateChangedCallback(PlayStateChangedCallback callback);
    void removePlayStateChangedCallback();

private:
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine;

    SLObjectItf outputMixObject = NULL;


    SLObjectItf playerObject = NULL;
    SLPlayItf playerPlay;

    SLAndroidSimpleBufferQueueItf playerBufferQueue;



    IAudioDataProvider *dataProvider = NULL;

    IAudioDataProvider *spareDataProvider = NULL;

    bool removeAudioDataProviderFlag = false;

    int16_t *buffer = NULL;

    PlayStateChangedCallback stateCallback = NULL;
};


#endif //FFMPEGAUDIOPLAYER_SLAUDIOPLAYER_H
