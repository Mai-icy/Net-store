#include "Server.h"
#include "ThreadPool.h"
#include <functional>

TCPServer::TCPServer(int port) : port(port){
        listenSocket = socket(AF_INET, SOCK_STREAM, 0);
        // AF_INET / AF_INET6 / AF_UNIX   ipv4/ipv6/本地套接字
        if (listenSocket == -1) {
            perror("socket");
            exit(1);
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY; // inet_addr("192.168.1.100")
		
        // 绑定套接字
        if (bind(listenSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
            perror("bind");
            exit(1);
        }

        // 设置监听队列大小
        if (listen(listenSocket, 5) == -1) {
            perror("listen");
            exit(1);
        }

        // 创建 epoll 实例
        epollFd = epoll_create1(0);
        if (epollFd == -1) {
            perror("epoll_create1");
            exit(1);
        }

        // 添加监听套接字到 epoll 实例中
        epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = listenSocket;
        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, listenSocket, &event) == -1) {
            perror("epoll_ctl");
            exit(1);
        }
}

void TCPServer::run(){
    std::cout << "Server is running on port " << port << std::endl;

    std::function<void(int)> func = std::bind(&TCPServer::handleClientEvent, this, std::placeholders::_1);
    
    ThreadPool pool(4);
    while (true) {
        epoll_event events[10];
        int numEvents = epoll_wait(epollFd, events, 10, -1);

        if (numEvents == -1) {
            perror("epoll_wait");
            exit(1);
        }

        for (int i = 0; i < numEvents; i++) {
            int fd = events[i].data.fd;

            if (fd == listenSocket) {
                // 有新连接请求到达
                handleNewConnection();
            } else {
                // 有已有连接的事件
                pool.enqueue(func, fd);
            }
        }
    }
}

void TCPServer::handleNewConnection(){
    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int clientSocket = accept(listenSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrSize);

    if (clientSocket == -1) {
        perror("accept");
        return;
    }

    // 将新连接的客户端套接字设置为非阻塞
    int flags = fcntl(clientSocket, F_GETFL, 0);
    fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);

    // 添加新连接的客户端套接字到 epoll 实例中以监视其事件
    epoll_event event;
    event.events = EPOLLIN | EPOLLET; // 边缘触发模式
    event.data.fd = clientSocket;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event) == -1) {
        perror("epoll_ctl");
        close(clientSocket);
        return;
    }

    fdClientMap[clientSocket] = std::make_shared<ClientSession>("Guest");

    std::cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
    clientSockets.push_back(clientSocket);
}

void TCPServer::handleClientEvent(int fd) {
    char buffer[1024];
    int bytesRead = recv(fd, buffer, sizeof(buffer), 0);

    if (bytesRead == -1) {
        perror("recv");
    } else if (bytesRead == 0) {
        // 客户端关闭连接
        epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr);
        close(fd);
        std::cout << "Connection closed" << std::endl;
    } else {
        // 处理客户端发送的数据
        buffer[bytesRead] = '\0'; 
        std::string str_buffer(buffer);
        std::vector<char> result = fdClientMap[fd] -> processEvent(str_buffer);
        // 把处理结果返回
        int bytesSent = send(fd, result.data(), result.size(), 0);

        if (bytesSent == -1) {
            perror("send");
        } else {
            std::cout << "Sent " << bytesSent << " bytes to the client." << std::endl;
        }
    }
}
