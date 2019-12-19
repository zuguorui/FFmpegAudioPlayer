//
// Created by rikson on 2019/12/16.
//

#ifndef FFMPEGAUDIOPLAYER_AUDIOFILEPLAYER_H
#define FFMPEGAUDIOPLAYER_AUDIOFILEPLAYER_H

#include <iostream>
#include <stdlib.h>
#include "SLAudioPlayer.h"
#include "AudioFileDecoder.h"

using namespace std;

class AudioFilePlayer {
public:
    AudioFilePlayer();
    ~AudioFilePlayer();
    bool openFile(const char *filePath);
    void closeInput();

    bool startPlay();
    void stopPlay();
    void seekTo(int64_t position);

private:
    AudioFileDecoder *decoder = NULL;
    SLAudioPlayer *audioPlayer = NULL;
};


#endif //FFMPEGAUDIOPLAYER_AUDIOFILEPLAYER_H
