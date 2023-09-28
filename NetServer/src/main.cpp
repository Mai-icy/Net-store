#include <iostream>
#include <sqlite3.h>
#include <dbHandle.h>
#include "FileSystemManager.h"
#include "ClientSession.h"
#include "Server.h"
#include <pthread.h>

int main(){
    using namespace std;

    DbHandle& instance = DbHandle::getInstance();
    instance.openDatabase("example.db");

    int ports;
    cout << "input your ports:" << endl;
    cin >> ports;
    TCPServer svr(ports);
    svr.run();
    
    instance.closeDatabase();
    return 0;

}






// int main() {
//     DbHandle& instance = DbHandle::getInstance();
//     instance.openDatabase("example.db");
//     FileSystemManager& fileSystem = FileSystemManager::getInstance();

//     ClientSession client("MAI");

//     while(true){
//         std::string text;
//         for (int i = 0; i < 4; ++i) {
//             std::string line;
//             std::getline(std::cin, line);
//             text += line + '\n';
//         }
//         if(text == "END\nEND\nEND\nEND\n"){
//             break;
//         }
//         std::cout << "received!" << std::endl;

//         std::vector<char> res = client.processEvent(text);

//         std::cout << "process done!" << std::endl;
//         std::string str(res.begin(), res.end());

//         std::cout << str << std::endl;
//     }   

//     instance.closeDatabase();
//     return 0;
// }
