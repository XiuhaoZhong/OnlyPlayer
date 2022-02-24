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
    
    // initialize the decoder and file to open;
    bool init(CSJDecoderType type, std::string *filePath);
    
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
    // decoder;
    CSJVideoDecoderBase             *m_pDecoder;
    
    // decode thread;
    std::thread                     m_vDecodeThread;
    
    // audio and video raw data;
    CSJRingBuffer<CSJAudioFrame>    *m_pAudioRingBuffer;
    CSJRingBuffer<CSJVideoFrame>    *m_pVideoRingBuffer;
    
    
};

#endif /* CSJMeidaSyncer_hpp */
