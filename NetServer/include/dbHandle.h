#ifndef DBHANDLE_H
#define DBHANDLE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <fstream>

struct FileData
{
    int fileID;
    int folderID;
    std::string fileName;
    std::string uploader;
    std::string uploadTime;
};

struct FolderData
{
    int folderID;
    int parentfolderID;
    std::string floderName;
};


class DbHandle {
public:
    static DbHandle& getInstance(){
        static DbHandle instance; // 在首次调用时创建唯一的实例
        return instance;
    }

    bool openDatabase(const std::string& dbPath_);
    bool closeDatabase();
    
    bool createTable();

    int insertFolder(const std::string& folderName, int parentFolderID);
    int insertFile(const std::string& fileName, int folderID, const std::string& uploader, const std::string& uploadTime);

    bool removeFolder(const std::string& folderName, int parentFolderID);
    bool removeFolders(int parentFolderID);
    bool removeFile(const std::string& fileName, int folderID);

    bool readAllFileData(std::vector<FileData> &result);
    bool readAllFolderData(std::vector<FolderData> &result);

private:
    std::string dbPath;
    sqlite3* db;
    char* errMsg;
    
    DbHandle() : dbPath(""), db(nullptr){ };

    bool prepareStatement(const char* query, sqlite3_stmt** stmt);
    bool isDatabaseExists();
};

#endif