//
//  CSJDecoderController.hpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/8/15.
//

#ifndef CSJDecoderController_hpp
#define CSJDecoderController_hpp

#include <stdio.h>
#include <unistd.h>

#define CHANNEL_PER_FRAME   2   // 一个frame里面channel数量;
#define BITS_PER_CHANNEL    16  // 采样位数;
#define BITS_PER_BYTE       8   // 一个字节8位;

// decode data to queue and queue size;
#define QUEUE_SIZE_MAX_THREASHOLD   25
#define QUEUE_SIZE_MIN_THREASHOLD   20


class CSJDecoder;
class CSJDecoderController {
  
public:
    CSJDecoderController();
    ~CSJDecoderController();
    
    // 初始化两个decoder，并且根据采样率计算伴奏和原唱的bufferSize;
    int init(const char *accompanyPath, const char *pcmFilePath);
    
    // 解码操作;
    void Decode();
    
    // 销毁Controller;
    void Destroy();
    
protected:
    FILE *pcmFile;
    
    // 伴奏解码器;
    CSJDecoder *csjDecoder;
    
    // 伴奏和原唱的采样率与解码伴奏和原唱的每个packet的大小;
    int accompanySampleRate;
    int accompanyPacketBufferSize;
    
    int bufferCnt;
};

#endif /* CSJDecoderController_hpp */
