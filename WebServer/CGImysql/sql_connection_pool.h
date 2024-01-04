#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include<mysql/mysql.h>
#include<list>
#include "../lock/locker.h"
#include "../log/log.h"

using namespace std;

class connection_pool
{
    public:
        string m_Url;
        string m_Port;
        string m_User;
        string m_PassWord;
        string m_DatabaseName;
        int m_close_log;
    public:
        MYSQL *GetConnection();
        bool ReleaseConnection(MYSQL *con);
        int GetFreeConn();
        void DestroyPool();

        static connection_pool *GetInstance();
        void init(string url, string User, string PassWord, string DataBaseName, int Port, int MaxConn, int close_log);

    private:
    connection_pool();
    ~connection_pool();

    int m_MaxConn;
    int m_CurConn;
    int m_FreeConn;
    locker lock;
    list<MYSQL *> connList;
    sem reserve;
};

class connectionRAII{
    public:
        connectionRAII(MYSQL **con, connection_pool *connpool);
        ~connectionRAII();
    
    private:
        MYSQL *conRAII;
        connection_pool *poolRAII;
};
#endif //_CONNECTION_POOL_

