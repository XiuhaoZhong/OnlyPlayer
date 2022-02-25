//
//  CSJFFmpegVideoDecoder.hpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/11/4.
//

#ifndef CSJFFmpegVideoDecoder_hpp
#define CSJFFmpegVideoDecoder_hpp

#include "CSJVideoDecoderBase.h"

#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavfilter/avfilter.h"
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libavutil/channel_layout.h"
#include "libavutil/opt.h"

#ifdef __cplusplus
}
#endif

using std::thread;

class CSJFFmpegVideoDecoder : public CSJVideoDecoderBase {
public:
    CSJFFmpegVideoDecoder();
    ~CSJFFmpegVideoDecoder();
    
    int openFile(string url) override;
    void closeFile() override;
    bool isEOF() override;
    
    virtual void decodeFrame(float duration) override;
    
    virtual void start() override;
    virtual void pause() override;
    virtual void resume() override;
    virtual void stop() override;
    
    virtual CSJDecoderStatus decodeStatus() override;
    
    virtual int getVideoTime() override;
    
protected:
    void decodeStart();
    
    void decodeAudioPacket(AVPacket *audioPacket);
    void decodeVideoPacket(AVPacket *videoPacket);
    
    void fillRawAudioData(uint8_t *data, int width, int height, float timerInterval, int format);
    void fillRawVideoData(uint8_t *data, int sampleRate, int channel, float timerInterval, int format);
    
    bool videoConvertIfNeeded(AVFrame *videoFrame);
    void createVideoConverter(AVFrame *videoFrame);

    // 音频重采样;
    int reSampleAudioData();
    
    /* 初始化音频重采样的swrContext;
     * @param channel_layout    输入数据的通道布局;
     * @param inputFmt          输入数据的格式;
     * @param inputSampleRate   输入数据的采样率;
    **/
    void createAudioConverter(AVFrame *audioFrame);
    
    // 是否需要重采样;
    bool resampleIfNeeded();
    // 音频转换;
    bool audioFmtConvert(AVFrame *audioFrame);
    
    void releaseResource();
    
private:
    AVFormatContext *m_pFormatCtx;
    AVCodecContext  *m_pVideoCodecCtx;
    AVCodecContext  *m_pAudioCodecCtx;
    
    AVPacket        *m_pDecoderPacket;
    AVFrame         *m_pVideoFrame;
    AVFrame         *m_pAudioFrame;
    
    int             m_vAudioStreamIdx;          // 音频流的stream index;
    int             m_vVideoStreamIdx;          // 视频流的stream index;
    
    AVPixelFormat   m_vOriginFmt;               // 当前视频文件中视频格式;
    int             m_vOriginWidth;             // 视频文件中视频的宽度;
    int             m_vOriginHeight;            // 视频文件中视频的高度;
    
    AVPixelFormat   m_vVideoFmt;                // 播放的视频格式;
    int             m_vVideoWidth;              // 播放视频的宽;
    int             m_vVideoHeight;             // 播放视频的高;
    SwsContext      *m_pVideoConverter;         // 视频格式转换上下文;
    void            *m_pVideoConverterBuffer;   // 存储视频转换结果的缓冲区;
    int             m_vVideoConverterBufferSize;// 视频转换缓冲区大小;
    
    /*
     *  以下三个音频属性是固定的设置的音频播放属性;
     *
     *  后期可以添加接口配置修改;
     */
    AVSampleFormat  m_vAudioFmt;               // 播放音频的格式;
    int64_t         m_vAudioChannelLayout;     // 播放音频的通道布局;
    int             m_vAudioSampleRate;        // 播放音频的采样率;
    void*           m_vAudioConvertBuffer;     // 音频转换时的缓冲区;
    int             m_vAudioConvertBufferSize; // 音频转换时的大小;
    
    // 音频重采样上下文;
    SwrContext      *m_pAudioConverter;
    
    thread          m_vDecodeThread;
    std::mutex      m_vMutex;                   // 控制解码线程的锁;
    std::condition_variable     m_vCondVar;     // 条件变量;
    
    // 播放器状态;
    bool            m_vStop;                    // 是否停止;
    bool            m_vPause;                   // 是否暂停;
    
    CSJDecoderStatus m_vPlayerStatus;           // 播放器状态;
};

#endif /* CSJFFmpegVideoDecoder_hpp */
