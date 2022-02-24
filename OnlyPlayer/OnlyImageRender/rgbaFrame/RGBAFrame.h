//
//  RGBAFrame.hpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/10/6.
//

#ifndef RGBAFrame_h
#define RGBAFrame_h

#include <string>

class RGBAFrame {
public:
    RGBAFrame();
    ~RGBAFrame();
    RGBAFrame* clone();
    
public:
    float position;
    float duration;
    uint8_t *pixels;
    int width;
    int height;
};

#endif /* RGBAFrame_hpp */
