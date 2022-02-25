//
//  CSJDecoderDataDelegate.hpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2022/2/24.
//

#ifndef CSJDecoderDataDelegate_hpp
#define CSJDecoderDataDelegate_hpp

#include <stdio.h>
#include <memory>

#include "CSJAudioFrame.h"
#include "CSJVideoFrame.h"

// This is the delegate that get raw audio/video data from decoder;
class CSJDecoderDataDelegate {
public:
    CSJDecoderDataDelegate() {};
    virtual ~CSJDecoderDataDelegate() {};
    
    virtual void fillAudioData(std::unique_ptr<CSJAudioFrame> audioData) = 0;
    virtual void fillVideoData(std::unique_ptr<CSJVideoFrame> videoData) = 0;
};

#endif /* CSJDecoderDataDelegate_hpp */
