//
//  CSJMeidaSyncer.hpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2022/2/23.
//

#ifndef CSJMeidaSyncer_hpp
#define CSJMeidaSyncer_hpp

#include <stdio.h>

#include "CSJRingBuffer.h"

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
private:
    CSJVideoDecoderBase *decoder;
    
    CSJRingBuffer<std::unique_ptr<CSJAudioFrame>> *m_pAudioRingBuffer;
    CSJRingBuffer<std::unique_ptr<CSJVideoFrame>> *m_pVideoRingBuffer;
};

#endif /* CSJMeidaSyncer_hpp */
