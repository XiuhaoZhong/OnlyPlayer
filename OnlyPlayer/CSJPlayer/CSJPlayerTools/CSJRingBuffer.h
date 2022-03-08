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
#include <memory>
#include <mutex>
#include <chrono>
#include <thread>

/*
 * This ring buffer managers data with std::unique_ptr, and guraratees the data is not be copied.
 * When called get the data using pop() function, must delete after using the data;
 *
 */

template <typename T>
class CSJRingBuffer {
public:
    CSJRingBuffer(unsigned size, float fullThreshold = 0, float emptyThreshold = 0) : m_vSize(size),
                                                                                      m_vFront(0),
                                                                                      m_vRear(0),
                                                                                      m_vFullThreshold(fullThreshold),
                                                                                      m_vEmptyThreshold(emptyThreshold) {
        m_ptrData = new std::unique_ptr<T>[size];
    }
    
    ~CSJRingBuffer() {
        delete []m_ptrData;
    }
    
    void push(std::unique_ptr<T> element) {
        // std::unique_lock与std::lock_guard 在上锁方面，两者一样;
        // 主要区别在于lock_guard 禁止拷贝构造和移动构造
        // unique_lock 支持移动构造，需要时可以转移;
        
        //std::lock_guard<std::mutex> lock(m_vRWMutex);
        std::unique_lock<std::mutex> lock(m_vRWMutex);
        
        if (isFull()) {
            m_vNotFull_cv.wait(lock);
        }
        
        m_ptrData[m_vRear] = std::move(element);
        m_vRear = (m_vRear + 1) % m_vSize;
    }
    
    std::unique_ptr<T> pop() {
        //std::lock_guard<std::mutex> lock(m_vRWMutex);
        std::unique_lock<std::mutex> lock(m_vRWMutex);
        
        if (isEmpty()) {
            // TODO: log current buffer is null;
            return nullptr;
        }
        
        std::unique_ptr<T> temp = std::move(m_ptrData[m_vFront]);
        m_vFront = (m_vFront + 1) % m_vSize;
        
        if (m_vEmptyThreshold > 0 && getFullRate() < m_vEmptyThreshold) {
            m_vNotEmpty_cv.notify_one();
        } else {
            m_vNotEmpty_cv.notify_one();
        }
        
        if (m_vEmptyThreshold == 0) {
            // 不使用缓冲区满的阈值，直接通知可以插入数据;
            m_vNotFull_cv.notify_one();
        } else if (m_vEmptyThreshold > 0 && getFullRate() < m_vEmptyThreshold) {
            // 使用了缓冲区满的阈值，当低于空档阈值之后，通知可以插入数据;
            m_vNotFull_cv.notify_one();
        } else {
            // 使用了缓冲区满的阈值，当低于空档阈值之后，通知可以插入数据;
        }
        
        return temp;
    }
    
    bool isEmpty() {
        return m_vFront == m_vRear;
    }

    bool isFull() {
        return m_vFront == (m_vRear + 1) % m_vSize;
    }
    
    float getFullRate() {
        return (m_vRear - m_vFront) * 1.0 / m_vSize;
    }
    
private:
    size_t  m_vSize;
    int     m_vFront;
    int     m_vRear;
    
    float   m_vFullThreshold;   // push时的阈值;
    float   m_vEmptyThreshold;  // pop后的阈值;
    
    std::mutex m_vRWMutex;
    
    std::condition_variable m_vNotFull_cv;
    std::condition_variable m_vNotEmpty_cv;
    
    std::unique_ptr<T> *m_ptrData;
    
};

#endif /* CSJRingBuffer_hpp */
