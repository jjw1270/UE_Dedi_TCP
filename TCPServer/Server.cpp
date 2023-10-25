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
	ListenSockAddr.sin_port = htons(7871);

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

	// send req login
	bool bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_Login_UserIDReq);
	if (!bSendSuccess)
	{
		SendError(ClientSocket);
	}
	else
	{
		while (true)
		{
			// Recv PacketSize
			unsigned short PacketSize = 0;
			int RecvByte = recv(ClientSocket, (char*)(&PacketSize), 2, MSG_WAITALL);
			if (RecvByte == 0 || RecvByte < 0)
			{
				//close, recv Error
				RecvError(ClientSocket);
				break;
			}

			//Recv Code, Data
			PacketSize = ntohs(PacketSize);
			char* Buffer = new char[PacketSize];

			RecvByte = recv(ClientSocket, Buffer, PacketSize, MSG_WAITALL);
			if (RecvByte == 0 || RecvByte < 0)
			{
				//close, recv Error
				RecvError(ClientSocket);
				break;
			}

			//code 
			//[][]
			unsigned short Code = 0;
			memcpy(&Code, Buffer, 2);
			Code = ntohs(Code);

			// Data
			unsigned short DataSize = PacketSize - 2;
			bool bSendSuccess = false;
			switch ((EPacket)Code)
			{
			case EPacket::C2S_Login_UserIDAck:
			{
				char UserID[100] = { 0, };
				memcpy(&UserID, Buffer + 2, DataSize);

				// Check ID Exist in DB
				string SqlQuery = "SELECT * FROM userconfig WHERE ID = ?";
				sql::PreparedStatement* Sql_PreStatement = Sql_Connection->prepareStatement(SqlQuery);
				Sql_PreStatement->setString(1, UserID);
				sql::ResultSet* Sql_Result = Sql_PreStatement->executeQuery();

				// If ID doesnt exist in db
				if (Sql_Result->rowsCount() == 0)
				{
					//cout << "ID Does Not Exist." << endl;

					// check TempUser already exist in TempUserList
					if (TempUserList.count(UserNumber) > 0)
					{
						TempUserList[UserNumber].UserID = UserID;

						bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_Login_UserIDFailureReq);
						if (!bSendSuccess)
						{
							SendError(ClientSocket);
							break;
						}
					}
					else
					{
						//cout << "Make new Temp User" << endl;
						UserData NewTempUser(UserID);
						TempUserList.emplace(UserNumber, NewTempUser);

						bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_Login_UserIDFailureReq);
						if (!bSendSuccess)
						{
							SendError(ClientSocket);
							break;
						}
					}
				}
				else
				{
					// Confirm ID Success
					printf("[%d] User Login Requested : %s\n", UserNumber, UserID);

					// check User already exist in UserList
					if (UserList.count(UserNumber) > 0)
					{
						UserList[UserNumber].UserID = UserID;
					}
					else
					{
						// cout << "Make new User" << endl;
						UserData NewUser(UserID);
						UserList.emplace(UserNumber, NewUser);
					}

					bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_Login_UserPwdReq);
					if (!bSendSuccess)
					{
						SendError(ClientSocket);
						break;
					}

					// delete temp user from list
					if (TempUserList.count(UserNumber > 0))
					{
						TempUserList.erase(TempUserList.find(UserNumber));
					}
				}

				delete Sql_Result;
				delete Sql_PreStatement;
			}
			break;
			case EPacket::C2S_Login_MakeNewUserReq:
			{
				printf("[%d] Make New User Requested\n", UserNumber);

				bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_Login_NewUserNickNameReq);
				if (!bSendSuccess)
				{
					SendError(ClientSocket);
					break;
				}
			}
			break;
			case EPacket::C2S_Login_UserIDReq:
			{
				//cout << "C2S_Login_UserIDReq" << endl;
				bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_Login_UserIDReq);
				if (!bSendSuccess)
				{
					SendError(ClientSocket);
					break;
				}
			}
			break;
			case EPacket::C2S_Login_NewUserNickNameAck:
			{
				char UserNickName[100] = { 0, };
				memcpy(&UserNickName, Buffer + 2, DataSize);

				printf("[%d] New User NickName : %s\n", UserNumber, UserNickName);

				// Check NickName already exist in db (NickName cant overlaped)
				string SqlQuery = "SELECT * FROM userconfig WHERE NickName = ?";
				sql::PreparedStatement* Sql_PreStatement = Sql_Connection->prepareStatement(SqlQuery);
				Sql_PreStatement->setString(1, MyUtility::MultibyteToUtf8(UserNickName));
				sql::ResultSet* Sql_Result = Sql_PreStatement->executeQuery();

				if (Sql_Result->rowsCount() > 0)
				{
					//cout << "NickName Already Exist" << endl;
					bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_CastMessage, "Nick Name Already Exist.");
					if (!bSendSuccess)
					{
						SendError(ClientSocket);
						break;
					}
					bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_Login_NewUserNickNameReq);
					if (!bSendSuccess)
					{
						SendError(ClientSocket);
						break;
					}
				}
				else
				{
					TempUserList[UserNumber].NickName = UserNickName;
					bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_Login_NewUserPwdReq);
					if (!bSendSuccess)
					{
						SendError(ClientSocket);
						break;
					}
				}

				delete Sql_Result;
				delete Sql_PreStatement;
			}
			break;
			case EPacket::C2S_Login_NewUserPwdAck:
			{
				char NewUserPwd[100] = { 0, };
				memcpy(&NewUserPwd, Buffer + 2, DataSize);

				// cout << "New User password : " << NewUserPwd << endl;

				// Create New Userconfig
				string SqlQuery = "INSERT INTO userconfig(ID, Password, NickName) VALUES(?,?,?)";
				sql::PreparedStatement* Sql_PreStatement = Sql_Connection->prepareStatement(SqlQuery);
				Sql_PreStatement->setString(1, TempUserList[UserNumber].UserID);
				Sql_PreStatement->setString(2, NewUserPwd);
				Sql_PreStatement->setString(3, MyUtility::MultibyteToUtf8(TempUserList[UserNumber].NickName));
				Sql_PreStatement->execute();

				printf("[%d] New User Registed\n", UserNumber);
				bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_CastMessage, "<< New User Registed! >>");
				if (!bSendSuccess)
				{
					SendError(ClientSocket);
					break;
				}
				bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_Login_UserIDReq);
				if (!bSendSuccess)
				{
					SendError(ClientSocket);
					break;
				}}
			break;
			case EPacket::C2S_Login_UserPwdAck:
			{
				char UserPwd[100] = { 0, };
				memcpy(&UserPwd, Buffer + 2, DataSize);

				// cout << "User password : " << UserPwd << endl;

				// Check Password
				string SqlQuery = "SELECT * FROM userconfig WHERE ID = ?";
				sql::PreparedStatement* Sql_PreStatement = Sql_Connection->prepareStatement(SqlQuery);
				Sql_PreStatement->setString(1, UserList[UserNumber].UserID);
				sql::ResultSet* Sql_Result = Sql_PreStatement->executeQuery();

				if (Sql_Result->next()) {
					string dbPassword = Sql_Result->getString("Password");

					if (strcmp(UserPwd, dbPassword.c_str()) == 0)
					{
						// if correct
						printf("[%d] Password Matched\n", UserNumber);

						string UserNickName = MyUtility::Utf8ToMultibyte(Sql_Result->getString("NickName"));

						// Check Login Overlaping
						bool bIsLoginOverlap = false;
						for (const auto& UserPair : UserList)
						{
							if (UserPair.second.NickName == UserNickName)
							{
								bIsLoginOverlap = true;
								break;
							}
						}

						if (bIsLoginOverlap)
						{
							printf("[%d] Login Overlapped\n", UserNumber);
							// Login Overlapped
							bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_CastMessage, "You are already logged in.");
							if (!bSendSuccess)
							{
								SendError(ClientSocket);
								break;
							}

							bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_Login_UserIDReq);
							if (!bSendSuccess)
							{
								SendError(ClientSocket);
								break;
							}

							break;
						}
						else
						{
							printf("[%d] Login Success!\n", UserNumber);
							// Login Success
							UserList[UserNumber].UserSocket = ClientSocket;
							UserList[UserNumber].NickName = UserNickName;

							bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_LoginSuccess);
							if (!bSendSuccess)
							{
								SendError(ClientSocket);
								break;
							}

							bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_CanChat);
							if (!bSendSuccess)
							{
								SendError(ClientSocket);
								break;
							}

							string BroadCastMessage = UserList[UserNumber].NickName + " is here!";
							PacketMaker::SendPacketToAllConnectedClients(UserList, EPacket::S2C_CastMessage, BroadCastMessage.data(), UserNumber);
						}
					}
					else
					{
						// else, Ask Re-Enter Pwd or Re-Enter ID
						printf("[%d] Password Failure\n", UserNumber);

						bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_Login_UserPwdFailureReq);
						if (!bSendSuccess)
						{
							SendError(ClientSocket);
							break;
						}
					}
				}
				else
				{
					// DB Error, this will never be executed
					RecvError(ClientSocket);
					// cout << "User not found in the database." << endl;
				}

				delete Sql_Result;
				delete Sql_PreStatement;
			}
			break;
			case EPacket::C2S_Login_UserPwdReq:
			{
				//cout << "C2S_Login_UserPwdReq" << endl;
				bSendSuccess = PacketMaker::SendPacket(&ClientSocket, EPacket::S2C_Login_UserPwdReq);
				if (!bSendSuccess)
				{
					SendError(ClientSocket);
					break;
				}
			}
			break;
			case EPacket::C2S_Chat:
			{
				char RecvChat[1024] = { 0, };
				memcpy(&RecvChat, Buffer + 2, DataSize);

				printf("[%d] Send Chat : %s\n", UserNumber, RecvChat);

				string ChatUserNickName = UserList[UserNumber].NickName;

				string BroadCastMessage = ChatUserNickName + " : " + RecvChat;
				PacketMaker::SendPacketToAllConnectedClients(UserList, EPacket::S2C_Chat, BroadCastMessage.data(), UserNumber);

				// Add to User Chatting Log
				string SqlQuery = "INSERT INTO chatlog(NickName, Chat) VALUES(?,?)";
				sql::PreparedStatement* Sql_PreStatement = Sql_Connection->prepareStatement(SqlQuery);
				Sql_PreStatement->setString(1, MyUtility::MultibyteToUtf8(ChatUserNickName));
				Sql_PreStatement->setString(2, MyUtility::MultibyteToUtf8(RecvChat));
				Sql_PreStatement->execute();

				delete Sql_PreStatement;
			}
			break;
			default:
				break;
			}
			delete[] Buffer;
		}
	}

	return 0;
}
