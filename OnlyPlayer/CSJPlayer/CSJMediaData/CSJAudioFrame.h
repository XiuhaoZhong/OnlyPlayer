//
//  CSJAudioFrame.hpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/11/3.
//

#ifndef CSJAudioFrame_h
#define CSJAudioFrame_h

#include <stdio.h>
#include <stdlib.h>

/*
 * 表示音频帧数据;
 */
class CSJAudioFrame {
public:
    CSJAudioFrame();
    CSJAudioFrame(uint8_t *data, int nb_samples, int sampleRate, int channels, int timerInterval, int fmt);
    CSJAudioFrame(CSJAudioFrame&& audioFrame);
    CSJAudioFrame& operator=(CSJAudioFrame&& audioFrame);
    ~CSJAudioFrame();
    
    uint8_t* getAudioData();
    
private:
    uint8_t *m_pData;      // 音频数据;
    int     m_vNbSamples;  // 每个通道数据长度;
    int     m_vChannel;    // 通道数;
    int     m_vSampleRate; // 采样率;
    int     m_vFormat;     // 音频数据格式;
    float   m_vPosition;   // 时间位置(时间戳);
    int     m_vDataLen;    // 数据长度;
};

#endif /* CSJAudioFrame_hpp */
