#include "client.h"

int main(){
    int port;
    std::string addr;
    std::cout << "input your addr:" << std::endl;
    std::cin >> addr;
    std::cout << "input your port:" << std::endl;
    std::cin >> port;

    Client client(port, addr);
    client.connectServer();

    while (true){
        std::string line;
        std::getline(std::cin, line);
        client.process(line);
    }
    return 0;

}

