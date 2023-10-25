#include "PacketMaker.h"
#include <iostream>
#include <string>
using namespace std;

bool PacketMaker::SendPacket(SOCKET* ClientSocket, EPacket PacketToSend)
{
	pair<char*, int> BufferData = MakeBuffer(PacketToSend);

	int SendByte = send(*ClientSocket, BufferData.first, BufferData.second, 0);
	if (SendByte <= 0)
	{
		cout << "Send Error : " << GetLastError() << endl;
		return false;
	}
	return true;
}

bool PacketMaker::SendPacket(SOCKET* ClientSocket, EPacket PacketToSend, const char* MessageToSend)
{
	pair<char*, int> BufferData = MakeBuffer(PacketToSend, MessageToSend);

	int SendByte = send(*ClientSocket, BufferData.first, BufferData.second, 0);
	if (SendByte <= 0)
	{
		cout << "Send Error" << GetLastError() << endl;
		return false;
	}
	return true;
}

void PacketMaker::SendPacketToAllConnectedClients(const map<unsigned short, UserData>& UserList, EPacket PacketToSend, const char* MessageToSend)
{
	pair<char*, int> BufferData = MakeBuffer(PacketToSend, MessageToSend);

	for (const auto& UserPair : UserList)
	{
		if (UserPair.second.UserSocket == INVALID_SOCKET)
		{
			continue;
		}

		int SendByte = send(UserPair.second.UserSocket, BufferData.first, BufferData.second, 0);
		if (SendByte <= 0)
		{
			cout << "Send Error from : " << UserPair.first << ". ErrorCode : " << GetLastError() << endl;
			continue;
		}
	}
}

void PacketMaker::SendPacketToAllConnectedClients(const map<unsigned short, UserData>& UserList, EPacket PacketToSend, const char* MessageToSend, unsigned short ExcepUserNumber)
{
	pair<char*, int> BufferData = MakeBuffer(PacketToSend, MessageToSend);

	for (const auto& UserPair : UserList)
	{
		if (UserPair.second.UserSocket == INVALID_SOCKET)
		{
			continue;
		}

		if (UserPair.first != ExcepUserNumber)
		{
			int SendByte = send(UserPair.second.UserSocket, BufferData.first, BufferData.second, 0);
			if (SendByte <= 0)
			{
				cout << "Send Error from : " << UserPair.first << ". ErrorCode : " << GetLastError() << endl;
				continue;
			}
		}
	}
}

pair<char*, int> PacketMaker::MakeBuffer(EPacket Type)
{
	int BufferSize = DefaultBufferSize;
	char* Buffer = new char[BufferSize];

	MakeHeader(Buffer, Type, 0);

	return make_pair(Buffer, BufferSize);
}

pair<char*, int> PacketMaker::MakeBuffer(EPacket Type, const char* NewData)
{
	// Header       Data
	//[][][][] [Variable data]

	int PacketSize = (int)strlen(NewData);
	int BufferSize = DefaultBufferSize + PacketSize;

	char* Buffer = new char[BufferSize];
	MakeHeader(Buffer, Type, PacketSize);

	memcpy(&Buffer[4], NewData, PacketSize);

	return make_pair(Buffer, BufferSize);
}

char* PacketMaker::MakeHeader(char* Buffer, EPacket Type, unsigned short Size)
{
	// Header
	//size code
	//[][] [][]
	unsigned short size = htons(Size + 2);
	unsigned short code = htons(static_cast<unsigned short>(Type));

	memcpy(Buffer, &size, 2);
	memcpy(&Buffer[2], &code, 2);

	return Buffer;
}
