/**************
 * 阻塞队列
***************/
#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include<iostream>
#include<sys/time.h>
#include "lock/locker.h"

template <class T>
class block_queue
{
private:
    int m_max_size;
    int m_size;
    int m_front;
    int m_back;
    T m_array;

    locker mutex;
    cond m_cond;
public:
    block_queue(int max_size = 1000){
        if(max_size <= 0){
            exit(-1);
        }
        m_max_size = max_size;
        a_array = new T[max_size];
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }
    ~block_queue(){
        mutex.lock();
        if (m_array != NULL){
            delete m_array;
        }
        mutex.unlock();
    }
    void clear(){
        mutex.lock();
        m_size = 0;
        m_front = -1;
        m_back = -1;
        mutex.unlock();
    }

    bool empty(){
        mutex.lock();
        if(m_size == 0){
            mutex.unlock();
            return true;
        }
        mutex.unlock();
        return false;
    }
    // 添加队尾元素
    bool push(const T &item){
        mutex.lock();
        if(m_size >= m_max_size){
            m_cond.broadcast();
            mutex.unlock();
            return false;
        }

        m_size++;
        m_back = (m_back + 1) % m_max_size;
        m_array[m_back] = item;
        
        m_cond.broadcast();
        mutex.unlock();
        return true;
    }
    // 添加队首元素
    bool pop(const T &item){
        mutex.lock();
        if(m_size >= m_max_size){
            m_cond.broadcast();
            mutex.unlock();
            return false;
        }

        m_size++;
        m_front = (m_front + 1) % m_max_size;
        m_array[m_front] = item;

        m_cond.broadcast();
        mutex.unlock();
        return true;
    }

    // 添加超时处理
    bool pop(const T &item,int ms_timeout){
        struct timespec t={0,0};
        struct timeval now={0,0};
        mutex.lock();
        grttimeofday(&now,NULL);
        if(m_size <= 0){
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = now.tv_usec*1000 + (ms_timeout % 1000) * 1000000;

            if(t.tv_nsec >= 1000000000){
                t.tv_sec = t.tv_nsec / 1000000000;
                t.tv_nsec = t.tv_nsec % 1000000000;
            }

            if(m_cond.wait(mutex.get(),t)){
                mutex.unlock();
                return false;
            }
        }
        if(m_size <= 0){
            mutex.unlock();
            return false;
        }

        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        mutex.unlock();
        return true;

    }

    // 返回队首元素
    bool front(T &value){
        mutex.lock();
        if(m_size <= 0){
            mutex.unlock();
            return false;
        }
        value = m_array[m_front];
        mutex.unlock();
        return true;
    }
    // 返回队尾元素
    bool back(T &value){
        mutex.lock();
        if(m_size <= 0){
            mutex.unlock();
            return false;
        }
        value = m_array[m_back];
        mutex.unlock();
        return true;
    }
    // 返回队列大小
    int size(){
        int temp;
        mutex.lock();
        temp = m_size;
        mutex.unlock();
        return temp;
    }
    
    // 返回循环队列大小
    int max_size(){
        int temp;
        mutex.lock();
        temp = m_max_size;
        mutex.unlock();
        return temp;
    }

    // 判断队列是否已满
    bool full(){
        mutex.lock();
        if(m_size>=m_max_size){
            mutex.unlock();
            return true;
        }
        return false;
    }

    // 判断队列是否为空
    bool empty(){
        mutex.lock();
        if(m_size <= 0){
            mutex.unlock();
            return true;
        }
        mutex.unlock();
        return false;
    }

};


#endif /*BLOCK_QUEUE_H*/
