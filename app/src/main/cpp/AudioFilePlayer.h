//
// Created by rikson on 2019/12/16.
//

#ifndef FFMPEGAUDIOPLAYER_AUDIOFILEPLAYER_H
#define FFMPEGAUDIOPLAYER_AUDIOFILEPLAYER_H

#include <iostream>
#include <stdlib.h>
#include "SLAudioPlayer.h"
#include "AudioFileDecoder2.h"

using namespace std;

#define MAX_FILE_PATH_LEN 200

class AudioFilePlayer {
public:
    AudioFilePlayer();
    ~AudioFilePlayer();
    bool openFile(const char *filePath);
    void closeInput();

    bool startPlay();
    void stopPlay();
    void seekTo(int64_t position);

    void setStateCallback(PlayStateChangedCallback callback);
    void setInfoGetCallback(InfoGetCallback callback);
    void setProgressChangedCallback(ProgressChangedCallback callback);

    void removeStateCallback();
    void removeInfoGetCallback();
    void removeProgressChangedCallback();

    const int8_t *getPicData();

    int getPicBufferLen();

private:
    AudioFileDecoder2 *decoder = NULL;
    SLAudioPlayer *audioPlayer = NULL;

    char *currentFile = NULL;
};


#endif //FFMPEGAUDIOPLAYER_AUDIOFILEPLAYER_H
