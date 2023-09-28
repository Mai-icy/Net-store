#ifndef FILESYSTEMMANAGER_H
#define FILESYSTEMMANAGER_H

#include <map>
#include <vector>
#include "FileNode.h"
#include "dbHandle.h"

bool isValidFileName(const std::string& fileName);

class FileException : public std::exception {
public:
    FileException(const std::string& errorMessage) : errorMessage(errorMessage) {}

    const char* what() const noexcept override {
        return errorMessage.c_str();
    }
private:
    std::string errorMessage;
};


class FileSystemManager{
public:
    static FileSystemManager& getInstance(){
        static FileSystemManager instance;
        return instance;
    }
    
    bool creatNewFolder(pFileNode currentNode, std::string folderName);

    bool creatNewFile(pFileNode currentNode, const std::string &fileName, const std::string &uploader, const std::vector<char> &binaryData);

    bool removeFile(pFileNode currentNode, const std::string &fileName);

    pFileNode & getRoot(){ return root;}

private:
    DbHandle& db;
    pFileNode root;

    std::map<int, pFileNode> fileNodeMap;
    std::map<int, pFileNode> folderNodeMap;

    FileSystemManager() : db(DbHandle::getInstance()) {
        std::shared_ptr<FileNode> emptySharedPtr;
        root = std::make_shared<FileNode>("data", emptySharedPtr, 0);
        folderNodeMap[0] = root;
        biuldFileTree();
    };

    void biuldFileTree();
};


#endif