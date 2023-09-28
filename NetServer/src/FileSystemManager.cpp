#include "FileSystemManager.h"
#include <memory>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <chrono>
#include <ctime>
#include <iomanip>

typedef std::shared_ptr<FileNode> pFileNode;




bool isValidFileName(const std::string& fileName) {
    if (fileName.empty()) {
        return false;
    }
    if (fileName.length() > 255) {
        return false;
    }

    // 检查文件名中的非法字符
    static const char* illegalChars = "\\/:*?\"<>|";
    if (fileName.find_first_of(illegalChars) != std::string::npos) {
        return false;
    }

    // 检查文件名是否以空格开头或结尾
    if (std::isspace(fileName.front()) || std::isspace(fileName.back())) {
        return false;
    }
    return true;
}

std::string getCurrentTimeFormatted() {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* timeInfo = std::localtime(&currentTime);
    std::stringstream ss;
    ss << std::put_time(timeInfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool FileSystemManager::creatNewFolder(pFileNode currentNode, std::string folderName){
    if (not isValidFileName(folderName)){
        throw FileException("Invalid file name");
    }
    if (currentNode -> findChild(folderName)){
        throw FileException("The folder already exists");
    }

    
    int status = mkdir((currentNode -> getFullPath() + '/' + folderName).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (status != 0) {
        throw FileException("Folder creation failure");
    }

    int new_id = db.insertFolder(folderName, currentNode->getFileID());
    pFileNode newFile = std::make_shared<FileNode>(folderName, currentNode, new_id);
    currentNode->addChild(newFile);

    return true;
}

bool FileSystemManager::creatNewFile(pFileNode currentNode, const std::string &fileName, const std::string &uploader, const std::vector<char> &binaryData){
    if (not isValidFileName(fileName)){
        throw FileException("Invalid file name");
    }
    if (currentNode -> findChild(fileName)){
        throw FileException("The file already exists");
    }
    
    std::ofstream outFile((currentNode -> getFullPath() + '/' + fileName).c_str(), std::ios::binary);
    if(!outFile){
        throw FileException("File write error");
    }
    outFile.write(binaryData.data(), binaryData.size());
    outFile.close();

    std::string nowTime = getCurrentTimeFormatted();
    int new_id = db.insertFile(fileName, currentNode->getFileID(), uploader, nowTime);
    pFileNode newFile = std::make_shared<FileNode>(fileName, currentNode, uploader,nowTime, new_id);
    currentNode->addChild(newFile);
    return true;
}

bool FileSystemManager::removeFile(pFileNode currentNode, const std::string &fileName){
    pFileNode it = currentNode -> findChild(fileName);
    if(it == nullptr){
        throw FileException("File does not exist");
    }

    if(it->isFile()){
        int res = std::remove(it -> getFullPath().c_str());
        if(res != 0) throw FileException("File remove error");
        db.removeFile(fileName, currentNode -> getFileID());
    }else{
        int res = rmdir(it -> getFullPath().c_str());
        if(res != 0) throw FileException("Folder remove error");
        db.removeFolder(fileName, currentNode -> getFileID());
    }

    currentNode -> removeChild(fileName);
    return true;
}

void FileSystemManager::biuldFileTree(){
    std::vector<FolderData> folders;
    db.readAllFolderData(folders);
    for(const FolderData & it: folders){
        pFileNode newFile = std::make_shared<FileNode>(it.floderName, folderNodeMap[it.parentfolderID], it.folderID);
        folderNodeMap[it.folderID] = newFile;
        folderNodeMap[it.parentfolderID] -> addChild(newFile);
    }
    
    std::vector<FileData> files;
    db.readAllFileData(files);
    for(const FileData & it: files){
        pFileNode newFile = std::make_shared<FileNode>(it.fileName, folderNodeMap[it.folderID], it.uploader, it.uploadTime, it.fileID);
        fileNodeMap[it.folderID] = newFile;
        folderNodeMap[it.folderID] -> addChild(newFile);
    }
}
