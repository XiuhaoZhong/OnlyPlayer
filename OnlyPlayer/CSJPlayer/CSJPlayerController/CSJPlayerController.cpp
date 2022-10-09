//
//  CSJPlayer.cpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2022/3/9.
//

#include "CSJPlayerController.hpp"

CSJPlayerController::CSJPlayerController() {
    
}

CSJPlayerController::~CSJPlayerController() {
    
}

bool CSJPlayerController::init(std::string file) {
    /**
     *  1.Create CSJDecoder instance,  openFile.
     *
     *  2.Create the video render context and render thread.
     *
     *  3.Create the audio render context and render thread.
     */
    
    return false;
}

void CSJPlayerController::start() {
    /**
     *  1. invoke decoder start.
     *
     *  2.start the render thread after a small time.
     *
     *  3.set the current status is started.
     */
}

void CSJPlayerController::pause() {
    /**
     *  1.pause the decoder thread.
     *
     *  2.pause the render thread.
     */
}

void CSJPlayerController::resume() {
    /**
     *  1.pause the decoder thread.
     *
     *  2.pause the render thread.
     */
}

void CSJPlayerController::stop() {
    /**
     *  1.stop the decoder thread.
     *
     *  2.stop the render thread.
     *
     *  3.clear the decoder resources.
     */
}
