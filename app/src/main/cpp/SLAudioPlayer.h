//
// Created by zu on 2019-12-10.
//

#ifndef FFMPEGAUDIOPLAYER_SLAUDIOPLAYER_H
#define FFMPEGAUDIOPLAYER_SLAUDIOPLAYER_H


class SLAudioPlayer {
public:
    SLAudioPlayer();
    ~SLAudioPlayer();
    bool createPlayer();
    void releasePlayer();
    bool play();
    bool stop();

};


#endif //FFMPEGAUDIOPLAYER_SLAUDIOPLAYER_H
