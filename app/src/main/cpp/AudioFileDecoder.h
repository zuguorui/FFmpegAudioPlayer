//
// Created by zu on 2019-12-11.
//

#ifndef FFMPEGAUDIOPLAYER_AUDIOFILEDECODER_H
#define FFMPEGAUDIOPLAYER_AUDIOFILEDECODER_H

#include <stdlib.h>
#include <iostream>
#include <thread>

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
}

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
    NEED_NEW_PACKET
};

class AudioFileDecoder: IAudioDataProvider {
public:

    void getAudioData(int16_t *audio_data, int *sampleCount);
    bool openFile(string filePath);
    long getDuration();
    long getCurrentPosition();
    uint8_t *picBuffer = NULL;

private:
    int32_t bufferLength = BUFFER_SIZE;
    int32_t bufferCount = 10;

    AVFormatContext *formatContext = NULL;
    int streamIndex = -1;
    AVStream *audioStream = NULL;
    AVCodecContext *codecContext = NULL;

    int32_t in_sample_rate = 0, out_sample_rate = 0;
    int32_t in_channels = 0, out_channels = 0;
    AVSampleFormat in_sample_fmt, out_sample_fmt;
    uint64_t in_channel_layout = 0, out_channel_layout = 0;

    SwrContext *swrContext = NULL;





};


#endif //FFMPEGAUDIOPLAYER_AUDIOFILEDECODER_H
