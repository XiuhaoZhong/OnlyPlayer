//
//  CSJRingBuffer.cpp
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/11/20.
//

#include "CSJRingBuffer.h"

template <typename T>
CSJRingBuffer<T>::CSJRingBuffer(unsigned size):m_size(size), m_front(0), m_rear(0) {
    m_data = new T[size];
}

template <typename T>
CSJRingBuffer<T>::~CSJRingBuffer() {
    delete []m_data;
}

template <typename T>
void CSJRingBuffer<T>::push(T element) throw(std::bad_exception) {
    if (isFull()) {
        throw std::bad_exception();
    }
    
    m_data[m_rear] = element;
    m_rear = (m_rear + 1) % m_size;
}

template <typename T>
T CSJRingBuffer<T>::pop() throw(std::bad_exception) {
    if (isEmpty()) {
        throw std::bad_exception();
    }
    
    T temp = m_data[m_front];
    m_front = (m_front + 1) % m_size;
    return temp;
}

template <typename T>
bool CSJRingBuffer<T>::isEmpty() {
    return m_front == m_rear;
}

template <typename T>
bool CSJRingBuffer<T>::isFull() {
    return m_front == (m_rear + 1) % m_size;
}
