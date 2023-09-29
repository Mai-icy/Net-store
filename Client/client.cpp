#include "client.h"


void Client::parseCommand(const std::string& command)
{
	std::istringstream iss(command);
    std::vector<std::string> words;
    std::string word;

    while (iss >> word) {
        words.push_back(word);
    }

	std::vector<std::string> cmd_set({"CD", "DELETE", "DOWNLOAD", "LISTDIR", "MKDIR", "UPLOAD"});
	bool isCommandVaild = false;
	for(const std::string & x:cmd_set){
		if(x == words[0]){
			isCommandVaild = true;
			break;
		}
	}

	if(!isCommandVaild || words.size() > 3)
	{
		std::cout << "INVALID COMMAND";
	}
	else
	{
		request.command = words[0];
		if (words[0] == _LISTDIR) 
		    request.filename = "0";
		else {
			request.filename = words[1];
			request.filesize = 0;
			request.data = std::vector<char>('0');
			if (words.size() == 3) { localPath = words[2]; }
			if (words[0] == _UPLOAD) {
				request.data = readFile(localPath);
				request.filesize = request.data.size();
			}
		}
	}
}

std::vector<char> Client::readFile(const std::string& path)
{
	std::ifstream fin;
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	std::vector<char> res;
	fin.open(path, std::ios::in | std::ios::binary);
	if (!fin) { std::cerr << "OPEN FILE FAILED"; }
	while (fin >> buf)
	{
		for (auto c : buf) {
			res.push_back(c);
		}
		memset(buf, 0, sizeof(buf));
	}
	return res;
}

void Client::generateMessage()
{
	std::stringstream ss;
	ss << "Command: " << request.command << "\n"
		<< "Filename: " << request.filename << "\n"
		<< "Filesize: " << request.filesize << "\n"
		<< "Data: " << &request.data[0];
	int size = ss.str().size();
	memcpy(message, ss.str().c_str(), size + 1);
}

void Client::connectServer(){
	WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return ;
    }

	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(port);
	addrSrv.sin_addr.S_un.S_addr = inet_addr(addr.c_str());

	sockClient = socket(AF_INET, SOCK_STREAM, 0);

	// std::cout << sockClient << std::endl;

	if (SOCKET_ERROR == sockClient) {
		std::cerr << "Socket() error:" << WSAGetLastError();
		return;
	}

	if (connect(sockClient, (struct  sockaddr*)&addrSrv, sizeof(addrSrv)) == INVALID_SOCKET) {
		std::cerr << "Connect failed:" << WSAGetLastError();
		return;
	}
	std::cout << "Connect Success!" << std::endl;

}

void Client::correspond()
{
	// std::cout << message << std::endl;

	send(sockClient, message, strlen(message) + 1, 0);
	// std::cout << "Wait for response" << std::endl;
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	if (request.command != _DOWNLOAD)
	{
		int bytesRead = recv(sockClient, buff, sizeof(buff), 0);
		buff[bytesRead] = '\0';
		std::cout << buff << std::endl;
		// std::cout << "Recived!" << std::endl;
	}
	else
	{
		int iResult = 0;
		std::ofstream fout;
		fout.open(localPath, std::ios::out | std::ios::binary);
		do
		{
			iResult = recv(sockClient, buff, sizeof(buff), 0);
			fout << buff;
			memset(buff, 0, sizeof(buff));
		} while (iResult > 0);
		fout.close();
	}
}

void Client::process(const std::string& command){
	parseCommand(command);
	generateMessage();
	correspond();
}
