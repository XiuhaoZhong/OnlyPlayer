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

/*
 * This ring buffer managers data with std::unique_ptr, and guraratees the data is not be copied.
 * When called get the data using pop() function, must delete after using the data;
 *
 */

template <typename T>
class CSJRingBuffer {
public:
    CSJRingBuffer(unsigned size):m_size(size), m_front(0), m_rear(0) {
        m_ptrData = new std::unique_ptr<T>[size];
    }
    
    ~CSJRingBuffer() {
        delete []m_ptrData;
    }
    
    void push(std::unique_ptr<T> element) throw(std::bad_exception) {
        if (isFull()) {
            throw std::bad_exception();
        }
        
        m_ptrData[m_rear] = std::move(element);
        m_rear = (m_rear + 1) % m_size;
    }
    
    std::unique_ptr<T> pop() throw(std::bad_exception) {
        if (isEmpty()) {
            throw std::bad_exception();
        }
        
        std::unique_ptr<T> temp = std::move(m_ptrData[m_front]);
        m_front = (m_front + 1) % m_size;
        
        return temp;
    }
    
    bool isEmpty() {
        return m_front == m_rear;
    }

    bool isFull() {
        return m_front == (m_rear + 1) % m_size;
    }
    
private:
    size_t m_size;
    int m_front;
    int m_rear;
    
    std::unique_ptr<T> *m_ptrData;
    
};

#endif /* CSJRingBuffer_hpp */
