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

#define FD_SETSIZE 64

fd_set Reads;
fd_set CopyReads;

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

unsigned WINAPI LoginServerThread(void* arg);

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

	/*Connect to LoginServer-----------------------------------------------------*/
	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == INVALID_SOCKET)
	{
		cout << "Fail." << endl;
		cout << "ServerSocket Error : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	SOCKADDR_IN ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	ServerSockAddr.sin_family = AF_INET;
	ServerSockAddr.sin_port = htons(8881);
	Result = inet_pton(AF_INET, "127.0.0.1", &(ServerSockAddr.sin_addr.s_addr));
	if (Result == SOCKET_ERROR)
	{
		cout << "inet_pton Error : " << GetLastError() << endl;
		system("pause");
		exit(-1);
	}

	Result = connect(ServerSocket, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));
	if (Result == SOCKET_ERROR)
	{
		if (GetLastError() == 10061)
		{
			cout << "Server Is Sleeping.." << endl;
		}
		else
		{
			cout << "connect Error : " << GetLastError() << endl;
		}
		system("pause");
		exit(-1);
	}
	else
	{
		cout << "Connect to Login Server" << endl;
	}

	_beginthreadex(nullptr, 0, LoginServerThread, (void*)&ServerSocket, 0, nullptr);

	/*---------------------------------------------------------------------------*/

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
		}
	}

	// Clean Up

	closesocket(ListenSocket);
	closesocket(ServerSocket);
	WSACleanup();

	delete Sql_Connection;

	system("pause");

	return 0;
}

// forward declare funcs
void RecvError(SOCKET& ClientSocket);
void SendError(SOCKET& ClientSocket);
void ProcessPacket(SOCKET& TargetSocket, const EPacket& PacketType, char*& Payload);

const int HeaderSize = 4;

// AS CLIENT
unsigned WINAPI LoginServerThread(void* arg)
{
	SOCKET ServerSocket = *(SOCKET*)arg;

	printf("[%d] LoginServer Thread Started\n", (unsigned short)ServerSocket);

	while (true)
	{
		// Recv Header
		char HeaderBuffer[HeaderSize] = { 0, };
		int RecvByte = recv(ServerSocket, HeaderBuffer, HeaderSize, MSG_WAITALL);
		if (RecvByte == 0 || RecvByte < 0) //close, Error
		{
			RecvError(ServerSocket);
			break;
		}

		unsigned short PayloadSize = 0;
		unsigned short PacketType = 0;

		memcpy(&PayloadSize, HeaderBuffer, 2);
		memcpy(&PacketType, &HeaderBuffer[2], 2);

		printf("[Receive] Payload size : %d, Packet type : %d\n", PayloadSize, PacketType);

		char* Payload = nullptr;

		// Recv Payload
		if (PayloadSize > 0)
		{
			Payload = new char[PayloadSize + 1];

			RecvByte = recv(ServerSocket, Payload, PayloadSize, MSG_WAITALL);
			if (RecvByte == 0 || RecvByte < 0)
			{
				//close, recv Error
				RecvError(ServerSocket);
				break;
			}
			Payload[PayloadSize] = '\0';
			cout << "Data : " << Payload << endl;
		}

		ProcessPacket(ServerSocket, static_cast<EPacket>(PacketType), Payload);

		delete[] Payload;
		Payload = nullptr;
	}

	return 0;
}

// AS SERVER
unsigned WINAPI ServerThread(void* arg)
{
	SOCKET ClientSocket = *(SOCKET*)arg;

	printf("[%d] Server Thread Started\n", (unsigned short)ClientSocket);

	bool bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_ConnectDediSuccess);
	if (!bSendSuccess)
	{
		SendError(ClientSocket);
		return 0;
	}

	while (true)
	{
		// Recv Header
		char HeaderBuffer[HeaderSize] = { 0, };
		int RecvByte = recv(ClientSocket, HeaderBuffer, HeaderSize, MSG_WAITALL);
		if (RecvByte == 0 || RecvByte < 0) //close, Error
		{
			RecvError(ClientSocket);
			break;
		}

		unsigned short PayloadSize = 0;
		unsigned short PacketType = 0;

		memcpy(&PayloadSize, HeaderBuffer, 2);
		memcpy(&PacketType, &HeaderBuffer[2], 2);

		printf("[Receive] Payload size : %d, Packet type : %d\n", PayloadSize, PacketType);

		char* Payload = nullptr;

		// Recv Payload
		if (PayloadSize > 0)
		{
			Payload = new char[PayloadSize + 1];

			RecvByte = recv(ClientSocket, Payload, PayloadSize, MSG_WAITALL);
			if (RecvByte == 0 || RecvByte < 0)
			{
				//close, recv Error
				RecvError(ClientSocket);
				break;
			}
			Payload[PayloadSize] = '\0';
			cout << "Data : " << Payload << endl;
		}

		ProcessPacket(ClientSocket, static_cast<EPacket>(PacketType), Payload);

		delete[] Payload;
		Payload = nullptr;
	}

	return 0;
}

const int MaxPlayerInDediServer = 2;

struct DediServerInfo
{
public:
	DediServerInfo() : IP(), bCanJoin(true) {}
	DediServerInfo(const string& NewIP)
		: IP(NewIP), bCanJoin(true)
	{
	}

	string IP;
	//int CurrentPlayer[MaxPlayerInDediServer] = { 0, };
	bool bCanJoin = false;
};

// Used for Manage Dedicate Server
map<SOCKET, DediServerInfo> DediServers;

SOCKET LoginTCPServer{ INVALID_SOCKET };

void RecvError(SOCKET& ClientSocket)
{
	cout << "Server Recv Error : " << GetLastError() << endl;

	SOCKADDR_IN ClientSocketAddr;
	int ClientSockAddrLength = sizeof(ClientSocketAddr);
	getpeername(ClientSocket, (SOCKADDR*)&ClientSocketAddr, &ClientSockAddrLength);

	char IP[1024] = { 0, };
	inet_ntop(AF_INET, &ClientSocketAddr.sin_addr.s_addr, IP, 1024);
	cout << "disconnected : " << IP << endl;

	/*Clean UP-----------------------------------------------------------------------------*/
	const unsigned short ClientNumber = (unsigned short)ClientSocket;
	/*-------------------------------------------------------------------------------------*/

	closesocket(ClientSocket);
	FD_CLR(ClientSocket, &Reads);
	CopyReads = Reads;
}

void SendError(SOCKET& ClientSocket)
{
	cout << "Server Send Error : " << GetLastError() << endl;
}

void ProcessPacket(SOCKET& TargetSocket, const EPacket& PacketType, char*& Payload)
{
	const unsigned short UserNumber = (unsigned short)TargetSocket;

	bool bSendSuccess = false;
	sql::PreparedStatement* Sql_PreStatement = nullptr;
	sql::ResultSet* Sql_Result = nullptr;

	// Packet for Login TCP Server
	if (PacketType >= EPacket::C2S_ReqDediTCPConnect || PacketType == EPacket::S2C_ConnectSuccess)
	{
		switch (PacketType)
		{
		case EPacket::S2C_ConnectSuccess:
		{
			bSendSuccess = PacketMaker::SendPacket(&TargetSocket, EPacket::C2S_ReqDediTCPConnect);
			if (!bSendSuccess)
			{
				SendError(TargetSocket);
				break;
			}
		}
		break;
		case EPacket::S2C_ReqAvailableDediServer:
		{
			bool bHasAvailableDediServer = false;
			for (const auto& DediServer : DediServers)
			{
				if (DediServer.second.bCanJoin)
				{
					bHasAvailableDediServer = true;

					bSendSuccess = PacketMaker::SendPacket(&TargetSocket, EPacket::C2S_ResAvailableDediServer, DediServer.second.IP.c_str());
					if (!bSendSuccess)
					{
						SendError(TargetSocket);
						break;
					}

					break;
				}
			}

			if (!bHasAvailableDediServer)
			{
				//Run in Dedi TCP
				cout << "Run Dedi Server.. ";
				int bRunSuccess = system("F:\\UnrealProjects\\AMyProject\\TCPStudy1\\Package\\Windows\\TCPStudy1\\Binaries\\Win64\\TCPStudy1ServerWithLog.exe.lnk");
				cout << ((bRunSuccess == 0) ? "Success" : "Failure") << endl;
			}
		}
		break;
		default:
			break;
		}
	}
	// Packet for Dedi Server
	else
	{
		switch (PacketType)
		{
		default:
			break;
		}
	}

	delete Sql_Result;
	delete Sql_PreStatement;
}
