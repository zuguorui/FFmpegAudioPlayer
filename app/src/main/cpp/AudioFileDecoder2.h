//
// Created by incus-pc-2 on 2019/12/26.
//

#ifndef FFMPEGAUDIOPLAYER_AUDIOFILEDECODER2_H
#define FFMPEGAUDIOPLAYER_AUDIOFILEDECODER2_H


#include "IAudioDataProvider.h"

#include <iostream>
#include <stdlib.h>
#include <thread>
#include <list>
#include "AACUtil.h"
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
#include "libavutil/mem.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}

#include "Commons.h"

using namespace std;

/**
 * Audio file decoder, it is also responsible for update audio duration, attached pic and current play position.
 * All time unit is millisecond(ms)
 * */

typedef void (*InfoGetCallback)(int64_t, int32_t);
typedef void (*ProgressChangedCallback)(int64_t, bool);

typedef struct {
    int64_t pts;
    int sampleCount;
    int16_t *buffer;
}PCMBufferNode;

enum DecodeState
{
    NORMAL,
    NEED_READ_FIRST,
    NEED_NEW_PACKET,
    FINISHED
};

enum FileDecodeState
{
    DECODING,
    DECODING_FINISHED,
    INIT_COMPONENT_FAILED
};

class AudioFileDecoder2: public IAudioDataProvider {
public:
    AudioFileDecoder2();
    ~AudioFileDecoder2();
    void openFile(const char *inputFile);
    void closeInput();
    void setInfoGetCallback(InfoGetCallback callback);
    void setProgressChangedCallback(ProgressChangedCallback callback);
    void removeInfoGetCallback();
    void removeProgressChangedCallback();
    void getAudioData(int16_t *audioData, int *sampleCount);
    void decode();

    int getPicBufferLen();

    const int8_t *getPicData();

    void seekTo(int64_t position); //ms

    int64_t progressUpdateInterval = 1000;
    int64_t oldPosition = 0;


private:

    bool initComponents(const char* file);


    void resetComponents();

    thread *decodeThread = NULL;

    inline PCMBufferNode* getFreeNode();


    uint8_t *picBuffer = NULL;
    int picBufferLen = 0;

    AVFormatContext *formatCtx = NULL;
    int audioIndex = -1;
    AVStream *audioStream = NULL;

    AVCodecContext *codecCtx = NULL;
    AVCodec *codec = NULL;

    SwrContext *convertCtx;

    int32_t in_sample_rate = 0;
    int32_t in_channels = 0;
    AVSampleFormat in_sample_fmt;
    uint64_t in_channel_layout = 0;

    int64_t duration = 0;
    int64_t currentPosition = 0;

    static const int32_t out_sample_rate = 44100;
    static const int32_t out_channels = 2;
    static const AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    static const uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;

    mutex bufferMu;
    mutex usedBufferMu;
    condition_variable notFullSignal;
    condition_variable notEmptySignal;

    list<PCMBufferNode *> buffers;
    list<PCMBufferNode *> usedBuffers;

    FileDecodeState fileDecodeState;

    int64_t seekTarget = 0;
    bool seekReq = false;

    bool stopFlag = false;

    InfoGetCallback infoGetCallback = NULL;
    ProgressChangedCallback progressChangedCallback = NULL;

    void discardAllDecodedData();
};


#endif //FFMPEGAUDIOPLAYER_AUDIOFILEDECODER2_H
