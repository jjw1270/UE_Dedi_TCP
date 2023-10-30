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

// Used for User Sign Up
map<unsigned short, UserData> TempUserList;

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
	//Sql_Connection->setClientOption("charset", "utf8");

	cout << "Starting ClientLogin Server... ";

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
	ListenSockAddr.sin_port = htons(8881);

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

// forward declare funcs
void RecvError(SOCKET& ClientSocket);
void SendError(SOCKET& ClientSocket);
void ProcessPacket(SOCKET& ClientSocket, const unsigned short& UserNumber, const EPacket& PacketType, char*& Payload);

const int HeaderSize = 4;

unsigned WINAPI ServerThread(void* arg)
{
	SOCKET ClientSocket = *(SOCKET*)arg;
	unsigned short UserNumber = (unsigned short)ClientSocket;

	printf("[%d] Server Thread Started\n", UserNumber);

	bool bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_ConnectSuccess);
	if (!bSendSuccess)
	{
		SendError(ClientSocket);
		return 0;
	}

	while (true)
	{
		// Recv Header
		char HeaderBuffer[4] = { 0, };
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

		/* I Skip Network Byte Ordering because most of game devices use little endian */
		//PayloadSize = ntohs(PayloadSize);
		//PacketType = ntohs(PacketType);

		cout << "[Receive] Payload size : " << PayloadSize << ", Packet type : " << PacketType << endl;

		char* Payload = nullptr;

		// Recv Payload
		if (PayloadSize > 0)
		{
			Payload = new char[PayloadSize];

			RecvByte = recv(ClientSocket, Payload, PayloadSize, MSG_WAITALL);
			if (RecvByte == 0 || RecvByte < 0)
			{
				//close, recv Error
				RecvError(ClientSocket);
				break;
			}
			cout << "Data : " << Payload << endl;
		}

		ProcessPacket(ClientSocket, UserNumber, static_cast<EPacket>(PacketType), Payload);

		delete[] Payload;
	}

	return 0;
}

void RecvError(SOCKET& ClientSocket)
{
	cout << "Server Recv Error : " << GetLastError() << endl;

	SOCKADDR_IN ClientSocketAddr;
	int ClientSockAddrLength = sizeof(ClientSocketAddr);
	getpeername(ClientSocket, (SOCKADDR*)&ClientSocketAddr, &ClientSockAddrLength);

	char IP[1024] = { 0, };
	inet_ntop(AF_INET, &ClientSocketAddr.sin_addr.s_addr, IP, 1024);
	cout << "disconnected : " << IP << endl;

	if (TempUserList.count((unsigned short)ClientSocket) > 0)
	{
		TempUserList.erase(TempUserList.find((unsigned short)ClientSocket));
	}

	closesocket(ClientSocket);
	FD_CLR(ClientSocket, &Reads);
	CopyReads = Reads;
}

void SendError(SOCKET& ClientSocket)
{
	cout << "Server Send Error : " << GetLastError() << endl;

	//SOCKADDR_IN ClientSocketAddr;
	//int ClientSockAddrLength = sizeof(ClientSocketAddr);
	//getpeername(ClientSocket, (SOCKADDR*)&ClientSocketAddr, &ClientSockAddrLength);

	//char IP[1024] = { 0, };
	//inet_ntop(AF_INET, &ClientSocketAddr.sin_addr.s_addr, IP, 1024);
	//cout << "disconnected : " << IP << endl;

	//if (TempUserList.count((unsigned short)ClientSocket) > 0)
	//{
	//	TempUserList.erase(TempUserList.find((unsigned short)ClientSocket));
	//}

	//closesocket(ClientSocket);
	//FD_CLR(ClientSocket, &Reads);
	//CopyReads = Reads;
}

void ProcessPacket(SOCKET& ClientSocket, const unsigned short& UserNumber, const EPacket& PacketType, char*& Payload)
{
	bool bSendSuccess = false;
	sql::PreparedStatement* Sql_PreStatement = nullptr;
	sql::ResultSet* Sql_Result = nullptr;

	switch (PacketType)
	{
	case EPacket::C2S_ReqSignIn:
	{
		cout << "C2S_ReqSignIn" << endl;

		char* ColonPtr = strchr(Payload, ':');
		if (ColonPtr != nullptr)
		{
			int IDLen = ColonPtr - Payload;

			string UserID(Payload, IDLen);
			string UserPwd(ColonPtr + 1);

			cout << "ID : " << UserID << " Pwd : " << UserPwd << endl;

			// Check ID Exist in DB UserConfig
			string SqlQuery = "SELECT * FROM userconfig WHERE ID = ?";
			Sql_PreStatement = Sql_Connection->prepareStatement(SqlQuery);
			Sql_PreStatement->setString(1, UserID);
			Sql_Result = Sql_PreStatement->executeQuery();

			// If ID is valid
			if (Sql_Result->next()) {
				string dbPassword = Sql_Result->getString("Password");

				// If Password correct, Login Success!!
				if (UserPwd == dbPassword)
				{
					printf("[%d] Password Matched\n", UserNumber);

					string UserNickName = MyUtility::Utf8ToMultibyte(Sql_Result->getString("NickName"));

					bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_ResSignIn_Success, UserNickName.c_str());
					if (!bSendSuccess)
					{
						SendError(ClientSocket);
						break;
					}
				}
				else
				{
					// If Password incorrect
					bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_ResSignIn_Fail_InValidPassword);
					if (!bSendSuccess)
					{
						SendError(ClientSocket);
						break;
					}
				}
			}
			else
			{
				// else ID doesnt exist in db
				cout << "ID Does Not Exist." << endl;

				bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_ResSignIn_Fail_InValidID);
				if (!bSendSuccess)
				{
					SendError(ClientSocket);
					break;
				}
			}
		}
	}
		break;
	default:
		break;
	}

	delete Sql_Result;
	delete Sql_PreStatement;
}
