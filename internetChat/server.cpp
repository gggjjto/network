#define _GNU_SOURCE 1

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <poll.h>
#include <iostream>
#include <assert.h>

#define USER_LIMIT 5
#define BUFFER_SIZE 64
#define FD_LIMIT 65535
/**
 * 存储客户端数据
*/
struct client_data
{
    sockaddr_in address;
    char* write_buf;
    char buf[BUFFER_SIZE];
};
/**
 * 设置为非阻塞模式 
*/
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int main(int argc, char* argv[])
{
    using namespace std;
    if(argc <= 2)
    {
        cout<< "usage:"<<basename(argv[0])<<"ip_address prot_number\n";
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    // 创建套接字，监听连接
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip , &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    ret = bind(listenfd, (struct sockaddr*) &address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5); // 监听服务器端开始传入连接请求的函数，等待连接数5
    assert(ret != 1);

    client_data* users = new client_data[FD_LIMIT]; // 储存客户端数据
    pollfd fds[USER_LIMIT + 1]; // 监视文件描述符上发生事件的数据结构
    int user_counter = 0;
    for(int i = 1; i <= USER_LIMIT; i++)
    {
        fds[i].fd = -1;
        fds[i].events = 0;
    }
    fds[0].fd = listenfd;
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0; 

    while(1)
    {
        ret = poll(fds, user_counter + 1, -1);
        if(ret < 0)
        {
            cout<<"poll failure\n";
            break;
        }

        for(int i = 0; i< user_counter + 1; i++){
            // 该文件描述符开始传入
            if((fds[i].fd == listenfd) && (fds[i].revents & POLLIN))
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength); // 接受客户端的连接请求，并存储客户端的地址信息
                if(connfd < 0){
                    printf("errno is: %d\n", errno);
                    continue;
                }
                //printf("地址：%d\n",connfd);
                if(user_counter >= USER_LIMIT)
                {
                    const char* info = "too many users\n";
                    cout<<info<<endl;
                    send(connfd, info, strlen(info), 0); // 用于发送数据到已连接的对等方
                    close(connfd);
                    continue;
                }
                user_counter++; // 连接的用户数量
                users[connfd].address = client_address;
                setnonblocking(connfd);
                fds[user_counter].fd = connfd; 
                fds[user_counter].events = POLLIN | POLLRDHUP | POLLERR; //文件描述符上发生了错误事件，例如连接被关闭或出现读写错误
                fds[user_counter].revents = 0;
                cout<<"comes a new user, now have"<<user_counter<< "users\n";
            }
            else if(fds[i].revents & POLLERR)
            {
                cout<<"get an error from "<<fds[i].fd<<"\n";
                char errors[100];
                memset(errors, '\0', 100);
                socklen_t length = sizeof(errors);
                if(getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &errors, &length) < 0) // 拿出错误
                {
                    cout<<"get socket option failed\n";
                }
                cout<<errors<<'\n';
                continue;
            }
            else if(fds[i].revents & POLLRDHUP) // 挂起结束
            {
                users[fds[i].fd] = users[fds[user_counter].fd];
                close(fds[i].fd);
                fds[i] = fds[user_counter];
                i--;
                user_counter--;
                cout<<"a client left\n";
            }
            else if(fds[i].revents & POLLIN) // 收到请求开始
            {
                int connfd = fds[i].fd;
                memset(users[connfd].buf, '\0', BUFFER_SIZE);
                ret = recv(connfd, users[connfd].buf, BUFFER_SIZE - 1, 0); // 接受数据
                printf("get %d bytes of client data %s from %d\n", ret, users[connfd].buf, connfd); // 说明数据的各项数据
                if(ret < 0)
                {
                    if(errno != EAGAIN)
                    {
                        close(connfd);
                        users[fds[i].fd] = users[fds[user_counter].fd];
                        fds[i] = fds[user_counter];
                        i--;
                        user_counter--;
                    }
                }
                else if(ret == 0){

                }
                else {
                    for(int j = 1; j<= user_counter; j++){
                        if(fds[j].fd == connfd){
                            continue;
                        }

                        fds[j].events |= ~POLLIN; // 清除POLLIN位
                        fds[j].events |= POLLOUT;
                        users[fds[j].fd].write_buf = users[connfd].buf;
                    }
                }
            }
            else if(fds[i].revents & POLLOUT) // 写
            {
                int connfd = fds[i].fd;
                if(!users[connfd].write_buf){
                    continue;
                }
                ret = send( connfd, users[connfd].write_buf,strlen(users[connfd].write_buf), 0); // 发送数据
                users[connfd].write_buf = NULL;
                fds[i].events |= ~POLLOUT;
                fds[i].events |= POLLIN;
            }
        }
    }
    delete [] users;
    close(listenfd);
    return 0;
}



