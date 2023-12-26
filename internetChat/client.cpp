#define _GNU_SOURCE 1
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include<stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <iostream>

#define BUFFER_SIZE 64

int main(int argc, char* argv[])
{
    using namespace std;
    if(argc <= 2){
        cout<<"usage:"<<basename(argv[0])<<"ip_address port_number\n";
        return 1;
    }
    const char* ip = argv[1]; // ip地址
    int port = atoi(argv[2]); // 字符串转port

    // 创建套接字，
    struct sockaddr_in server_address; // 表示IPv4地址和端口信息
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server_address.sin_addr);
    server_address.sin_port = htons(port);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0); // 创建socket
    assert(sockfd >= 0); // 创建失败为 -1
    if(connect(sockfd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) // 和server_address建立网络通信连接
    {
        cout<<"connection failed\n";
        close(sockfd);
        return 1;
    }
    //cout<<"连接建立成功\n";
    pollfd fds[2];
    fds[0].fd = 0;
    fds[0].events =POLLIN; // 等待事件
    fds[0].revents = 0;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN |POLLRDHUP; // 可读|挂起
    fds[1].revents = 0;

    char read_buf[BUFFER_SIZE];
    int pipefd[2];
    int ret = pipe(pipefd); // 一对文件描述符，一个读取，一个写入
    assert(ret != -1);

    while(1)
    {
        //cout<<"成功建立连接\n";
        ret = poll(fds, 2, -1); // -1 等待
        if(ret < 0)
        {
            cout<<"poll failure\n";
            break;
        }

        if(fds[1].revents & POLLRDHUP) // 通信结束
        {
            cout<<"server close the connection\n";
            break;
        }
        else if(fds[1].revents & POLLIN) // 通信开始
        {
            //cout<<"通信开始\n";
            memset(read_buf,'\0',BUFFER_SIZE);
            recv(fds[1].fd, read_buf, BUFFER_SIZE - 1, 0); // 接受数据
            cout<<read_buf<<endl;
        }
        if(fds[0].revents & POLLIN) // 上传数据
        {
            ret = splice(0, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
            ret = splice(pipefd[0], NULL, sockfd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
            //printf("pipefd[0]:%d pipefd[1]:%d\n",pipefd[0],pipefd[1]);
        }
    }
    close(sockfd);
    return 0;
}
