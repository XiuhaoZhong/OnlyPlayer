//
//  CSJRingBuffer.hpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/11/20.
//

#ifndef CSJRingBuffer_hpp
#define CSJRingBuffer_hpp

#include <stdio.h>

#include <iostream>

template <typename T>
class CSJRingBuffer {
public:
    CSJRingBuffer(unsigned size);
    ~CSJRingBuffer();
    
    void push(T element)throw(std::bad_exception);
    
    T pop() throw(std::bad_exception);
    
    bool isEmpty();
    
    bool isFull();
    
private:
    size_t m_size;
    int m_front;
    int m_rear;
    T m_data;
    
};

#endif /* CSJRingBuffer_hpp */
