//
// Created by zu on 2019-12-11.
//

#ifndef FFMPEGAUDIOPLAYER_AUDIOFILEDECODER_H
#define FFMPEGAUDIOPLAYER_AUDIOFILEDECODER_H

#include <stdlib.h>
#include "IAudioDataProvider.h"

class AudioFileDecoder: IAudioDataProvider {
public:

    void getAudioData(int16_t *audio_data, int *samples);

};


#endif //FFMPEGAUDIOPLAYER_AUDIOFILEDECODER_H
