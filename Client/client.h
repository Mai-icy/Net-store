#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

const std::string _UPLOAD("UPLOAD");
const std::string _MKDIR("MKDIR");
const std::string _LISTDIR("LISTDIR");
const std::string _DELETE("DELETE");
const std::string _DOWNLOAD("DOWNLOAD");
const std::string _CD("CD");

struct Request
{
    std::string command;
    std::string filename;
    int filesize;
    std::vector<char> data;
};

class Client
{
public:
	Client(int port_, const std::string &addr_): port(port_), addr(addr_) {};
	~Client() {};

    void process(const std::string& command);
    void connectServer();

private:
    void parseCommand(const std::string& command);
    std::vector<char> readFile(const std::string& path);
    void generateMessage();
    void correspond();

    Request request;
    std::string localPath;
    SOCKET sockClient;
    char message[2000];

    int port;
    std::string addr;

};
