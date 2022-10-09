//
//  CSJPlayer.hpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2022/3/9.
//

#ifndef CSJPlayer_hpp
#define CSJPlayer_hpp

#include <stdio.h>
#include <string>

#include "CSJPlayerCommonData.h"

using std::string;

class CSJMediaSyncer;

/*
 * the kernel class for the Player, responses for achieving
 * the main operations which player provided for the users.
 *
 * the CSJPlayerController is the top inner class in the
 * player framework.
 *
 */
class CSJPlayerController {
public:
    CSJPlayerController();
    ~CSJPlayerController();
    
    bool init(std::string file);
    
    void start();
    void pause();
    void resume();
    void stop();
    
    static CSJPlayerController* sharedInstance();
    
private:
    std::shared_ptr<CSJMediaSyncer> m_pMediaSyncer;
    
    CSJPlayerStatus status;
};

#endif /* CSJPlayer_hpp */
