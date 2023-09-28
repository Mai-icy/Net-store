#include "dbHandle.h"

bool DbHandle::openDatabase(const std::string& dbPath_){
    if(db){
        return true;
    }

    dbPath = dbPath_;
    if(not isDatabaseExists()){
        // 数据库不存在，新建数据库
        if(createTable()){
            // 数据库创建成功
            return true;
        }else{
            // 数据库创建失败
            return false;
        }
    }else{
        int rc = sqlite3_open(dbPath_.c_str(), &db);
        if(rc != SQLITE_OK){
            // 错误处理
            return false;
        }else{
            return true;
        }
    }
}

bool DbHandle::closeDatabase() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
        return true;
    } else {
        return false;
    }
}

bool DbHandle::createTable(){
    int rc = sqlite3_open("mydatabase.db", &db);
    if(rc != SQLITE_OK){
        // 错误处理
        return false;
    }
    const char* createNewFloderTable = "CREATE TABLE IF NOT EXISTS Folder (\
                                FolderID INTEGER PRIMARY KEY AUTOINCREMENT,\
                                ParentFolderID INTEGER,\
                                FolderName TEXT NOT NULL,\
                                FOREIGN KEY (ParentFolderID) REFERENCES Folder(FolderID));";
    const char* createNewFileTable = "CREATE TABLE IF NOT EXISTS File (\
                                FileID INTEGER PRIMARY KEY AUTOINCREMENT,\
                                FolderID INTEGER NOT NULL,\
                                FileName TEXT NOT NULL,\
                                Uploader TEXT NOT NULL,\
                                UploadTime DATETIME NOT NULL,\
                                FOREIGN KEY (FolderID) REFERENCES Folder(FolderID));";
    
    rc = sqlite3_exec(db, createNewFloderTable, 0, 0, &errMsg);
    rc = sqlite3_exec(db, createNewFileTable, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        // 创建表失败
        sqlite3_free(errMsg);
        return false;
    } else {
        // 创建表成功 log
        return true;
    }

}

int DbHandle::insertFolder(const std::string& folderName, int parentFolderID){
    const char* insertFolderSQL = "INSERT INTO Folder (FolderName, ParentFolderID) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    if(not prepareStatement(insertFolderSQL, &stmt)) return -1;

    sqlite3_bind_text(stmt, 1, folderName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, parentFolderID);

    int rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        //执行插入语句失败: sqlite3_errmsg(db)
        sqlite3_finalize(stmt);
        return -1;
    }

    // 重置语句以备将来使用
    int lastInsertedFolderID = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    
    return lastInsertedFolderID;
}

int DbHandle::insertFile(const std::string& fileName, int folderID, const std::string& uploader, const std::string& uploadTime){
    const char* insertFileSQL = "INSERT INTO File (FileName, FolderID, Uploader, UploadTime) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if(not prepareStatement(insertFileSQL, &stmt)) return -1;

    sqlite3_bind_text(stmt, 1, fileName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, folderID);
    sqlite3_bind_text(stmt, 3, uploader.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, uploadTime.c_str(), -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        // 执行插入语句失败: sqlite3_errmsg(db_)
        sqlite3_finalize(stmt);
        return -1;
    }
    int lastInsertedFileID = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);

    return lastInsertedFileID;
}

bool DbHandle::removeFolder(const std::string& folderName, int parentFolderID){
    const char* removeFolderSQL = "DELETE FROM Folder WHERE FolderName = ? AND ParentFolderID = ?;";

    sqlite3_stmt* stmt;
    if(not prepareStatement(removeFolderSQL, &stmt)) return false;

    sqlite3_bind_text(stmt, 1, folderName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, parentFolderID);

    int rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        // "执行删除语句失败: " sqlite3_errmsg(db)
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    return true;
}

bool DbHandle::removeFolders(int parentFolderID) {
    const char* removeFoldersSQL = "DELETE FROM Folder WHERE ParentFolderID = ?;";

    sqlite3_stmt* stmt;
    if(not prepareStatement(removeFoldersSQL, &stmt)) return false;

    sqlite3_bind_int(stmt, 1, parentFolderID);

    int rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        // "执行删除语句失败: "sqlite3_errmsg(db_)
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    return true;
}

bool DbHandle::removeFile(const std::string& fileName, int fileID) {
    const char* removeFileSQL = "DELETE FROM File WHERE FileName = ? AND FolderID = ?;";
    sqlite3_stmt* stmt;
    if(not prepareStatement(removeFileSQL, &stmt)) return false;

    sqlite3_bind_text(stmt, 1, fileName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, fileID);

    int rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        // "执行删除语句失败: " sqlite3_errmsg(db_) 
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    return true;
}

bool DbHandle::prepareStatement(const char* query, sqlite3_stmt** stmt){

    int rc = sqlite3_prepare_v2(db, query, -1, stmt, nullptr);

    if (rc != SQLITE_OK) {
        // "准备删除语句失败: "sqlite3_errmsg(db_)
        return false;
    }
    return true;
}

bool DbHandle::isDatabaseExists() {
        std::ifstream file(dbPath.c_str());
        return file.good();
    }

bool DbHandle::readAllFileData(std::vector<FileData> &result) {
    const char* selectAllFile = "SELECT * FROM File;";
    sqlite3_stmt* stmt;
    if(not prepareStatement(selectAllFile, &stmt)) return false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        FileData file;
        file.fileID = sqlite3_column_int(stmt, 0);
        file.folderID = sqlite3_column_int(stmt, 1);
        file.fileName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        file.uploader = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        file.uploadTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        result.push_back(file);
    }
    
    sqlite3_finalize(stmt);

    return true;
}

bool DbHandle::readAllFolderData(std::vector<FolderData> &result) {
    const char* selectAllFolders = "SELECT * FROM Folder;";
    sqlite3_stmt* stmt;
    if(not prepareStatement(selectAllFolders, &stmt)) return false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        struct FolderData folder;
        folder.folderID = sqlite3_column_int(stmt, 0);
        folder.parentfolderID = sqlite3_column_int(stmt, 1);
        folder.floderName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        result.push_back(folder);
    }

    sqlite3_finalize(stmt);

    return true;
}
