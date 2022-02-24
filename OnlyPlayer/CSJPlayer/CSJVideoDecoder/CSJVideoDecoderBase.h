//
//  CSJVideoDecoder.hpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/11/3.
//

#ifndef CSJVideoDecoderBase_H
#define CSJVideoDecoderBase_H

#include <stdio.h>
#include <string>

using std::string;

/*
 * 视频解码器基类，包括视频解码和音频解码;
 *
 * 定义解码器接口，便于各种解码器扩展;
 */
class CSJVideoDecoderBase {
public:
    CSJVideoDecoderBase();
    virtual ~CSJVideoDecoderBase();
    
    virtual int openFile(string url);
    virtual void closeFile();
    virtual bool isEOF();
    // 各个子类有不同的解码方法，交给各个子类自己实现
    virtual void decodeFrame(float duration) = 0;
    
    // 解码器的常规操作;
    virtual void start() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;
    
    // 获取视频时间长度;
    virtual int getVideoTime() = 0;
    
protected:
    void *protoclParser;
    void *formatDemuxer;
    void *decoder;
};

#endif /* CSJVideoDecoderBase_H */
