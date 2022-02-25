//
//  CSJVideoFrame.cpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/11/3.
//

#include "CSJVideoFrame.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

CSJVideoFrame::CSJVideoFrame() {
    m_pData = nullptr;
    m_vWidth = 0;
    m_vHeight = 0;
    m_vPosition = 0.0;
    m_vForamt = -1;
}

CSJVideoFrame::CSJVideoFrame(uint8_t* data, int dataLen, int width, int height, float position, int format)
                            : m_vWidth(width),
                              m_vHeight(height),
                              m_vPosition(position),
                              m_vForamt(format) {
    m_pData = new uint8_t[dataLen];
    memcpy(m_pData, data, dataLen);
}

CSJVideoFrame::~CSJVideoFrame() {
    if (m_pData) {
        delete []m_pData;
        m_pData = nullptr;
    }
}

CSJVideoFrame::CSJVideoFrame(CSJVideoFrame&& videoFrame) {
    m_pData = videoFrame.m_pData;
    videoFrame.m_pData = nullptr;
    
    m_vWidth = videoFrame.m_vWidth;
    m_vHeight = videoFrame.m_vHeight;
    m_vPosition = videoFrame.m_vPosition;
    m_vForamt = videoFrame.m_vForamt;
}

CSJVideoFrame& CSJVideoFrame::operator=(CSJVideoFrame&& videoFrame) {
    if (this != &videoFrame) {
        m_pData = videoFrame.m_pData;
        videoFrame.m_pData = nullptr;
        
        m_vWidth = videoFrame.m_vWidth;
        m_vHeight = videoFrame.m_vHeight;
        m_vPosition = videoFrame.m_vPosition;
        m_vForamt = videoFrame.m_vForamt;
    }
    
    return *this;
}

void* CSJVideoFrame::getData() {
    return m_pData;
}
