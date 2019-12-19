//
// Created by zu on 2019-12-11.
//

#include "AudioFileDecoder.h"
#include <android/log.h>

#define MODULE_NAME  "AudioFileDecoder"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, MODULE_NAME, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, MODULE_NAME, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, MODULE_NAME, __VA_ARGS__)

void thread_callback(void *context)
{
    AudioFileDecoder *decoder = (AudioFileDecoder *)context;
    decoder->decode();
}

AudioFileDecoder::AudioFileDecoder() {
    avcodec_register_all();
    currentFile = (char *)malloc(FILE_NAME_LEN * sizeof(char));
    memset(currentFile, 0, FILE_NAME_LEN * sizeof(char));
}

AudioFileDecoder::~AudioFileDecoder() {
    stopDecode();
    PCMBufferNode *toDeleteNode = NULL;
    while(buffers.size() > 0)
    {
        toDeleteNode = buffers.front();
        buffers.pop_front();
        delete(toDeleteNode);
    }

    while(usedBuffers.size() > 0)
    {
        toDeleteNode = usedBuffers.front();
        usedBuffers.pop_front();
        delete(toDeleteNode);
    }

    if(currentFile != NULL)
    {
        free(currentFile);
    }

}

bool AudioFileDecoder::openFile(const char *filePath) {
    if (filePath == NULL) {
        LOGE("file path is NULL");
        return false;
    }
    bool result = true;
    strcpy(currentFile, filePath);
    resetComponent();
    result = initComponent();

    if(result)
    {
        LOGD("initComponent succeed");
        startDecode();
    }else{
        LOGE("initComponent failed");
        resetComponent();
    }

    return result;
}

void AudioFileDecoder::closeInput() {
    stopDecode();
    resetComponent();

}

void AudioFileDecoder::seekTo(int64_t position) {
    if(position > duration)
    {
        LOGE("seek to position %ld is larger than duration %ld", position, duration);
        return;
    }
    seekTarget = position;
    seekReq = true;
}

bool AudioFileDecoder::initComponent() {
    LOGD("to open file %s", currentFile);

    frame = av_frame_alloc();
    formatContext = avformat_alloc_context();
    if (formatContext == NULL) {
        LOGE("Alloc format context failed");
        return false;
    }

    if (avformat_open_input(&formatContext, currentFile, NULL, NULL) < 0) {
        LOGE("Open file failed");
        return false;
    }

    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOGE("Find stream info failed");
        return false;
    }

    av_format_inject_global_side_data(formatContext);

    audioIndex = -1;

    for (int i = 0; i < formatContext->nb_streams; i++) {
        AVStream *st = formatContext->streams[i];
        AVMediaType type = st->codecpar->codec_type;
        if (type == AVMEDIA_TYPE_AUDIO) {
            audioIndex = i;
        }
        if (st->disposition & AV_DISPOSITION_ATTACHED_PIC) {
            LOGD("Found attached pic");
            AVPacket p = st->attached_pic;
            picBufferLen = p.size;
            picBuffer = (uint8_t *) malloc(picBufferLen * sizeof(int8_t));
            memcpy(picBuffer, p.data, picBufferLen);
        }
    }

    if (audioIndex == -1) {
        LOGE("Can not find audio stream");
        if (picBuffer != NULL) {
            free(picBuffer);
            picBufferLen = 0;
        }

        return false;
    }

    LOGD("audio index = %d", audioIndex);

    audioStream = formatContext->streams[audioIndex];

    duration = audioStream->duration;
    LOGD("Audio duration = %ld", duration);

    decoder = avcodec_find_decoder(audioStream->codecpar->codec_id);
    if (decoder == NULL) {
        LOGE("Can not find decoder");
        return false;
    }

    codecContext = avcodec_alloc_context3(decoder);
    if (codecContext == NULL) {
        LOGE("Can not allocate codec context");
        return false;
    }
    if (avcodec_parameters_to_context(codecContext, audioStream->codecpar) < 0) {
        LOGE("Copy params to codec context failed");
        return false;
    }

    if (avcodec_open2(codecContext, decoder, NULL) != 0) {
        LOGE("Failed to open codec");
        return false;
    }

    in_sample_rate = audioStream->codecpar->sample_rate;
    in_channel_layout = audioStream->codecpar->channel_layout;
    in_channels = audioStream->codecpar->channels;
    in_sample_fmt = codecContext->sample_fmt;

    LOGD("input info: \n    sample rate = %d\n    channel layout = %ld\n    channels = %d\n    sample fmt = %d",
         in_sample_rate, in_channel_layout, in_channels, in_sample_fmt);


    swrContext = swr_alloc_set_opts(swrContext, out_channel_layout, out_sample_fmt, out_sample_rate,
                                    in_channel_layout, in_sample_fmt, in_sample_rate, 0, NULL);
    if (swrContext == NULL) {
        LOGE("Failed to allocate swr context");
        return false;
    }

    swr_init(swrContext);

    packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(packet);

}

void AudioFileDecoder::resetComponent() {
    if (frame != NULL) {
        av_frame_free(&frame);
        frame = NULL;
    }
    if (packet != NULL) {
        av_packet_free(&packet);
        packet = NULL;
    }
    if (formatContext != NULL) {
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        formatContext = NULL;
    }
    if (codecContext != NULL) {
        avcodec_free_context(&codecContext);
        codecContext = NULL;
    }

    if (swrContext != NULL) {
        swr_free(&swrContext);
        swrContext = NULL;
    }

    decoder = NULL;

    duration = 0;
    currentPosition = 0;
    if (picBuffer != NULL) {
        free(picBuffer);
    }

    if (decodeThread != NULL) {
        delete (decodeThread);
        decodeThread = NULL;
    }


}

void AudioFileDecoder::startDecode() {
    if (codecContext == NULL || decoder == NULL || swrContext == NULL || packet == NULL ||
        frame == NULL) {
        LOGE("Component not init complete, can not start decode");
        return;
    }
    stopDecodeFlag = false;
    decodeThread = new thread(thread_callback, this);
}

void AudioFileDecoder::stopDecode() {

    if (decodeThread != NULL && decodeState != DecodeState::FINISHED) {
        stopDecodeFlag = true;
        decodeThread->join();//join the thread and wait it to finished by itself.
    }

    free(decodeThread);
    decodeThread = NULL;
}

long AudioFileDecoder::getCurrentPosition() {
    return currentPosition;
}

long AudioFileDecoder::getDuration() {
    return duration;
}

void AudioFileDecoder::getAudioData(int16_t *audio_data, int *sampleCount) {
    if(usingNode != NULL)
    {
        putNodeToUsedDeque(usingNode);
    }
    usingNode = getDataNode();
    audio_data = usingNode->buffer;
    *sampleCount = usingNode->sampleCount;
    currentPosition = usingNode->currentPosition;

}

void AudioFileDecoder::decode() {

    int err = 0;
    while (1) {

        if(stopDecodeFlag)
        {
            break;
        }
        if(seekReq)
        {
            seekReq = false;
            discardDecodedBuffers();
            av_seek_frame(formatContext, audioIndex, seekTarget, 0);
        }

        if (decodeState != NEED_READ_FIRST) {
            if (av_read_frame(formatContext, packet) < 0) {
                //Can not read more data from file

                break;
            }
        }

        if (packet->pts > audioStream->start_time) {
            err = avcodec_send_packet(codecContext, packet);
            if (err == AVERROR(EAGAIN)) {
                /*
                 * codec buffer is full, need to call avcodec_receive_frame() first and then send this packet
                 * again.
                 * */
                decodeState = NEED_READ_FIRST;
            } else if (err == 0) {
                //Normal
                decodeState = NORMAL;
            } else if (err == AVERROR_EOF) {
                //End of file
            } else if (err == AVERROR(EINVAL)) {

            } else {
                LOGD("call avcodec_send_packet() returns %d\n", err);
            }

            while (1) {
                err = avcodec_receive_frame(codecContext, frame);
                if (err == AVERROR(EAGAIN)) {
                    //Can not read until send a new packet
                    break;
                } else if (err == AVERROR_EOF) {
                    //The codec is flushed, no more frame will be output
                    break;
                } else if (err == AVERROR(EINVAL)) {
                    break;
                } else if (err == 0) {
                    //success, we should read data from swresaple
                    PCMBufferNode *node = getFreeNode();
                    uint8_t * tempBuffer = (uint8_t *) (node->buffer);
                    node->sampleCount = swr_convert(swrContext, &(tempBuffer),
                                                 MAX_SAMPLE_COUNT, (const uint8_t **) frame->data,
                                                 frame->nb_samples);
                    if(node->sampleCount < 0)
                    {
                        putNodeToUsedDeque(node);
                    }else{
                        putNodeToDeque(node);
                    }
                    //if the input frame count is greater than out buffer size, we need to read for times.
                    while(1)
                    {
                        node = getFreeNode();
                        tempBuffer = (uint8_t *)(node->buffer);
                        node->sampleCount = swr_convert(swrContext, &(tempBuffer), MAX_SAMPLE_COUNT, (const uint8_t **)frame->data, frame->nb_samples);
                        if(node->sampleCount < 0)
                        {
                            putNodeToUsedDeque(node);
                            break;
                        }else{
                            putNodeToDeque(node);
                        }
                    }
                }
            }
        }

        if(decodeState != NEED_READ_FIRST)
        {
            av_packet_unref(packet);
        }

    }
    decodeState = FINISHED;
}

/*
* At first, we should check the size of usedBuffers. if
* usedBuffers.size < USED_BUFFER_QUEUE_SIZE, we should allocate
* a new PCMBufferNode, or we can pull a used PCMBufferNode from
* usedBuffers.
* */
PCMBufferNode *AudioFileDecoder::getFreeNode() {

    PCMBufferNode *node = NULL;
    unique_lock<mutex> locker(usedBufferMu);
    if (usedBuffers.size() < USED_BUFFER_QUEUE_SIZE) {
        node = new PCMBufferNode();
        node->buffer = (int16_t *) malloc(MAX_SAMPLE_COUNT * 2 * sizeof(int16_t));
    } else {
        node = usedBuffers.front();
        usedBuffers.pop_front();
    }
    locker.unlock();
    return node;
}

PCMBufferNode* AudioFileDecoder::getDataNode() {
    PCMBufferNode *node = NULL;
    unique_lock<mutex> locker(bufferMu);
    while(buffers.size() == 0)
    {
        notEmptySignal.wait(locker);
    }
    node = buffers.front();
    buffers.pop_front();
    notFullSignal.notify_all();
    locker.unlock();
    return node;
}

void AudioFileDecoder::putNodeToDeque(PCMBufferNode *node) {
    unique_lock<mutex> locker(bufferMu);
    while(buffers.size() >= BUFFER_QUEUE_SIZE)
    {
        notFullSignal.wait(locker);
    }
    buffers.push_back(node);
    notEmptySignal.notify_all();
    locker.unlock();
}

void AudioFileDecoder::putNodeToUsedDeque(PCMBufferNode *node) {
    unique_lock<mutex> locker(usedBufferMu);
    usedBuffers.push_back(node);
    locker.unlock();
}

void AudioFileDecoder::discardDecodedBuffers() {
    unique_lock<mutex> locker(bufferMu);

    list<PCMBufferNode *>::iterator it = buffers.begin();
    while(it != buffers.end())
    {
        usedBuffers.push_back(*it);
        it++;
    }
    buffers.clear();
    locker.unlock();
}

