#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <pthread.h>
#include "ClientSession.h"



class TCPServer {
public:
    TCPServer(int port);
    ~TCPServer() {
        // 关闭监听套接字和 epoll 实例
        close(listenSocket);
        close(epollFd);
    }
    void run();


private:
    int port;
    int listenSocket;
    int epollFd;
    std::vector<int> clientSockets;
    std::map<int, pClientSession> fdClientMap;

    void handleNewConnection();
    void handleClientEvent(int fd);
};

#endif