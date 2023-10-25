#include <iostream>
#include <fstream>
#include <string>
#include <process.h>

using namespace std;

#include "Packet.h"
#include "PacketMaker.h"


//--mysql
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
//--

#include "MyUtility.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32")

#define FD_SETSIZE				100

fd_set Reads;
fd_set CopyReads;

// Used for Regist User
map<unsigned short, UserData> TempUserList;

// Use when User Login
map<unsigned short, UserData> UserList;

bool GetConfigFromFile(string& OutServer, string& OutUserName, string& OutPassword)
{
	ifstream ConfigFile("config.txt");
	if (!ConfigFile.is_open())
	{
		cout << "Error: Could not open config file." << endl;
		return false;
	}

	string Line;
	size_t Start;
	size_t End;
	while (getline(ConfigFile, Line)) {
		if (Start = Line.find("Server = ") != string::npos) {
			End = Line.find('\n', Start);
			OutServer = Line.substr(Start + 8, End);		// Extract server information
		}
		else if (Start = Line.find("Username = ") != string::npos)
		{
			End = Line.find('\n', Start);
			OutUserName = Line.substr(Start + 10, End);	// Extract username
		}
		else if (Start = Line.find("Password = ") != string::npos)
		{
			End = Line.find('\n', Start);
			OutPassword = Line.substr(Start + 10, End);	// Extract password
		}
	}
	ConfigFile.close();
	return true;
}

unsigned WINAPI ServerThread(void* arg);

sql::Connection* Sql_Connection;

int main()
{
	cout << "Connecting to DB Server... ";

	string Server;
	string Username;
	string Password;
	bool GetConfigSuccess = GetConfigFromFile(Server, Username, Password);
	if (!GetConfigSuccess)
	{
		cout << "Fail." << endl;
		cout << "Get Config txt Error" << endl;
		system("pause");
		exit(-1);
	}

	sql::Driver* Sql_Driver;

	try
	{
		Sql_Driver = get_driver_instance();
		Sql_Connection = Sql_Driver->connect(Server, Username, Password);
		cout << "Done!" << endl;
	}
	catch (sql::SQLException e)
	{
		cout << "Fail." << endl;
		cout << "Could not connect to data base : " << e.what() << endl;
		system("pause");
		exit(-1);
	}

	Sql_Connection->setSchema("tcpproject");
	Sql_Connection->setClientOption("charset", "utf8");

	cout << "Starting Server... ";

	WSADATA WsaData;
	int Result = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if (Result != 0)
	{
		cout << "Fail." << endl;
		cout << "Error On StartUp : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		cout << "Fail." << endl;
		cout << "ListenSocket Error : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	ListenSockAddr.sin_port = htons(11233);

	Result = _WINSOCK2API_::bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));
	if (Result == SOCKET_ERROR)
	{
		cout << "Fail." << endl;
		cout << "Bind Error : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	Result = listen(ListenSocket, SOMAXCONN);
	if (Result == SOCKET_ERROR)
	{
		cout << "Fail." << endl;
		cout << "Listen Error : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	struct timeval Timeout;
	Timeout.tv_sec = 0;
	Timeout.tv_usec = 500;

	FD_ZERO(&Reads);
	FD_SET(ListenSocket, &Reads);

	cout << "Done!" << endl;
	cout << "Wait for Connecting... " << endl;

	while (true)
	{
		CopyReads = Reads;

		int ChangeSocketCount = select(0, &CopyReads, 0, 0, &Timeout);
		if (ChangeSocketCount > 0)
		{
			for (int i = 0; i < (int)Reads.fd_count; ++i)
			{
				if (FD_ISSET(Reads.fd_array[i], &CopyReads))
				{
					//connect
					if (Reads.fd_array[i] == ListenSocket)
					{
						SOCKADDR_IN ClientSocketAddr;
						memset(&ClientSocketAddr, 0, sizeof(ClientSocketAddr));
						int ClientSockAddrLength = sizeof(ClientSocketAddr);

						SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSocketAddr, &ClientSockAddrLength);
						if (ClientSocket == INVALID_SOCKET)
						{
							cout << "Accept Error : " << GetLastError() << endl;
							continue;
						}

						FD_SET(ClientSocket, &Reads);
						CopyReads = Reads;
						char IP[1024] = { 0, };
						inet_ntop(AF_INET, &ClientSocketAddr.sin_addr.s_addr, IP, 1024);
						printf("[%d] Connected : %s\n", (unsigned short)ClientSocket, IP);

						// create thread
						_beginthreadex(nullptr, 0, ServerThread, (void*)&ClientSocket, 0, nullptr);

						break;
					}
				}
			}
		}
		else
		{
			// when no changes on socket count while timeout
			continue;
		}
	}

	// Clean Up

	closesocket(ListenSocket);
	WSACleanup();

	delete Sql_Connection;

	system("pause");

	return 0;
}

void RecvError(SOCKET& ClientSocket)
{
	cout << "Server Recv Error : " << GetLastError() << endl;

	SOCKADDR_IN ClientSocketAddr;
	int ClientSockAddrLength = sizeof(ClientSocketAddr);
	getpeername(ClientSocket, (SOCKADDR*)&ClientSocketAddr, &ClientSockAddrLength);

	SOCKET DisconnectSocket = ClientSocket;
	closesocket(ClientSocket);
	FD_CLR(ClientSocket, &Reads);
	CopyReads = Reads;

	char IP[1024] = { 0, };
	inet_ntop(AF_INET, &ClientSocketAddr.sin_addr.s_addr, IP, 1024);
	cout << "disconnected : " << IP << endl;

	if (TempUserList.count((unsigned short)DisconnectSocket) > 0)
	{
		TempUserList.erase(TempUserList.find((unsigned short)DisconnectSocket));
	}

	string DisconnectUserNickName = UserList[(unsigned short)DisconnectSocket].NickName;

	if (UserList.count((unsigned short)DisconnectSocket) > 0)
	{
		UserList.erase(UserList.find((unsigned short)DisconnectSocket));
	}

	string BroadCastMessage = DisconnectUserNickName + " has left.";
	PacketMaker::SendPacketToAllConnectedClients(UserList, EPacket::S2C_CastMessage, BroadCastMessage.data());
}

void SendError(SOCKET& ClientSocket)
{
	cout << "Server Send Error : " << GetLastError() << endl;

	//SOCKADDR_IN ClientSocketAddr;
	//int ClientSockAddrLength = sizeof(ClientSocketAddr);
	//getpeername(ClientSocket, (SOCKADDR*)&ClientSocketAddr, &ClientSockAddrLength);

	//SOCKET DisconnectSocket = ClientSocket;
	//closesocket(ClientSocket);
	//FD_CLR(ClientSocket, &Reads);
	//CopyReads = Reads;

	//char IP[1024] = { 0, };
	//inet_ntop(AF_INET, &ClientSocketAddr.sin_addr.s_addr, IP, 1024);
	//cout << "disconnected : " << IP << endl;

	//if (TempUserList.count((unsigned short)DisconnectSocket) > 0)
	//{
	//	TempUserList.erase(TempUserList.find((unsigned short)DisconnectSocket));
	//}

	//if (UserList.count((unsigned short)DisconnectSocket) > 0)
	//{
	//	UserList.erase(UserList.find((unsigned short)DisconnectSocket));
	//}
}

unsigned WINAPI ServerThread(void* arg)
{
	SOCKET ClientSocket = *(SOCKET*)arg;
	unsigned short UserNumber = (unsigned short)ClientSocket;

	printf("[%d] Server Thread Started\n", UserNumber);


	bool bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_LoginSuccess, "hello!");
	if (!bSendSuccess)
	{
		SendError(ClientSocket);
		return 0;
	}
	
	while (true)
	{
		// Recv PacketSize
		char HeaderBuffer[4] = { 0, };
		int RecvByte = recv(ClientSocket, (char*)(&HeaderBuffer), 4, MSG_WAITALL);
		if (RecvByte == 0 || RecvByte < 0)
		{
			//close, recv Error
			RecvError(ClientSocket);
			break;
		}

		unsigned short PayloadSize;
		unsigned short PacketType;

		memcpy(&PayloadSize, HeaderBuffer, 2);
		memcpy(&PacketType, HeaderBuffer + 2, 2);

		PayloadSize = ntohs(PayloadSize);
		PacketType = ntohs(PacketType);

		cout << "Receive Header. " << PayloadSize << " " << PacketType << endl;

		if (PayloadSize > 0)
		{
			//Recv Code, Data
			char* Payload = new char[PayloadSize];

			RecvByte = recv(ClientSocket, Payload, PayloadSize, MSG_WAITALL);
			if (RecvByte == 0 || RecvByte < 0)
			{
				//close, recv Error
				RecvError(ClientSocket);
				break;
			}

			cout << "Data : " << Payload << endl;

			delete[] Payload;
		}
	}

	return 0;
}
