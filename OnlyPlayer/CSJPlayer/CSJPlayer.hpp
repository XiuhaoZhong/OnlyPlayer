//
//  CSJPlayer.hpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2022/3/9.
//

#ifndef CSJPlayer_hpp
#define CSJPlayer_hpp

#include <stdio.h>
#include <memory>
#include <string>

#include "CSJPlayerCommonData.h"

using std::string;

class CSJPlayerContoller;

/**
 * This class is the enter for the player. and provided all the functions which are need
 * by user.
 *
 * such as play operations, including start, pause, resume, stop and so on.
 *
 * such as get play informations, including player status, total time, play time and so on.
 *
 * such as audio and video informations.
 *
 */
class CSJPlayer {
public:
    CSJPlayer();
    ~CSJPlayer();
    
    bool init();
    
    /**
     * Set the file will be played.
     *
     * @return true: the file could be played, or couldn't be played.
     */
    bool setFileWithPath(std::string *filePath);
    bool setFileWithUrl(std::string *fileUrl);
    
    void start();
    void pause();
    void resume();
    void stop();
    
    CSJPlayerStatus getPlayerStatus();
    
    // 获取总时间;
    float totalTime();
    // 当前正在播放的时间;
    float currentTime();
    // 获取视频的分辨率;
    std::pair<int, int> getVideoResolution();
    
private:
    std::shared_ptr<CSJPlayerContoller> m_pController;
    
    static CSJPlayer *player;
};

#endif /* CSJPlayer_hpp */
