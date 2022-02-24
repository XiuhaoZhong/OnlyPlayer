//
//  CSJDecoder.cpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/8/14.
//

#include "CSJDecoder.h"

#define LOG_TAG "CSJDecoder"

CSJDecoder::CSJDecoder() {
    this->seek_seconds = 0.0f;
    this->seek_req = false;
    this->seek_resp = false;
    filePath = NULL;
}

CSJDecoder::~CSJDecoder() {
    if (filePath != NULL) {
        delete [] filePath;
        filePath = NULL;
    }
}

void CSJDecoder::init(const char *fileString, int packetBufferSizeParam) {
    init(fileString);
    packetBufferSize = packetBufferSizeParam;
}

int CSJDecoder::init(const char *fileString) {
    LOGI("enter CSJDecoder::init");
    audioBuffer = NULL;
    position = -1;
    audioBufferCursor = 0;
    audioBufferSize = 0;
    swrContext = NULL;
    swrBuffer = NULL;
    swrBufferSize = NULL;
    seek_success_read_frame_success = true;
    isNeedFirstFrameCorrectFlag = true;
    firstFrameCorrectionInSec = 0.0f;
    
    avFormatContext = avformat_alloc_context();
    
    // 打开输入文件;
    LOGI("CSJDecoder open file: %s", fileString);
    if (filePath == NULL) {
        size_t length = strlen(fileString);
        filePath = new char[length + 1];
        memset(filePath,  0, length + 1);
        memcpy(filePath, fileString, length + 1);
    }
    
    int result = avformat_open_input(&avFormatContext, filePath, NULL, NULL);
    if (result != 0) {
        LOGI("CSJDecoder can't open file %s, result is %d", filePath, result);
        return -1;
    } else {
        LOGI("CSJDecoder open file %s", filePath);
    }
    
    avFormatContext->max_analyze_duration = 50000;
    // 检查文件中的流的信息;
    result = avformat_find_stream_info(avFormatContext, NULL);
    if (result < 0) {
        LOGI("CSJDecoder can't find_stream_info, result is %d", result);
    } else {
        LOGI("CSJDecoder find_stream_info");
    }
    
    stream_index = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_AUDIO,  -1, -1, NULL, 0);
    LOGI("CSJDecoder stream_index is %d", stream_index);
    
    // 如果没有音频;
    if (stream_index == -1) {
        LOGI("CSJDecoder can't find audio stream");
        return -1;
    }
    
    // 音频流的处理;
    AVStream *audioStream = avFormatContext->streams[stream_index];
    if (audioStream->time_base.den && audioStream->time_base.num) {
        timeBase = av_q2d(audioStream->time_base);
    } else if (audioStream->codec->time_base.den && audioStream->codec->time_base.num) {
        // TODO: 商榷中，audioStream->codec 在当前版本已经提示废弃，使用codecpar替代，但是codecpar中还不知道time_base对应的值; 2021/08/14
        timeBase = av_q2d(audioStream->codec->time_base);
    }
    
    // 根据解码器上下文找到解码器;
    avCodecContext = audioStream->codec;
    LOGI("avCodecContext->codec_id is %d AV_CODEC_ID_AAC is %d", avCodecContext->codec_id, AV_CODEC_ID_AAC);
    AVCodec *avCodec = avcodec_find_decoder(avCodecContext->codec_id);
    if (!avCodec) {
        LOGI("UnSupported codec");
        return -1;
    }
    
    // 打开解码器;
    result = avcodec_open2(avCodecContext, avCodec, NULL);
    if (result < 0) {
        LOGI("CSJDecoder failed open avCodec, id: %d, result: %d", avCodecContext->codec_id, result);
    } else {
        LOGI("CSJDecoder open avCodec %d", avCodecContext->codec_id);
    }
    
    // 判断是否需要resample（重新采样）;
    if (!audioCodecIsSupported()) {
        LOGI("because of audio Codec isn't supported so we will init swaresample to resample audio");
        
        // 初始化resampler;
        swrContext = swr_alloc_set_opts(NULL,
                                        av_get_default_channel_layout(OUT_PUT_CHANNELS),
                                        AV_SAMPLE_FMT_S16,
                                        avCodecContext->sample_rate,
                                        av_get_default_channel_layout(avCodecContext->channels),
                                        avCodecContext->sample_fmt,
                                        avCodecContext->sample_rate,
                                        0,
                                        NULL);
        
        if (!swrContext || swr_init(swrContext)) {
            if (swrContext) {
                swr_free(&swrContext);
            }
            avcodec_close(avCodecContext);
            LOGI("init resample failed...");
            return -1;
        }
    }
    
    LOGI("chnnels is %d sampleRate is %d", avCodecContext->channels, avCodecContext->sample_rate);
    pAudioFrame = av_frame_alloc();
    LOGI("leave CSJDecoder init");
    
    return 0;
}

int CSJDecoder::getMusicMeta(const char *fileString, int *metaData) {
    int ret = init(fileString);
    if (ret != 0) {
        LOGI("CSJDecoder init failed, ret: %d", ret);
        return ret;
    }
    int sampleRate = avCodecContext->sample_rate;
    LOGI("sampleRate is %d", sampleRate);
    int bitRate = avCodecContext->bit_rate;
    LOGI("bitRate is %d", bitRate);
    destroy();
    metaData[0] = sampleRate;
    metaData[1] = bitRate;
    
    return 0;
}

bool CSJDecoder::audioCodecIsSupported() {
    if (avCodecContext->sample_fmt == AV_SAMPLE_FMT_S16) {
        return true;
    }
    
    return false;
}

AudioPacket* CSJDecoder::decodePacket() {
    LOGI("CSJDecoder: decoderPacket packetBufferSize is %d", packetBufferSize);
    
    short *samples = new short[packetBufferSize];
    int stereoSampleSize = readSamples(samples, packetBufferSize);
    AudioPacket *samplePacket = new AudioPacket();
    if (stereoSampleSize > 0) {
        // 构造一个Packet;
        samplePacket->buffer = samples;
        samplePacket->size = stereoSampleSize;
        // 由于每一个packet的大小不一致，可能是200ms，position可能不准确;
        samplePacket->position = position;
    } else {
        samplePacket->size = -1;
    }
    
    return samplePacket;
}

int CSJDecoder::readSamples(short *samples, int size) {
    if (seek_req) {
        audioBufferCursor = audioBufferSize;
    }
    
    int sampleSize = size;
    while (size > 0) {
        if (audioBufferCursor < audioBufferSize) {
            int audioBufferDataSize = audioBufferSize - audioBufferCursor;
            int copySize = MIN(size, audioBufferDataSize);
            memcpy(samples + (sampleSize - size), audioBuffer + audioBufferCursor, copySize * 2);
            size -= copySize;
            audioBufferCursor += copySize;
            LOGI("rest size: %d", size);
        } else {
            int ret = readFrame();
            if (ret < 0) {
                break;
            }
        }
    }
    
    int fillSize = sampleSize - size;
    if (fillSize == 0) {
        return -1;
    }
    
    return fillSize;
}

void CSJDecoder::seek_frame() {
    LOGI("\n CSJDecoder seek frame firstFrameCorrectionInSec is %.6f, seek_seconds=%f, position=%f \n", firstFrameCorrectionInSec, seek_seconds, position);
    
    float targetPosition = seek_seconds;
    float currentPosition = position;
    float frameDuration = duration;
    
    if (targetPosition < currentPosition) {
        this->destroy();
        this->init(filePath);
        // GT测试样本差距25ms，不会累加（啥意思？2021/08/14）
        currentPosition = 0.0;
    }
    
    int readFrameCode = -1;
    while (true) {
        av_init_packet(&packet);
        readFrameCode = av_read_frame(avFormatContext, &packet);
        if (readFrameCode > 0) {
            currentPosition += frameDuration;
            if (currentPosition >= targetPosition) {
                break;
            }
        }
        LOGI("currentPosition is %.3f", currentPosition);
        av_free_packet(&packet);
    }
    seek_resp = true;
    seek_req = false;
    seek_success_read_frame_success = false;
}

int CSJDecoder::readFrame() {
    LOGI("enter CSJDecoder::readFrame");
    
    if (seek_req) {
        this->seek_frame();
    }
    
    int ret = 1;
    av_init_packet(&packet);
    int gotFrame = 0;
    int readFrameCode = -1;
    
    while (true) {
        // 从文件或者提供的流里面读取下一个packet数据;
        readFrameCode = av_read_frame(avFormatContext, &packet);
        if (readFrameCode >= 0) {
            if (packet.stream_index == stream_index) {
                // TODO: avcodec_decode_audio4 当前版本标记废弃，解码的长度如何获取，gotFrame又如何获取; （2021/08/14）
                // 对读出的packet进行解码;
                int len = avcodec_decode_audio4(avCodecContext, pAudioFrame, &gotFrame, &packet);
                
//                ret = avcodec_send_packet(avCodecContext, &packet);
//                if (ret != 0) {
//                    LOGI("decode audioi error, skip packet");
//                }
//                ret = avcodec_receive_frame(avCodecContext, pAudioFrame);
                
                if (len < 0) {
                    LOGI("decode audio error, skip packet!");
                }
                
                if (gotFrame) {
                    int numChannels = OUT_PUT_CHANNELS;
                    int numFrames = 0;
                    void *audioData;
                    if (swrContext) {
                        // 需要重采样;
                        const int ratio = 2;
                        const int bufSize = av_samples_get_buffer_size(NULL,
                                                                       numChannels,
                                                                       pAudioFrame->nb_samples * ratio,
                                                                       AV_SAMPLE_FMT_S16,
                                                                       1);
                        
                        if (!swrBuffer || swrBufferSize < bufSize) {
                            swrBufferSize = bufSize;
                            swrBuffer = realloc(swrBuffer, swrBufferSize);
                        }
                        
                        // TODO: 源代码中使用的类型的byte，但是c++中好像没有byte这个类型，后面使用时的类型是uint8_t，此处先用uint8_t;
                        uint8_t *outbuf[2] = {(uint8_t *)swrBuffer, NULL};
                        numFrames = swr_convert(swrContext,
                                                outbuf,
                                                pAudioFrame->nb_samples * ratio,
                                                (const uint8_t **)pAudioFrame->data,
                                                pAudioFrame->nb_samples);
                        
                        if (numFrames < 0) {
                            LOGI("CSJDecoder failed resample audio!");
                            ret = -1;
                            break;
                        }
                        audioData = swrBuffer;
                    } else {
                        // 不需要重采样;
                        if (avCodecContext->sample_fmt != AV_SAMPLE_FMT_S16) {
                            LOGI("bucheck, audio format is invalid");
                            ret = -1;
                            break;
                        }
                        audioData = pAudioFrame->data[0];
                        numFrames = pAudioFrame->nb_samples;
                    }
                    
                    if (isNeedFirstFrameCorrectFlag && position >= 0) {
                        float expectedPosition = position + duration;
                        // TODO: av_frame_get_best_effort_timestamp 方法当前版本标记废弃，但还没有找到替代的方法; 2021/08/14
                        float actualPosition = av_frame_get_best_effort_timestamp(pAudioFrame) * timeBase;
                        firstFrameCorrectionInSec = actualPosition - expectedPosition;
                        isNeedFirstFrameCorrectFlag = false;
                    }
                    
                    // TODO: av_frame_get_pkt_duration 方法当前版本标记废弃，但还没有找到替代的方法; 2021/08/14
                    duration = av_frame_get_pkt_duration(pAudioFrame) * timeBase;
                    // TODO: av_frame_get_best_effort_timestamp 方法当前版本标记废弃，但还没有找到替代的方法; 2021/08/14
                    position = av_frame_get_best_effort_timestamp(pAudioFrame) * timeBase - firstFrameCorrectionInSec;
                    if (!seek_success_read_frame_success) {
                        LOGI("position si %.6f", position);
                        actualSeekPosition = position;
                        seek_success_read_frame_success = true;
                    }
                    
                    audioBufferSize = numFrames * numChannels;
                    audioBuffer = (short *)audioData;
                    audioBufferCursor = 0;
                    break;
                }
            }
        } else {
            ret = -1;
            break;
        }
    }
    // TODO: av_free_packet 方法当前版本标记废弃，但还没有找到替代的方法; 2021/08/14
    av_free_packet(&packet);
    
    LOGI("leave CSJDecoder::readFrame");
    return ret;
}

void CSJDecoder::destroy() {
    LOGI("enter CSJDecoder::destroy");
    if (swrBuffer != NULL) {
        free(swrBuffer);
        swrBuffer = NULL;
        swrBufferSize = 0;
    }
    
    if (swrContext) {
        swr_free(&swrContext);
        swrContext = NULL;
    }
    
    if (pAudioFrame) {
        av_free(pAudioFrame);
        pAudioFrame = NULL;
    }
    
    if (avCodecContext) {
        avcodec_close(avCodecContext);
        avCodecContext = NULL;
    }
    
    if (avFormatContext) {
        avformat_close_input(&avFormatContext);
        avFormatContext = NULL;
    }
    
    LOGI("leave CSJDecoder::destroy");
}
