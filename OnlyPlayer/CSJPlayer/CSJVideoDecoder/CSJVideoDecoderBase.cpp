//
//  CSJVideoDecoder.cpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/11/3.
//

#include "CSJVideoDecoderBase.h"

#include "CSJDecoderDataDelegate.hpp"

CSJVideoDecoderBase::CSJVideoDecoderBase() {
    
}

CSJVideoDecoderBase::~CSJVideoDecoderBase() {
    
}

int CSJVideoDecoderBase::openFile(string url) {
    return 0;
}

void CSJVideoDecoderBase::closeFile() {
    
}

bool CSJVideoDecoderBase::isEOF() {
    return true;
}

void CSJVideoDecoderBase::setDataDelegate(std::shared_ptr<CSJDecoderDataDelegate> delegate) {
    m_pDataDelegate = delegate;
}
