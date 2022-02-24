//
//  CSJAudioFrame.hpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/11/3.
//

#ifndef CSJAudioFrame_h
#define CSJAudioFrame_h

#include <stdio.h>

/*
 * 表示音频帧数据;
 */
class CSJAudioFrame {
public:
    CSJAudioFrame();
    ~CSJAudioFrame();
    
    short* getAudioData();
    
private:
    short   *data;      // 音频数据;
    int     channel;    // 通道数;
    int     sampleRate; // 采样率;
    int     format;     // 音频数据格式;
    float   position;   // 时间位置(时间戳);
};

#endif /* CSJAudioFrame_hpp */
