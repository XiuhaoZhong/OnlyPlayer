//
//  CSJMeidaSyncer.cpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2022/2/23.
//

#include "CSJMediaSyncer.hpp"

#include "CSJAudioFrame.h"
#include "CSJVideoFrame.h"

#include "CSJFFmpegVideoDecoder.hpp"

CSJMediaSynchronizer::CSJMediaSynchronizer() {
    m_pDecoder = nullptr;
}

CSJMediaSynchronizer::~CSJMediaSynchronizer() {
    
}

bool CSJMediaSynchronizer::init(CSJDecoderType type, std::string * filePath) {
    bool init = false;
    
    if (!filePath || filePath->size() == 0) {
        return false;
    }
    
    m_pDecoder = createDecoderByType(type);
    if (!m_pDecoder) {
        // TODO: log decoder create failed;
        return false;
    }
    
    init = m_pDecoder->openFile(*filePath);
    
    return init;
}

void CSJMediaSynchronizer::startDecode() {
    
}

void CSJMediaSynchronizer::pauseDecode() {
    
}

void CSJMediaSynchronizer::resumeDecode() {
    
}

void CSJMediaSynchronizer::stopDecode() {
    
}

void CSJMediaSynchronizer::fillAudioData(std::unique_ptr<CSJAudioFrame> audioData) {
    if (!m_pAudioRingBuffer) {
        return ;
    }
    
    if (!m_pAudioRingBuffer->isFull()) {
        // TODO: audio ring buffer is full;
    }
    
    m_pAudioRingBuffer->push(std::move(audioData));
}

void CSJMediaSynchronizer::fillVideoData(std::unique_ptr<CSJVideoFrame> videoData) {
    if (!m_pVideoRingBuffer) {
        return ;
    }
    
    if (!m_pVideoRingBuffer->isFull()) {
        // TODO: audio ring buffer is full;
    }
    
    m_pVideoRingBuffer->push(std::move(videoData));
}

std::unique_ptr<CSJAudioFrame> CSJMediaSynchronizer::getNextAudioData() {
    if (!m_pAudioRingBuffer || m_pAudioRingBuffer->isEmpty()) {
        return nullptr;
    }
    
    return m_pAudioRingBuffer->pop();
}

std::unique_ptr<CSJVideoFrame> CSJMediaSynchronizer::getNextVideoData() {
    if (!m_pVideoRingBuffer || m_pVideoRingBuffer->isEmpty()) {
        return nullptr;
    }
    
    return m_pVideoRingBuffer->pop();
}

CSJVideoDecoderBase* CSJMediaSynchronizer::createDecoderByType(CSJDecoderType type) {
    CSJVideoDecoderBase *decoder = nullptr;
    
    switch (type) {
        case CSJDecoderType_FFMpeg:
            decoder = new CSJFFmpegVideoDecoder();
            break;
        default:
            decoder = nullptr;
    }
    
    return decoder;
}
