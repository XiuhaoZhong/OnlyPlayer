//
//  CSJVideoFrame.hpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/11/3.
//

#ifndef CSJVideoFrame_hpp
#define CSJVideoFrame_hpp

#include <stdio.h>
#include <stdlib.h>

/*
 * 表示视频帧数据;
 */
class CSJVideoFrame {
public:
    CSJVideoFrame();
    ~CSJVideoFrame();
    
    void* getData();
    
private:
    uint8_t *data;      // 视频数据;
    int     width;      // 视频宽;
    int     height;     // 视频高;
    float   position;   // 当前数据的时间戳;
    int     foramt;     // 视频数据格式;
};

#endif /* CSJVideoFrame_hpp */
