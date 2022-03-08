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
    CSJVideoFrame(uint8_t *data, int width, int height,float position, int format);
    CSJVideoFrame(CSJVideoFrame&& videoFrame);
    CSJVideoFrame& operator=(CSJVideoFrame&& videoFrame);
    ~CSJVideoFrame();
    
    uint8_t* getData();
    
private:
    uint8_t *m_pData;      // 视频数据;
    int     m_vWidth;      // 视频宽;
    int     m_vHeight;     // 视频高;
    float   m_vPosition;   // 当前数据的时间戳;
    int     m_vForamt;     // 视频数据格式;
    int     m_vDataLen;     // 数据长度;
};

#endif /* CSJVideoFrame_hpp */
