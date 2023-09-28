#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "FileNode.h"
#include "FileSystemManager.h"

/*
Command: UPLOAD
Filename: example.txt
Filesize: 1024
Data: [文件二进制数据]

Command: MKDIR
Filename: example
Filesize: 0
Data: 0

Command: LISTDIR
Filename: 0
Filesize: 0
Data: 0

Command: DELETE
Filename: example.txt
Filesize: 0
Data: 0

Command: DOWNLOAD
Filename: example.txt
Filesize: 0
Data: 0

Command: CD
Filename: example
Filesize: 0
Data: 0
*/

struct Request
{
    std::string command;
    std::string filename;
    int filesize;
    std::vector<char> data;
};


Request parseRequest(const std::string& request);



class ClientSession{
public:
    ClientSession(std::string username_);

    std::vector<char> processEvent(const std::string & message);


private:
    std::string username;
    FileSystemManager & manager;
    pFileNode & currentNode;

    std::vector<char> uploadEvent(const Request & req);
    std::vector<char> deleteEvent(const Request & req);
    std::vector<char> downlaodEvent(const Request & req);
    std::vector<char> cdEvent(const Request & req);
    std::vector<char> mkdirEvent(const Request & req);
    std::vector<char> listdirEvent(const Request & req);
};

typedef std::shared_ptr<ClientSession> pClientSession;

#endif