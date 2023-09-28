#include "ClientSession.h"
#include <cstring>

Request parseRequest(const std::string& request){
    Request result;
    std::vector<std::string> lines;
    size_t pos = 0;

    size_t lastPos = 0;
    while ((pos = request.find("\n", lastPos)) != std::string::npos) {
        lines.push_back(request.substr(lastPos, pos - lastPos));
        lastPos = pos + 1;
    }

    // 解析请求头
    std::string command, filesize;
    for (const std::string& line : lines) {
        if (line.find("Command: ") == 0) {
            result.command = line.substr(9);
        } else if (line.find("Filename: ") == 0) {
            result.filename = line.substr(10);
        } else if (line.find("Filesize: ") == 0) {
            result.filesize = std::stoi(line.substr(10));
        } else if (line.find("Data: ") == 0) {
            std::string data_str = line.substr(6);
            result.data = std::vector<char>(data_str.begin(), data_str.end());
        }
    }

    return result;
}

ClientSession::ClientSession(std::string username_) :username(username_), manager(FileSystemManager::getInstance()), currentNode(manager.getRoot()) { }

std::vector<char> ClientSession::processEvent(const std::string & message){
    Request req = parseRequest(message);
    if (req.command == "UPLOAD"){
        return uploadEvent(req);
    }else if(req.command == "DELETE"){
        return deleteEvent(req);
    }else if(req.command == "DOWNLOAD"){
        return downlaodEvent(req);
    }else if(req.command == "CD"){
        return cdEvent(req);
    }else if(req.command == "LISTDIR"){
        return listdirEvent(req);
    }else if(req.command == "MKDIR"){
        return mkdirEvent(req);
    }else{
        std::vector<char> res = {'e', 'r', 'r', 'o', 'r', ' ', 'c', 'm', 'd'};
        return res;
    }
}

std::vector<char> ClientSession::uploadEvent(const Request & req){
    try{
        manager.creatNewFile(currentNode, req.filename, username, req.data);
        return {'O', 'K'};
    }
    catch(const FileException& e){
        std::vector<char> error(e.what(), e.what() + std::strlen(e.what()));
        return error;
    }
}

std::vector<char> ClientSession::deleteEvent(const Request & req){
    try{
        manager.removeFile(currentNode, req.filename);
        return {'O', 'K'};
    }
    catch(const FileException& e){
        std::vector<char> error(e.what(), e.what() + std::strlen(e.what()));
        return error;
    }


}

std::vector<char> ClientSession::downlaodEvent(const Request & req){
    pFileNode target = currentNode -> findChild(req.filename);

    std::ifstream inFile(target -> getFullPath(), std::ios::binary);
    // if (!inFile) {
    //     std::cerr << "无法打开文件 " << fileName << " 来读取数据" << std::endl;
    //     return 1;
    // }

    inFile.seekg(0, std::ios::end);
    std::streampos fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    std::vector<char> binaryData(fileSize);
    inFile.read(binaryData.data(), fileSize);
    inFile.close();

    return binaryData;
}

std::vector<char> ClientSession::cdEvent(const Request & req){
    pFileNode target = currentNode -> findChild(req.filename);
    if (target == nullptr){
        std::string error = "The folder does not exist";
        return std::vector<char>(error.begin(), error.end());
    }
    if (target -> isFile()){
        std::string error = "This is a file, not a folder!";
        return std::vector<char>(error.begin(), error.end());
    }

    currentNode = target;
    return {'O', 'K'};
}

std::vector<char> ClientSession::mkdirEvent(const Request & req){
    try{
        manager.creatNewFolder(currentNode, req.filename);
        return {'O', 'K'};
    }
    catch(const FileException& e){
        std::vector<char> error(e.what(), e.what() + std::strlen(e.what()));
        return error;
    }
}

std::vector<char> ClientSession::listdirEvent(const Request & req){
    std::string resOutput = currentNode -> listChildren();
    return std::vector<char>(resOutput.begin(), resOutput.end());
}