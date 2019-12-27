//
// Created by incus-pc-2 on 2019/12/26.
//

#include "AudioFileDecoder2.h"
#include <android/log.h>

#define BUFFER_QUEUE_SIZE 10

#define MODULE_NAME  "AudioFileDecoder2"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)

static void thread_callback(void *context, const char *file)
{
    AudioFileDecoder2 *fileDecoder = (AudioFileDecoder2 *)context;
    fileDecoder->decode();
}

AudioFileDecoder2::AudioFileDecoder2() {
    av_register_all();
}

AudioFileDecoder2::~AudioFileDecoder2() {

}

void AudioFileDecoder2::openFile(const char *inputFile) {
    closeInput();
    if(!initComponents(inputFile))
    {
        LOGE("init components error");
        resetComponents();
        return;
    }

    decodeThread = new thread(thread_callback, this, inputFile);
}

void AudioFileDecoder2::closeInput() {
    if(decodeThread != NULL && decodeThread->joinable())
    {
        LOGE("decode thread running... join it to wait finish by itself");
        stopFlag = true;
        decodeThread->join();
        resetComponents();
    }
    decodeThread = NULL;

    stopFlag = false;
}

bool AudioFileDecoder2::initComponents(const char* inputFileName) {


    int err = 0;

    formatCtx = avformat_alloc_context();
    if (!formatCtx)
    {
        LOGE("Can't allocate context\n");
        return false;
    }

    err = avformat_open_input(&formatCtx, inputFileName, NULL, NULL);
    if (err < 0)
    {
        LOGE("Can't open input file\n");
        return false;
    }

    if (avformat_find_stream_info(formatCtx, NULL) < 0)
    {
        LOGE("Error when find stream info\n");
        return false;
    }

    av_dump_format(formatCtx, 0, inputFileName, 0);

    audioIndex = -1;

    if(picBuffer != NULL)
    {
        free(picBuffer);
        picBuffer = NULL;
    }

    for (int i = 0; i < formatCtx->nb_streams; i++)
    {
        AVStream *st = formatCtx->streams[i];
        AVMediaType type = st->codecpar->codec_type;
        if (type == AVMEDIA_TYPE_AUDIO)
        {
            audioIndex = i;
        }
        if (st->disposition & AV_DISPOSITION_ATTACHED_PIC)
        {
            LOGE("pic index: %d\n", i);
            AVPacket p = st->attached_pic;



            picBufferLen = p.size;
            picBuffer = (uint8_t *)malloc(p.size * sizeof(uint8_t));
            memcpy(picBuffer, p.data, p.size * sizeof(uint8_t));

        }
    }

    if (audioIndex == -1)
    {
        LOGE("Can't find audio stream\n");
        return false;
    }

    LOGE("audio index is %d\n", audioIndex);

    audioStream = formatCtx->streams[audioIndex];


    codec = avcodec_find_decoder(audioStream->codecpar->codec_id);
    if (codec == NULL)
    {
        LOGE("can not find codec");
        return false;
    }

    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx)
    {
        LOGE("Error when alloc CodecContext\n");
        return false;
    }

    if (avcodec_parameters_to_context(codecCtx, audioStream->codecpar) < 0)
    {
        LOGE("Error when copy params to codec context");
        return false;
    }
    if (avcodec_open2(codecCtx, codec, NULL) < 0)
    {
        LOGE("Error when open codec\n");
        return false;
    }

    in_sample_rate = codecCtx->sample_rate;
    in_channels = codecCtx->channels;
    in_channel_layout = codecCtx->channel_layout;
    in_sample_fmt = codecCtx->sample_fmt;

    LOGE("Convert data format:\n");
    LOGE("    in_sample_rate: %d\n", in_sample_rate);
    LOGE("    in_sample_fmt: %d\n", in_sample_fmt);
    LOGE("    in_channels: %d\n", in_channels);
    LOGE("    in_channel_layout: %ld\n", in_channel_layout);

    LOGE("    out_sample_rate: %d\n", out_sample_rate);
    LOGE("    out_sample_fmt: %d\n", out_sample_fmt);
    LOGE("    out_channels: %d\n", out_channels);
    LOGE("    out_channel_layout: %ld\n", out_channel_layout);


    convertCtx = swr_alloc();
    convertCtx = swr_alloc_set_opts(convertCtx, out_channel_layout, out_sample_fmt, out_sample_rate, in_channel_layout, in_sample_fmt, in_sample_rate, 0, NULL);

    swr_init(convertCtx);

    duration = audioStream->duration;

    return true;
}

/*
 * Don't call this function if decode thread is running. I don't know why if call this
 * at init stage, initing components will crash with SIGNAL FAULT.
 * */
void AudioFileDecoder2::resetComponents() {
    if(convertCtx != NULL)
    {
        swr_free(&convertCtx);
        convertCtx = NULL;
    }

    if(codecCtx != NULL)
    {
        avcodec_close(codecCtx);
        avcodec_free_context(&codecCtx);
        codecCtx = NULL;
    }

    codec = NULL;

    if(formatCtx != NULL)
    {
        avformat_close_input(&formatCtx);
        avformat_free_context(formatCtx);
        formatCtx = NULL;
    }
}

void AudioFileDecoder2::decode() {

    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(packet);

    AVFrame *frame = av_frame_alloc();


    DecodeState decode_state = NORMAL;

    unique_lock<mutex> bufferLock(bufferMu);
    bufferLock.unlock();

    unique_lock<mutex> usedBufferLock(usedBufferMu);
    usedBufferLock.unlock();


    int32_t out_buffer_size = MAX_SAMPLE_COUNT * 2 * sizeof(int16_t);
    LOGE("out_buffer_size = %d\n", out_buffer_size);
    uint8_t *out_buffer = (uint8_t *)malloc(out_buffer_size);

    bool readFinished = false;
    while (1)
    {
        if(stopFlag)
        {
            break;
        }
        if (decode_state != NEED_READ_FIRST)
        {
            int err1 = av_read_frame(formatCtx, packet);
            if (err1 < 0)
            {
                readFinished = true;
                packet->size = 0;
                LOGE("av_read_frame returns %d\n", err1);
            }
        }

        int err2 = avcodec_send_packet(codecCtx, packet);
        if (err2 == AVERROR(EAGAIN))
        {
            //Need to call avcodec_receive_frame() first, and then resend
            //this packet.
            decode_state = NEED_READ_FIRST;
//            LOGE("call send_packet() returns AVERROR(EAGAIN)\n");
        }
        else if (err2 == AVERROR_EOF)
        {
//            LOGE("call send_packet() returns AVERROR_EOF\n");
        }
        else if (err2 == AVERROR(EINVAL))
        {
//            LOGE("call send_packet() returns AVERROR(EINVAL)\n");
        }
        else if (err2 < 0)
        {
//            LOGE("call send_packet() returns %d\n", err2);
        }
        else
        {
//            LOGE("call send_packet() normal, returns %d\n", err2);
            decode_state = NORMAL;
        }

        while (1)
        {
            int err3 = avcodec_receive_frame(codecCtx, frame);
            if (err3 == AVERROR(EAGAIN))
            {
                //Can not read until send a new packet

//                LOGE("call avcodec_receive_frame() returns AVERROR(EAGAIN)\n");
                break;
            }
            else if (err3 == AVERROR_EOF)
            {
//                LOGE("call avcodec_receive_frame() returns AVERROR_EOF\n");
                break;
            }
            else if (err3 == AVERROR(EINVAL))
            {
//                LOGE("call avcodec_receive_frame() returns AVERROR(EINVAL)\n");
                break;
            }
            else if (err3 < 0)
            {
//                LOGE("call avcodec_receive_frame() returns %d\n", err3);
                break;
            }
            else
            {
//                LOGE("success receive a frame\n");

                int frameCount = swr_convert(convertCtx, &out_buffer, MAX_SAMPLE_COUNT, (const uint8_t **)frame->data, frame->nb_samples);

                if(frameCount > 0)
                {
                    usedBufferLock.lock();
                    PCMBufferNode *node = NULL;
                    if(usedBuffers.size() <= 0)
                    {
                        node = new PCMBufferNode();
                        node->buffer = (int16_t *)malloc(MAX_SAMPLE_COUNT * out_channels * sizeof(int16_t));
                    }
                    else{
                        node = usedBuffers.front();
                        usedBuffers.pop_front();
                    }
                    usedBufferLock.unlock();
                    memset(node->buffer, 0, MAX_SAMPLE_COUNT * out_channels * sizeof(int16_t));


//                fwrite(out_buffer, frameCount, out_channels * 2 * sizeof(uint8_t), out);
                    node->sampleCount = frameCount;
                    node->currentPosition = frame->pts;
                    memcpy(node->buffer, out_buffer, frameCount * out_channels * sizeof(int16_t));

                    bufferLock.lock();
                    if(buffers.size() >= BUFFER_QUEUE_SIZE)
                    {
//                        LOGE("buffer is full, waiting...");
                        notFullSignal.wait(bufferLock);
                    }
                    buffers.push_back(node);
                    bufferLock.unlock();
                    notEmptySignal.notify_all();
                }


                while (1)
                {

                    frameCount = swr_convert(convertCtx, &out_buffer, MAX_SAMPLE_COUNT, NULL, 0);
                    if (frameCount > 0)
                    {
                        usedBufferLock.lock();
                        PCMBufferNode *node = NULL;
                        if(usedBuffers.size() <= 0)
                        {
                            node = new PCMBufferNode();
                            node->buffer = (int16_t *)malloc(MAX_SAMPLE_COUNT * out_channels * sizeof(int16_t));
                        }
                        else{
                            node = usedBuffers.front();
                            usedBuffers.pop_front();
                        }
                        usedBufferLock.unlock();
                        node->sampleCount = frameCount;
                        node->currentPosition = frame->pts;
                        memset(node->buffer, 0, MAX_SAMPLE_COUNT * out_channels * sizeof(int16_t));
                        memcpy(node->buffer, out_buffer, frameCount * out_channels * sizeof(int16_t));

//                        fwrite(out_buffer, frameCount, out_channels * 2 * sizeof(uint8_t), out);

                        bufferLock.lock();
                        if(buffers.size() >= BUFFER_QUEUE_SIZE)
                        {
//                            LOGE("buffer is full, waiting...");
                            notFullSignal.wait(bufferLock);
                        }
                        buffers.push_back(node);
                        bufferLock.unlock();
                        notEmptySignal.notify_all();
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        //If decode_state is NEED_READ_FIRST, we need to call avcodec_receive_frame() first, and then resend this packet to avcodec_send_packet, so we can't unref it and
        // can't install new data by av_read_frame().
        if (decode_state != NEED_READ_FIRST)
        {
            av_packet_unref(packet);
        }

        if (readFinished)
        {
            break;
        }
    }

    av_frame_free(&frame);
    av_free_packet(packet);

}

void AudioFileDecoder2::getAudioData(int16_t *audioData, int *sampleCount) {
    unique_lock<mutex> bufferLock(bufferMu);
    if(buffers.size() == 0)
    {
//        LOGE("buffer is empty, waiting...");
        notEmptySignal.wait(bufferLock);
    }
    PCMBufferNode *node = buffers.front();
    buffers.pop_front();
    bufferLock.unlock();

    notFullSignal.notify_all();

    memcpy(audioData, node->buffer, node->sampleCount * 2 * sizeof(int16_t));
    *sampleCount = node->sampleCount;

    unique_lock<mutex> usedBufferLock(usedBufferMu);
    usedBuffers.push_back(node);
    usedBufferLock.unlock();

}

