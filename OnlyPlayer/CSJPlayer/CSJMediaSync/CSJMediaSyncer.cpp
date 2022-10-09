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
    
    m_pAudioRingBuffer = new CSJRingBuffer<CSJAudioFrame>(20);
    m_pVideoRingBuffer = new CSJRingBuffer<CSJVideoFrame>(20);
}

CSJMediaSynchronizer::~CSJMediaSynchronizer() {
    if (m_pAudioRingBuffer) {
        delete m_pAudioRingBuffer;
        m_pAudioRingBuffer = nullptr;
    }
    
    if (m_pVideoRingBuffer) {
        delete m_pVideoRingBuffer;
        m_pVideoRingBuffer = nullptr;
    }
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
    if (m_pDecoder) {
        m_pDecoder->start();
    }
}

void CSJMediaSynchronizer::pauseDecode() {
    if (m_pDecoder) {
        m_pDecoder->pause();
    }
}

void CSJMediaSynchronizer::resumeDecode() {
    if (m_pDecoder) {
        m_pDecoder->resume();
    }
}

void CSJMediaSynchronizer::stopDecode() {
    if (m_pDecoder) {
        m_pDecoder->stop();
    }
}

void CSJMediaSynchronizer::setDecoderDataDelegate(std::shared_ptr<CSJDecoderDataDelegate> dataDelegate) {
    m_pDecoder->setDataDelegate(dataDelegate);
}

void CSJMediaSynchronizer::fillAudioData(std::unique_ptr<CSJAudioFrame> audioData) {
    if (!m_pAudioRingBuffer) {
        return ;
    }
    
    m_pAudioRingBuffer->push(std::move(audioData));
}

void CSJMediaSynchronizer::fillVideoData(std::unique_ptr<CSJVideoFrame> videoData) {
    if (!m_pVideoRingBuffer) {
        return ;
    }
    
    m_pVideoRingBuffer->push(std::move(videoData));
}

std::unique_ptr<CSJAudioFrame> CSJMediaSynchronizer::getNextAudioData() {
    std::lock_guard<std::mutex> lock(m_vDataMutex);
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

std::shared_ptr<CSJVideoDecoderBase> CSJMediaSynchronizer::createDecoderByType(CSJDecoderType type) {
    switch (type) {
        case CSJDecoderType_FFMpeg:
            return std::make_shared<CSJFFmpegVideoDecoder>();
            break;
        default:
            return nullptr;
    }
    
    return nullptr;
}
