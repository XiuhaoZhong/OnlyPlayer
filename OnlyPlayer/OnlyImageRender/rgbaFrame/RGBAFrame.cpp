//
//  RGBAFrame.cpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/10/6.
//

#include "RGBAFrame.h"

#define LOG_TAG "RGBAFrame"

RGBAFrame::RGBAFrame() {
    position = 0.0f;
    duration = 0.0f;
    pixels = NULL;
    width = 0;
    height = 0;
}

RGBAFrame::~RGBAFrame() {
    if (pixels == NULL) {
        delete [] pixels;
        pixels = NULL;
    }
}

RGBAFrame * RGBAFrame::clone() {
    RGBAFrame *ret = new RGBAFrame();
    ret->duration = this->duration;
    ret->width = this->width;
    ret->height = this->height;
    ret->position = this->position;
    
    int pixelsLength = this->width * this->height * 4;
    ret->pixels = new uint8_t[pixelsLength];
    memcpy(ret->pixels, this->pixels, pixelsLength);
    
    return ret;
}
