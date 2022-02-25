//
//  CSJFFmpegVideoDecoder.cpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/11/4.
//

#include "CSJFFmpegVideoDecoder.hpp"

#include <iostream>

// 默认播放音频的通道数;
static const int g_vPlayAudioChannels = 2;

CSJFFmpegVideoDecoder::CSJFFmpegVideoDecoder() {
    m_pFormatCtx = nullptr;
    m_pVideoCodecCtx = nullptr;
    m_pAudioCodecCtx = nullptr;
    m_pAudioConverter = nullptr;
    
    m_pVideoConverter = nullptr;
    m_vOriginFmt = (AVPixelFormat)-1;
    m_vVideoFmt = AV_PIX_FMT_RGBA;
    m_pVideoConverterBuffer = 0;
    m_vVideoWidth = 1280;
    m_vVideoHeight = 760;
    
    // 音频转换参数初始化;
    m_vAudioFmt = AV_SAMPLE_FMT_S16;
    m_vAudioChannelLayout = av_get_default_channel_layout(g_vPlayAudioChannels);
    m_vAudioConvertBuffer = nullptr;
    m_vAudioConvertBufferSize = 0;
    
    m_vStop = true;
    m_vPause = false;
}

CSJFFmpegVideoDecoder::~CSJFFmpegVideoDecoder() {
    releaseResource();
    
    m_vDecodeThread.join();
}

void CSJFFmpegVideoDecoder::start() {
    m_vStop = false;
    
    decodeFrame(0);
}

void CSJFFmpegVideoDecoder::pause() {
    m_vPause = true;
}

void CSJFFmpegVideoDecoder::resume() {
    m_vPause = false;
    
    m_vCondVar.notify_one();
}

void CSJFFmpegVideoDecoder::stop() {
    m_vStop = true;
    m_vDecodeThread.join();
}

int CSJFFmpegVideoDecoder::getVideoTime() {
    return 0;
}

CSJDecoderStatus CSJFFmpegVideoDecoder::decodeStatus() {
    return m_vPlayerStatus;
}

int CSJFFmpegVideoDecoder::openFile(string url) {
    if (url.length() <= 0) {
        // TODO: log 当前文件路径有误;
        
        return -1;
    }
    
    m_pFormatCtx = avformat_alloc_context();
    m_pFormatCtx->probesize = 50 * 1024;
    m_pFormatCtx->max_analyze_duration = 75000;
    
    int status = avformat_open_input(&m_pFormatCtx, url.c_str(), NULL, NULL);
    if (status != 0) {
        // TODO: log 打开文件失败;
        
        return -1;
    }
    
    // 打印文件详细信息;
    av_dump_format(m_pFormatCtx, -1, url.c_str(), 0);
    
    // 解码上下文是否分配成功;
    bool decodeCtxAllocRes = false;
    
    // 获取对应流的索引号;
    int videoStreamIdx = -1;
    int audioStreamIdx = -1;
    
    for (int i = 0; i < m_pFormatCtx->nb_streams; i++) {
        if (m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIdx = i;
        }
        
        if (m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIdx = i;
        }
    }
    
    if (videoStreamIdx == -1) {
        // TODO: log 视频流索引异常;
        decodeCtxAllocRes |= false;
    } else {
        // 获取视频流和音频流的解码器;
        m_pVideoCodecCtx = avcodec_alloc_context3(NULL);
        if (avcodec_parameters_to_context(m_pVideoCodecCtx, m_pFormatCtx->streams[videoStreamIdx]->codecpar) < 0) {
            // TODO: log 获取视频解码器失败;
            return -1;
        }
        if (videoStreamIdx != -1 && !m_pVideoCodecCtx->codec) {
            m_pVideoCodecCtx->codec = avcodec_find_decoder(m_pVideoCodecCtx->codec_id);
        }
        
        if (m_pVideoCodecCtx->pix_fmt != m_vVideoFmt) {
            
        }
        
        m_vVideoStreamIdx = videoStreamIdx;
        decodeCtxAllocRes |= true;
    }
    
    if (audioStreamIdx == -1) {
        // TODO: log 音频流索引异常;
        decodeCtxAllocRes |= false;
    } else {
        m_vAudioStreamIdx = audioStreamIdx;
        m_pAudioCodecCtx = avcodec_alloc_context3(NULL);
        if (avcodec_parameters_to_context(m_pAudioCodecCtx, m_pFormatCtx->streams[audioStreamIdx]->codecpar) < 0) {
            // TODO: log 获取音频解码器失败;
            return -1;
        }
        // 设置一下这两个属性，并不能让音频解码成相应的格式，至少aac和mp3不行;
//        m_pAudioCodecCtx->request_sample_fmt = AV_SAMPLE_FMT_S16;
//        m_pAudioCodecCtx->request_channel_layout = m_vAudioChannelLayout;
        if (!m_pAudioCodecCtx->codec) {
            m_pAudioCodecCtx->codec = avcodec_find_decoder(m_pAudioCodecCtx->codec_id);
        }
        
        // 根据音频解码器的属性创建音频转换器;
        this->m_vAudioSampleRate = this->m_pAudioCodecCtx->sample_rate;
        decodeCtxAllocRes |= true;
    }
    
    if (!decodeCtxAllocRes) {
        // TODO: 音频和视频解码Ctx都创建失败，返回
        return -1;
    }
    
    // 分配解码过程需要的packet和frame;
    // AVPacket 存储从文件读出的数据;
    // frame 存储从packet中解码得到的数据;
    m_pDecoderPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    m_pVideoFrame = av_frame_alloc();
    m_pAudioFrame = av_frame_alloc();
    
    return 0;
}

void CSJFFmpegVideoDecoder::closeFile() {
    releaseResource();
}

bool CSJFFmpegVideoDecoder::isEOF() {
    return true;
}

void CSJFFmpegVideoDecoder::decodeFrame(float duration) {
    m_vDecodeThread = std::thread(&CSJFFmpegVideoDecoder::decodeStart, this);
}

void CSJFFmpegVideoDecoder::decodeStart() {
    int readFrameCode = -1;
    
    while (1) {
        if (m_vStop) {
            break;
        }
        
        if (m_vPause) {
            std::cout << "decode debug: 解码已经暂停" << std::endl;
            std::unique_lock<std::mutex> lock(m_vMutex);
            m_vCondVar.wait(lock);
        }
        
        std::cout << "decode debug: 解码数据" << std::endl;
        readFrameCode = av_read_frame(m_pFormatCtx, m_pDecoderPacket);
        if (readFrameCode < 0) {
            // TODO: read frame error;
            return ;
        }
        
        if (m_pDecoderPacket->stream_index == m_vVideoStreamIdx) {
            // packet是视频流数据;
            decodeVideoPacket(m_pDecoderPacket);
        } else if (m_pDecoderPacket->stream_index == m_vAudioStreamIdx) {
            // packet是音频流数据;
            decodeAudioPacket(m_pDecoderPacket);
        }
    }
}

void CSJFFmpegVideoDecoder::decodeAudioPacket(AVPacket *audioPacket) {
    if (!audioPacket || audioPacket->stream_index < 0) {
        return ;
    }
    
    if (avcodec_is_open(m_pAudioCodecCtx) <= 0) {
        avcodec_open2(m_pAudioCodecCtx, m_pAudioCodecCtx->codec, NULL);
    }
    
    int res = 0;
    res = avcodec_send_packet(m_pAudioCodecCtx, audioPacket);
    if (res < 0) {
        // send packet error;
        return ;
    }
    
    // 解码的音频是MP3时，在avcodec_send_packet之前，mAudioCodecCtx的sample_rate为0，此处check一下，重新赋值;
    if (!this->m_vAudioSampleRate) {
        this->m_vAudioSampleRate = m_pAudioCodecCtx->sample_rate;
    }
    
    res = avcodec_receive_frame(m_pAudioCodecCtx, m_pAudioFrame);
    if (res < 0) {
        // receive frame error;
        return ;
    }
    
    void *audioData;
    if (!resampleIfNeeded()) {
        // 解码得到的数据是不支持的数据格式（非16位）
        bool convertRes = audioFmtConvert(m_pAudioFrame);
        if (!convertRes) {
            // TODO: 音频转换出错;
            return ;
        }
        audioData = m_vAudioConvertBuffer;
    } else {
        // 解码的数据是支持的格式
        audioData = m_pAudioFrame->data[0];
    }
        
    // TODO: 将解码得到的数据全部存储到音频数据的ringbuffer中;
}

void CSJFFmpegVideoDecoder::decodeVideoPacket(AVPacket *videoPacket) {
    if (!videoPacket || videoPacket->stream_index < 0) {
        return ;
    }
    
    if (!avcodec_is_open(m_pVideoCodecCtx)) {
        avcodec_open2(m_pVideoCodecCtx, m_pVideoCodecCtx->codec, NULL);
    }
    
    int res = 0;
    res = avcodec_send_packet(m_pVideoCodecCtx, videoPacket);
    if (res < 0) {
        // TODO: send video packet to videoCodec failed;
        return ;
    }
    
    res = avcodec_receive_frame(m_pVideoCodecCtx, m_pVideoFrame);
    if (res < 0) {
        // TODO: 解码视频帧失败;
        return ;
    }
    
    if (!m_pVideoFrame) {
        return ;
    }
    
    // swsContext is null and videoCodecContext's format is different with m_vVideoFmt
    // needs construct the swsContext;
    if (!m_pVideoConverter && m_pVideoCodecCtx->pix_fmt != m_vVideoFmt) {
        createVideoConverter(m_pVideoFrame);
    } else {
        // When the m_pVideoFrame's size is different from m_vVideoWidth and m_vVideoHeight;
        // needs recontruct the swsContext with m_pVideoFrame's size;
        // by the way, this hardly happend(almost not happen);
        if (m_pVideoFrame->width != m_vVideoWidth || m_pVideoFrame->height != m_vVideoHeight) {
            createVideoConverter(m_pVideoFrame);
        }
    }

    // m_pVideoConterter is not null, indicates the video format needs to be convert;
    if (m_pVideoConverter) {
        // m_pVideoConverterBuffer save the video data after convert;
        // m_pVideoConverterBuffer is malloc only once;
        if (!m_pVideoConverterBuffer) {
            m_pVideoConverterBuffer = malloc(sizeof(uint8_t) * m_vVideoWidth * m_vVideoHeight * 4);
        }

        uint8_t *dstBuffer[1] = {(uint8_t *)m_pVideoConverterBuffer};
        int dstLineStride[1] = {m_vVideoWidth * 4};
        int scaleRes = sws_scale(m_pVideoConverter, m_pVideoFrame->data, m_pVideoFrame->linesize, 0, m_pVideoFrame->height, dstBuffer, dstLineStride);
        if (scaleRes < 0) {
            // TODO: scale faile
            return ;
        }
        
        // video data convert successful, add to ring buffer;
       
        
    } else {
        // 不需要转换;
    }
}

void CSJFFmpegVideoDecoder::fillRawAudioData(uint8_t *data, int width, int height, float timerInterval, int format) {
    
}

void CSJFFmpegVideoDecoder::fillRawVideoData(uint8_t *data, int sampleRate, int channel, float timerInterval, int format) {
    
}

void CSJFFmpegVideoDecoder::createVideoConverter(AVFrame *videoFrame) {
    if (!videoFrame) {
        return ;
    }
    
    if (m_pVideoConverter) {
        free(m_pVideoConverter);
        m_pVideoConverter = nullptr;
    }
    
    m_vOriginWidth = videoFrame->width;
    m_vOriginHeight = videoFrame->height;
    // construct swsContext to convert video format;
    m_pVideoConverter = sws_getContext(m_vOriginWidth, m_vOriginHeight, (AVPixelFormat)videoFrame->format,
                                       m_vVideoWidth, m_vVideoHeight, m_vVideoFmt, 0, NULL, NULL, NULL);
}

bool CSJFFmpegVideoDecoder::videoConvertIfNeeded(AVFrame *videoFrame) {
    bool isSupport = false;
    
    if (!m_pVideoCodecCtx) {
        return isSupport;
    }
    
    if (videoFrame->format != m_vOriginFmt) {
        m_vOriginFmt = (AVPixelFormat)videoFrame->format;
        return false;
    } else {
        isSupport = true;
    }
    
    if (m_vOriginWidth != 0 || m_vOriginHeight != 0) {
        if (videoFrame->width != m_vOriginWidth || videoFrame->height != m_vOriginHeight) {
            return false;
        }
    } else {
        isSupport = false;
    }
    
    return isSupport;
}

bool CSJFFmpegVideoDecoder::resampleIfNeeded() {
    if (!m_pAudioCodecCtx) {
        return false;
    }
    
    if (m_pAudioCodecCtx->sample_fmt == this->m_vAudioFmt &&
        av_get_default_channel_layout(m_pAudioCodecCtx->channels) == this->m_vAudioChannelLayout) {
        return true;
    }
    
    return false;
}

void CSJFFmpegVideoDecoder::createAudioConverter(AVFrame *audioFrame) {
    if (!audioFrame) {
        return ;
    }
    
    this->m_pAudioConverter = swr_alloc();
    swr_alloc_set_opts(this->m_pAudioConverter,
                       this->m_vAudioChannelLayout,
                       this->m_vAudioFmt,
                       this->m_vAudioSampleRate,
                       av_get_default_channel_layout(audioFrame->channels),
                       (AVSampleFormat)audioFrame->format,
                       audioFrame->sample_rate, 0, NULL);
    if (swr_init(m_pAudioConverter) < 0) {
        // TODO: log swrContext init failed;
    }
    
    int isPlanar = av_sample_fmt_is_planar((AVSampleFormat)audioFrame->format);
    
    // 计算转换结果需要的buffer大小;
    int ratio = g_vPlayAudioChannels / m_pAudioFrame->channels;
    int bufferSize = av_samples_get_buffer_size(NULL, g_vPlayAudioChannels, audioFrame->nb_samples * ratio, (AVSampleFormat)audioFrame->format, 0);
    if (!m_vAudioConvertBuffer || m_vAudioConvertBufferSize < bufferSize) {
        m_vAudioConvertBufferSize = bufferSize * 2;
        m_vAudioConvertBuffer = realloc(m_vAudioConvertBuffer, m_vAudioConvertBufferSize);
    }
}

bool CSJFFmpegVideoDecoder::audioFmtConvert(AVFrame *audioFrame) {
    if (!audioFrame) {
        return false;
    }
    
    if (!m_pAudioConverter) {
        createAudioConverter(audioFrame);
    }
    
    int sampleConvertRate = g_vPlayAudioChannels / audioFrame->channels;
    uint8_t *output[2] = {(uint8_t *)m_vAudioConvertBuffer, NULL};
    int numFrames = swr_convert(m_pAudioConverter, output, audioFrame->nb_samples * sampleConvertRate, (const uint8_t **)audioFrame->data, audioFrame->nb_samples);
    
    int totalSize = 0;
    totalSize = swr_get_out_samples(m_pAudioConverter, totalSize);
    // TODO: 如果totalSize大于0，可能需要考虑继续转换音频数据;
    
//    if (numFrames < 0) {
//        // TODO: log 音频转换失败;
//        return false;
//    }
    
    return true;
}

void CSJFFmpegVideoDecoder::releaseResource() {
    if (m_pVideoFrame) {
        av_frame_free(&m_pVideoFrame);
        m_pVideoFrame = NULL;
    }
    
    if (m_pAudioFrame) {
        av_frame_free(&m_pAudioFrame);
        m_pAudioFrame = NULL;
    }
    
    if (m_pDecoderPacket) {
        av_packet_free(&m_pDecoderPacket);
        m_pDecoderPacket = NULL;
    }
    
    if (m_pVideoCodecCtx) {
        avcodec_free_context(&m_pVideoCodecCtx);
        m_pVideoCodecCtx = NULL;
    }
    
    if (m_pAudioCodecCtx) {
        avcodec_free_context(&m_pAudioCodecCtx);
        m_pAudioCodecCtx = NULL;
    }
    
    if (m_pFormatCtx) {
        avformat_close_input(&m_pFormatCtx);
        m_pFormatCtx = NULL;
    }
}
