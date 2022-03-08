//
//  CSJAudioFrame.cpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/11/3.
//

#include "CSJAudioFrame.h"

#include <string.h>

CSJAudioFrame::CSJAudioFrame() {
    
}

CSJAudioFrame::CSJAudioFrame(uint8_t *data, int nb_samples, int sampleRate, int channels, int timerInterval, int fmt)
                            : m_vSampleRate(sampleRate),
                              m_vNbSamples(nb_samples),
                              m_vChannel(channels),
                              m_vPosition(timerInterval),
                              m_vFormat(fmt) {
    m_vDataLen = sizeof(uint8_t) * nb_samples * channels;
    m_pData = new uint8_t[m_vDataLen];
    memcpy(m_pData, data, m_vDataLen);
}

CSJAudioFrame::CSJAudioFrame(CSJAudioFrame&& audioFrame) {
    m_pData = audioFrame.m_pData;
    audioFrame.m_pData = nullptr;
    
    m_vSampleRate = audioFrame.m_vSampleRate;
    m_vChannel = audioFrame.m_vChannel;
    m_vFormat = audioFrame.m_vFormat;
    m_vPosition = audioFrame.m_vPosition;
}

CSJAudioFrame& CSJAudioFrame::operator=(CSJAudioFrame &&audioFrame) {
    if (this != &audioFrame) {
        m_pData = audioFrame.m_pData;
        audioFrame.m_pData = nullptr;
        
        m_vSampleRate = audioFrame.m_vSampleRate;
        m_vChannel = audioFrame.m_vChannel;
        m_vFormat = audioFrame.m_vFormat;
        m_vPosition = audioFrame.m_vPosition;
    }
    
    return *this;
}

CSJAudioFrame::~CSJAudioFrame() {
    if (m_pData) {
        delete m_pData;
        m_pData = nullptr;
    }
}

uint8_t* CSJAudioFrame::getAudioData() {
    return m_pData;
}
