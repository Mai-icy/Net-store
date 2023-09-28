#ifndef FILENODE_H
#define FILENODE_H

#include <memory> 
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
class FileNode;

typedef std::shared_ptr<FileNode> pFileNode;

class FileNode {
public:
    FileNode(const std::string& name, pFileNode parent, int id_) : name(name), isFolder(true), parentNode(parent), fileID(id_) { }
    FileNode(const std::string& name, pFileNode parent, std::string uploader_, std::string uploadTime_, int id_) 
            : name(name), uploader(uploader_), uploadTime(uploadTime_), isFolder(false), parentNode(parent), fileID(id_) { };

    void addChild(pFileNode child) {
        children.push_back(child);
    }

    void removeChild(const std::string& childName) {
        auto it = std::remove_if(children.begin(), children.end(),
            [childName](pFileNode child) { return child->name == childName; });
        children.erase(it, children.end());
    }

    pFileNode findChild(const std::string& childName) {
        for (pFileNode child : children) {
            if (child->name == childName) {
                return child;
            }
        }
        return nullptr;
    }

    std::string listChildren() {
        std::string result;
        for (pFileNode child : children) {
            result += (child->isFolder ? "Folder: " : "File: ") + child->name + "\n";
        }
        return result;
    }

    std::string getFullPath() const {
        if (parentNode) {
            return parentNode->getFullPath() + "/" + name;
        } else {
            return name;
        }
    }

    int getFileID(){ return fileID;}

    bool isFile(){ return !isFolder;}
private:
    std::string name;
    std::string uploader;
    std::string uploadTime;

    int fileID;
    bool isFolder;
    pFileNode parentNode;
    std::vector<pFileNode> children;
};

#endif