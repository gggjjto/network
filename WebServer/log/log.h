#ifndef LOG_H
#define LOG_H

#include "block_queue.h"

using namespace std;

class Log
{
private:
    char dir_name[128];
    char log_name[128];
    int m_split_lines;
    int m_log_buf_size;
    long long m_count;
    int m_today;
    FILE *m_fp;
    char *m_buf;
    block_queue<string> *m_log_queue;
    bool m_is_async;
    locker mutex;
    int m_close_log;

private:
    Log(/* args */);
    virtual ~Log();
    void *async_write_log(){
        string single_log;
        while(m_log_queue->pop(single_log)){
            mutex.lock();
            fputs(single_log.c_str(),m_fp);
            mutex.unlock();
        }
    }
public:

    static Log *get_instance(){
        static Log instance;
        return &instance;
    }

    static void *flush_log_thread(void *args){
        Log::get_instance()->async_write_log();
    }

    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);
    void write_log(int level, const char *format, ...);
    void flush(void);
};

#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#endif
