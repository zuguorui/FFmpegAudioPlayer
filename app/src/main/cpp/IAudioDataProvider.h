//
// Created by incus on 2019-12-11.
//

#ifndef FFMPEGAUDIOPLAYER_IAUDIODATAPROVIDER_H
#define FFMPEGAUDIOPLAYER_IAUDIODATAPROVIDER_H

#include <stdlib.h>

class IAudioDataProvider {
public:
    void getAudioData(int16_t *audio_data, int *samples);
};


#endif //FFMPEGAUDIOPLAYER_IAUDIODATAPROVIDER_H
