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

typedef struct {
    long currentPosition;
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

class AudioFileDecoder2: public IAudioDataProvider {
public:
    AudioFileDecoder2();
    ~AudioFileDecoder2();
    void openFile(const char *inputFile);
    void closeInput();

    void getAudioData(int16_t *audioData, int *sampleCount);
    void decode();



private:

    bool initComponents(const char* file);


    void resetComponents();

    thread *decodeThread = NULL;

//    inline PCMBufferNode* getFreeNode();
//    inline PCMBufferNode* getDataNode();
//    inline void putNodeToDeque(PCMBufferNode *node);
//    inline void putNodeToUsedDeque(PCMBufferNode *node);

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
    PCMBufferNode *usingNode = NULL;

    DecodeState decodeState;

    int64_t seekTarget = 0;
    bool seekReq = false;

    bool stopFlag = false;
};


#endif //FFMPEGAUDIOPLAYER_AUDIOFILEDECODER2_H
