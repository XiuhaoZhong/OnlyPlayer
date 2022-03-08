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
#include "CSJDecoderDataDelegate.hpp"

typedef enum {
    CSJDecoderType_NONE = -1,
    CSJDecoderType_FFMpeg
} CSJDecoderType;

class CSJVideoDecoderBase;
class CSJAudioFrame;
class CSJVideoFrame;



class CSJMediaSynchronizer : public CSJDecoderDataDelegate {
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
    
    std::unique_ptr<CSJAudioFrame> getNextAudioData();
    std::unique_ptr<CSJVideoFrame> getNextVideoData();
    
    // set data delegate for decoder;
    void setDecoderDataDelegate(std::shared_ptr<CSJDecoderDataDelegate> dataDelegate);
    
    // override from CSJDecoderDataDelegate;
    // put the raw data after codec into the ring buffer;
    virtual void fillAudioData(std::unique_ptr<CSJAudioFrame> audioData) override;
    virtual void fillVideoData(std::unique_ptr<CSJVideoFrame> videoData) override;
    
protected:
    std::shared_ptr<CSJVideoDecoderBase> createDecoderByType(CSJDecoderType type);
    
private:
    // decoder;
    std::shared_ptr<CSJVideoDecoderBase> m_pDecoder;
    
    // decode thread;
    std::thread                          m_vDecodeThread;
    
    // audio and video raw data;
    CSJRingBuffer<CSJAudioFrame>         *m_pAudioRingBuffer;
    CSJRingBuffer<CSJVideoFrame>         *m_pVideoRingBuffer;
    std::mutex                           m_vDataMutex;
};

#endif /* CSJMeidaSyncer_hpp */
