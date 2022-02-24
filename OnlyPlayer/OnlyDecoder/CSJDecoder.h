//
//  CSJDecoder.h
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/8/14.
//

#ifndef CSJDecoder_h
#define CSJDecoder_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#ifdef __cplusplus
extern "C" {
//#endif

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavutil/channel_layout.h"
#include "libavutil/samplefmt.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/common.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"


//#ifdef __cpluscplus
};
//#endif

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define LOGI(...) printf(" ");printf(__VA_ARGS__);printf("\t - <%s> \n", LOG_TAG);

#define OUT_PUT_CHANNELS 2

typedef struct AudioPacket {
    static const int AUDIO_PACKET_ACTION_PLAY = 0;
    static const int AUDIO_PACKET_ACTION_PAUSE = 100;
    static const int AUDIO_PACKET_ACTION_SEEK = 101;
    
    short *buffer;
    int   size;
    float position;
    int   action;
    
    float extra_param1;
    float extra_param2;
    
    AudioPacket() {
        buffer = NULL;
        size = 0;
        position = -1;
        action = 0;
        extra_param1 = 0;
        extra_param2 = 0;
    }
    
    ~AudioPacket() {
        if (buffer) {
            delete [] buffer;
            buffer = NULL;
        }
    }
} AudioPacket;

class CSJDecoder {
public:
    CSJDecoder();
    virtual ~CSJDecoder();
    
    // 获取采样率以及比特率;
    virtual int getMusicMeta(const char* fileString, int *metaData);
    // 初始化decoder，即打开定制的mp3文件;
    virtual void init(const char* fileString, int packetBufferSizeParam);
    virtual  AudioPacket *decodePacket();
    
    // 销毁decoder;
    virtual void destroy();
    
    void setSeekReq(bool seekReqParam) {
        seek_req = seekReqParam;
        if (seek_req) {
            seek_resp = false;
        }
    };
  
    bool hasSeekReq() {
        return seek_req;
    }
    
    bool hasSeekResp() {
        return seek_resp;
    }
    
    // 设置播放位置，单位:s，后边有3位小数，精确到ms;
    void setPosition(float seconds) {
        actualSeekPosition = -1;
        this->seek_seconds = seconds;
        this->seek_req = true;
        this->seek_resp = false;
    }
    
    float getActualSeekPosition() {
        float ret = actualSeekPosition;
        if (ret != -1) {
            actualSeekPosition = -1;
        }
        return ret;
    }
    
    virtual void seek_frame();
    
private:
    int init(const char* fileString);
    int readSamples(short* samples, int size);
    int readFrame();
    bool audioCodecIsSupported();
    
private:
    // 快进/快退命令时先设置的参数;
    bool seek_req;
    bool seek_resp;
    float seek_seconds;
    
    float actualSeekPosition;
    
    AVFormatContext *avFormatContext;
    AVCodecContext  *avCodecContext;
    
    int stream_index;
    float timeBase;
    
    AVFrame *pAudioFrame;
    AVPacket packet;
    
    char *filePath;
    
    bool seek_success_read_frame_success;
    int packetBufferSize;
    
    // 每次解码得到的audioBuffer以及相应的时间戳操作情况;
    short *audioBuffer;
    float position;
    int audioBufferCursor;
    int audioBufferSize;
    float duration;
    bool isNeedFirstFrameCorrectFlag;
    float firstFrameCorrectionInSec;
    
    SwrContext *swrContext;
    void *swrBuffer;
    int swrBufferSize;
};

#endif /* CSJDecoder_h */
