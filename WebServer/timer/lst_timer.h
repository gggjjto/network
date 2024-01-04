#ifndef LST_TIMER
#define LST_TIMER

#include<arpa/inet.h>
#include <time.h>
#include "../http/http_conn.h"
#include "../log/log.h"

class util_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    util_timer *timer;
};

class util_timer
{
    public:
        util_timer() : prev(NULL), next(NULL){}
    public:
        time_t expire;

        void (* cb_func)(client_data *);
        client_data *user_data;
        util_timer *prev;
        util_timer *next;
};

class sort_timer_lst
{
    public:
        sort_timer_lst();
        ~sort_timer_lst();

        void add_timer(util_timer *timer);
        void adjust_timer(util_timer *timer);
        void del_timer(util_timer *timer);
        void tick();

    private:
        void add_timer(util_timer *timer, util_timer *lst_head);

        util_timer *head;
        util_timer *tail;
};

class Utils
{
    public:
        Utils() {}
        ~Utils() {}

        void init(int timeslot);
        int setnonblocking(int fd);
        void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);
        static void sig_handler(int sig);
        void addsig(int sig, void(handler)(int), bool restart = true);
        void timer_handler();
        void show_error(int connfd, const char *info);

    public:
        static int *u_pipefd;
        sort_timer_lst m_timer_lst;
        static int u_epollfd;
        int m_TIMESLOT;
};

void cb_func(client_data *user_data);

#endif
