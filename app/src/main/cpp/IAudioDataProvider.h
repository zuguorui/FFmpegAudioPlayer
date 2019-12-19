//
// Created by incus on 2019-12-11.
//

#ifndef FFMPEGAUDIOPLAYER_IAUDIODATAPROVIDER_H
#define FFMPEGAUDIOPLAYER_IAUDIODATAPROVIDER_H

#include <stdlib.h>

class IAudioDataProvider {
public:
    virtual void getAudioData(int16_t *audioData, int *sampleCount) = 0;
};


#endif //FFMPEGAUDIOPLAYER_IAUDIODATAPROVIDER_H
