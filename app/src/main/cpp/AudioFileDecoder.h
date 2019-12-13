//
// Created by zu on 2019-12-11.
//

#ifndef FFMPEGAUDIOPLAYER_AUDIOFILEDECODER_H
#define FFMPEGAUDIOPLAYER_AUDIOFILEDECODER_H

#include <stdlib.h>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <deque>


#include "IAudioDataProvider.h"
#include "Commons.h"
extern "C"
{
#include "FFmpeg/libavutil/avstring.h"
#include "FFmpeg/libavutil/eval.h"
#include "FFmpeg/libavutil/mathematics.h"
#include "FFmpeg/libavutil/pixdesc.h"
#include "FFmpeg/libavutil/imgutils.h"
#include "FFmpeg/libavutil/dict.h"
#include "FFmpeg/libavutil/parseutils.h"
#include "FFmpeg/libavutil/samplefmt.h"
#include "FFmpeg/libavutil/avassert.h"
#include "FFmpeg/libavutil/time.h"
#include "FFmpeg/libavformat/avformat.h"
#include "FFmpeg/libswscale/swscale.h"
#include "FFmpeg/libavutil/opt.h"
#include "FFmpeg/libavcodec/avfft.h"
#include "FFmpeg/libswresample/swresample.h"
#include "FFmpeg/libavcodec/avcodec.h"
#include "FFmpeg/libavutil/frame.h"
#include "FFmpeg/libavutil/channel_layout.h"
}

using namespace std;

#define MAX_SAMPLE_COUNT 512

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

class AudioFileDecoder: IAudioDataProvider {
public:
    AudioFileDecoder();
    ~AudioFileDecoder();
    void getAudioData(int16_t *audio_data, int *sampleCount);
    bool openFile(string filePath);
    long getDuration();
    long getCurrentPosition();
    bool jumpToPosition(long position);


private:
    string currentFile = NULL;

    bool initComponent();
    void resetComponent();

    void decode();

    thread *decodeThread = NULL;

    int32_t bufferCount = 10;

    AVFormatContext *formatContext = NULL;
    int audioIndex = -1;
    AVStream *audioStream = NULL;
    AVCodecContext *codecContext = NULL;
    AVCodec *decoder = NULL;

    uint8_t *picBuffer = NULL;
    int picBufferLen = 0;

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

    SwrContext *swrContext = NULL;

    AVPacket *packet = NULL;
    AVFrame *frame = NULL;

    mutex mu;
    condition_variable cond;

    deque<PCMBufferNode *> bufferDeque;

    DecodeState decodeState;
};


#endif //FFMPEGAUDIOPLAYER_AUDIOFILEDECODER_H
