#include "client.h"

void Client::parseCommand(const std::string& command)
{
	Request result;
	std::vector<std::string> words;
	std::string word("");
	for (auto c : command)
	{
		if (c == ' ')
		{
			words.push_back(word);
			word.clear();
		}
		word.append(1, c);
	}


	if(words[0].find(_CD+_DELETE+_DOWNLOAD+_LISTDIR+_MKDIR+_UPLOAD) == std::string::npos || words.size() > 3)
	{
		std::cerr << "INVALID COMMAND";
	}
	else
	{
		result.command = words[0];
		if (words[0] == _LISTDIR) { result.filename = "0"; }
		else {
			result.filename = words[1];
			result.filesize = 0;
			result.data = std::vector < char>('0');
			if (words.size() == 2) { localPath = words[2]; }
			if (words[0] == _UPLOAD) {
				result.data = readFile(localPath);
				result.filesize = result.data.size();
			}
		}
	}

	request = result;
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

char* Client::generateMessage()
{
	char* message;
	std::stringstream ss;
	ss << "Command: " << request.command << "\n"
		<< "Filename: " << request.filename << "\n"
		<< "Filesize: " << request.filesize << "\n"
		<< "Data: " << &request.data[0];
	int size = ss.str().size();
	message = new char[size + 1];
	memcpy(message, ss.str().c_str(), size + 1);
	return message;
}

void Client::correspond(char* message)
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "Failed to load Winsock";
		return;
	}

	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(port);
	addrSrv.sin_addr.S_un.S_addr = inet_addr(addr.c_str());

	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == sockClient) {
		std::cerr << "Socket() error:" << WSAGetLastError();
		return;
	}

	if (connect(sockClient, (struct  sockaddr*)&addrSrv, sizeof(addrSrv)) == INVALID_SOCKET) {
		std::cerr << "Connect failed:" << WSAGetLastError();
		return;
	}

	send(sockClient, message, strlen(message) + 1, 0);

	char buff[1024];
	memset(buff, 0, sizeof(buff));
	if (request.command != _DOWNLOAD)
	{
		recv(sockClient, buff, sizeof(buff), 0);
		std::cout << buff << std::endl;
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

	closesocket(sockClient);
	WSACleanup();
	delete[] message;
}

// void Client::run(){
// 	std::string line;
//     std::getline(std::cin, line);

// }
