//
//  CSJPlayer.cpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2022/3/9.
//

#include "CSJPlayer.hpp"

CSJPlayer *CSJPlayer::player = nullptr;

CSJPlayer::CSJPlayer() {
    
}

CSJPlayer::~CSJPlayer() {
    
}

bool CSJPlayer::init() {
    /**
     *  only init self, if this function is not need, remove this function in the future.
     */
    
    return false;
}

bool CSJPlayer::setFileWithPath(std::string *filePath) {
    /**
     *  invoke the m_pController init function.
     */
    
    return false;
}

bool CSJPlayer::setFileWithUrl(std::string *fileUrl) {
    /**
     *  invoke the m_pController init function.
     */
    
    return false;
}

void CSJPlayer::start() {
    /**
     *  invoke m_pController start function.
     */
}

void CSJPlayer::pause() {
    /**
     *  invoke m_pController pause function.
     */
}

void CSJPlayer::resume() {
    /**
     *  invoke m_pController resume function.
     */
}

void CSJPlayer::stop() {
    /**
     *  invoke m_pController start function.
     */
}

CSJPlayerStatus CSJPlayer::getPlayerStatus() {
    /**
     *  get the status from m_pController.
     */
    
    return CSJPlayerStatus_None;
}

float CSJPlayer::totalTime() {
    /**
     *  get the total time from m_pController.
     */
    return 0.0;
}

float CSJPlayer::currentTime() {
    /**
     *  get the current time from m_pController.
     */
    return 0.0;
}

std::pair<int, int> CSJPlayer::getVideoResolution() {
    /**
     *  get the resolution from m_pController.
     */
    return std::pair<int, int>(0, 0);
}
 

