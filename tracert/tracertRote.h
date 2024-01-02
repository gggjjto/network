#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip_icmp.h>
#include <assert.h>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

#define MAX_HOPS 30
#define MAX_ATTEMPTS 3
#define TIMEOUT 1
class tracertRote
{
private:
    
public:
    struct timeval sendTime;
    struct timeval receiveTime;
    std::string tracertAddress;
    std::string replyAdress;

    // 校验和
    unsigned short checksum(void* icmpHdr, int ttl)
    {
        unsigned short *buf = static_cast<unsigned short *>(icmpHdr);
        unsigned int sum = 0;
        unsigned short result;

        for(sum = 0;ttl > 1; ttl -= 2){
            sum += *buf++;
        }
        if(ttl == 1){
            sum += *(unsigned char *)buf;
        }

        sum  = (sum >> 16) + (sum & 0xFFFF);
        sum += (sum >> 16);
        result = ~sum;

        return result;
    }
    // 进行路由追踪
    void tracertRotePing()
    {
        int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
        assert(sockfd > 0);
        std::cout<< "Tracertrote to " << tracertAddress <<std::endl;
        for (int ttl = 1; ttl <= MAX_HOPS; ttl++)
        {
            std::cout<< ttl <<" ";
            bool reachedTarget = false;
            double rtt = 0.0;

            for(int attempt = 1; attempt <= MAX_ATTEMPTS; attempt++)
            {
                if(sendEchoRequest(sockfd, ttl))
                {
                    if(receiveEchoReply(sockfd, ttl, rtt))
                    {
                        reachedTarget = true;
                        break;
                    }
                
                }
                usleep(500000);
            }

            if(reachedTarget)
            {
                std::cout<< replyAdress <<" "<< rtt*1.0/1000 <<"ms" << std::endl;
                if(replyAdress == tracertAddress){
                    break;
                }
            }
            else 
            {
                std::cout<< "*" <<std::endl;
            }
            
        }
        close(sockfd);
    }
    // 发送ICMP回显请求
    bool sendEchoRequest(int sockfd, int ttl)
    {
        struct sockaddr_in targetAddr{};
        targetAddr.sin_family = AF_INET;
        targetAddr.sin_port = 8080;
        if(inet_pton(AF_INET, tracertAddress.c_str(),&targetAddr.sin_addr) <= 0){
            std::cerr<< "Error: Invalid address"<<std::endl;
            return false;
        }

        struct icmphdr icmpHeader;
        icmpHeader.code = 0;
        icmpHeader.type = ICMP_ECHO;
        icmpHeader.checksum = 0;
        icmpHeader.un.echo.id = getpid();
        icmpHeader.un.echo.sequence = ttl;
        icmpHeader.checksum = tracertRote::checksum(&icmpHeader, sizeof(icmpHeader));

        if(setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int)) < 0){
            std::cerr<< "Error: Failed to set TTL" <<std::endl;
            return false;
        }

        gettimeofday(&sendTime, NULL);
        if(sendto(sockfd, &icmpHeader, sizeof(icmpHeader), 0, (struct sockaddr *)&targetAddr, sizeof(targetAddr)) <= 0)
        {
            std::cerr<<"Error: Failed to send ICMP packet" << std::endl;
            return false;
        }

        return true;
    }
    // 接收ICMP回显应答
    bool receiveEchoReply(int sockfd, int ttl, double &rtt)
    {
        struct sockaddr_in senderAddr;
        socklen_t senderAddrLen = sizeof(senderAddr);

        char buffer[IP_MAXPACKET];

        struct timeval timeout;
        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;

        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(sockfd, &readSet);

        if(select(sockfd + 1, &readSet, NULL, NULL, &timeout) > 0)
        {
            if(recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr *)&senderAddr, &senderAddrLen) > 0)
            {
                struct ip *ipHeader = (struct ip *)buffer;
                struct icmphdr *icmpHeader = (struct icmphdr *)(buffer + (ipHeader->ip_hl << 2));
                gettimeofday(&receiveTime, NULL);

                if(icmpHeader->type == ICMP_TIME_EXCEEDED)
                {
                    struct ip *innerIpHeader = (struct ip *)(buffer + (ipHeader->ip_hl << 2) + 8);
                    struct icmphdr *innerIcmpHeader = (struct icmphdr *)((char *)innerIpHeader + (innerIpHeader->ip_hl << 2));
                    
                    replyAdress = inet_ntoa(senderAddr.sin_addr);
                    rtt = (receiveTime.tv_sec - sendTime.tv_sec) * 1000000 + (receiveTime.tv_usec - sendTime.tv_usec);
                    return true;
                }
                else if(icmpHeader->type == ICMP_ECHOREPLY)
                {
                    replyAdress = inet_ntoa(senderAddr.sin_addr);
                    rtt = rtt = (receiveTime.tv_sec - sendTime.tv_sec) * 1000000 + (receiveTime.tv_usec - sendTime.tv_usec);
                    return true;
                }
            }
        }
        return false;
    }

    tracertRote(std::string s):tracertAddress(s) {}
    ~tracertRote(){}
};
