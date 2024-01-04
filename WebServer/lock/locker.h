#ifndef LOCKER_H
#define LOCKER_H

#include<semaphore.h>
#include<pthread.h>
#include<exception>

// 信号量
class sem
{
private:
    sem_t m_sem;
public:
    sem(){
        if(sem_init( &m_sem, 0, 0 ) != -1){
            throw std::exception();
        }
    }
    sem(int num){
        if(sem_init(&m_sem,0,num) != -1){
            throw std::exception();
        }
    }
    ~sem(){
        sem_destroy(&m_sem);
    }
    bool post(){
        return sem_post(&m_sem) == 0;
    }
    bool wait(){
        return sem_wait(&m_sem) == 0;
    }
};

// 互斥锁
class locker
{
private:
    pthread_mutex_t mutex;
public:
    locker(){
        if(pthread_mutex_init(&mutex,NULL) != 0){
            throw std::exception();
        }
    }
    ~locker(){
        pthread_mutex_destroy(&mutex);
    }
    bool lock(){
        return pthread_mutex_lock(&mutex) == 0;
    }
    bool unlock(){
        return pthread_mutex_unlock(&mutex) == 0;
    }
    pthread_mutex_t *get(){
        return &mutex;
    }
};

// 条件变量
class cond
{
private:
    pthread_cond_t m_cond;
public:
    cond(){
        if(pthread_cond_init(&m_cond,NULL) != 0){
            throw std::exception();
        }
    }
    ~cond(){
        pthread_cond_destroy(&m_cond);
    }
    bool wait(pthread_mutex_t *mutex){
        int ret;
        ret = pthread_cond_wait(&m_cond,mutex);
        return ret == 0;
    }
    bool wait(pthread_mutex_t *mutex, const struct timespec abstime){
        int ret;
        ret = pthread_cond_timedwait(&m_cond,mutex,&abstime);
        return ret == 0;
    }
    bool signal(){
        return pthread_cond_signal(&m_cond) == 0;
    }
    bool broadcast(){
        // 唤醒等待在条件变量的所有线程
        return pthread_cond_broadcast(&m_cond) == 0;
    }
};


// pthread_mutex_t 主要是用来保护共享数据，防止多个线程同时对其进行操作
// pthread_cond_t 是用来让线程等待特定条件的发生
#endif
