//
//  CSJDecoderController.cpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/8/15.
//

#include "CSJDecoderController.hpp"

#include "CSJDecoder.h"

#define LOG_TAG "CSJDecoderController"

CSJDecoderController::CSJDecoderController() {
    pcmFile = NULL;
    csjDecoder = NULL;
    bufferCnt = 0;
}

CSJDecoderController::~CSJDecoderController() {
    
}

int CSJDecoderController::init(const char *accompanyPath, const char *pcmFilePath) {
    // 初始化两个decoder;
    CSJDecoder *tempDecoder = new CSJDecoder();
    int accompanyMetaData[2];
    int ret = tempDecoder->getMusicMeta(accompanyPath, accompanyMetaData);
    delete tempDecoder;
    
    if (ret != 0) {
        return ret;
    }
    
    accompanySampleRate = accompanyMetaData[0];
    int accompanyByteCountPerSec = accompanySampleRate * CHANNEL_PER_FRAME * BITS_PER_CHANNEL / BITS_PER_BYTE;
    accompanyPacketBufferSize = (int)((accompanyByteCountPerSec / 2) * 0.2); // 乘以0.2啥意思？ 2021/08/15;
    csjDecoder = new CSJDecoder();
    csjDecoder->init(accompanyPath, accompanyPacketBufferSize);
    pcmFile = fopen(pcmFilePath, "wb+");
    
    return 0;
}

void CSJDecoderController::Decode() {
    while (true) {
        AudioPacket *accompanyPacket = csjDecoder->decodePacket();
        if (accompanyPacket->size == -1) {
            break;
        }
        bufferCnt++;
        fwrite(accompanyPacket->buffer, sizeof(short), accompanyPacket->size, pcmFile);
        LOGI("write buffer to pcmFile, bufferSize: %d, bufferCnt: %d", accompanyPacket->size, bufferCnt);
        if (bufferCnt == 1545) {
            LOGI("write buffer to pcmFile, bufferSize: %d, bufferCnt: %d", accompanyPacket->size, bufferCnt);
        }
    }
}

void CSJDecoderController::Destroy() {
    if (csjDecoder) {
        csjDecoder->destroy();
        delete csjDecoder;
        csjDecoder = NULL;
    }
    
    if (pcmFile) {
        fclose(pcmFile);
        pcmFile = NULL;
    }
}
