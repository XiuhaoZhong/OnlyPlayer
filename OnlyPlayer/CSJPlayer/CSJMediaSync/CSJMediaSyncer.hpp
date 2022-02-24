//
//  CSJMeidaSyncer.hpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2022/2/23.
//

#ifndef CSJMeidaSyncer_hpp
#define CSJMeidaSyncer_hpp

#include <stdio.h>

#include <thread>

#include "CSJRingBuffer.h"

typedef enum {
    CSJDecoderType_NONE = -1,
    CSJDecoderType_FFMpeg
} CSJDecoderType;

class CSJVideoDecoderBase;
class CSJAudioFrame;
class CSJVideoFrame;

class CSJMediaSynchronizer {
public:
    CSJMediaSynchronizer();
    ~CSJMediaSynchronizer();
    
    // decoder operations;
    void startDecode();
    void pauseDecode();
    void resumeDecode();
    void stopDecode();
    
    void pushVideoRawData(std::unique_ptr<CSJAudioFrame> audioData);
    void pushAudioRawData(std::unique_ptr<CSJVideoFrame> videoData);
    
    std::unique_ptr<CSJAudioFrame> getNextAudioData();
    std::unique_ptr<CSJVideoFrame> getNextVideoData();
    
protected:
    CSJVideoDecoderBase* createDecoderByType(CSJDecoderType type);
    
private:
    CSJVideoDecoderBase *decoder;
    
    CSJRingBuffer<CSJAudioFrame> *m_pAudioRingBuffer;
    CSJRingBuffer<CSJVideoFrame> *m_pVideoRingBuffer;
    
    std::thread decodeThread;
};

#endif /* CSJMeidaSyncer_hpp */