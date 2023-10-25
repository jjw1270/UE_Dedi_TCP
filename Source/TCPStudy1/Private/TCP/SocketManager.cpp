// Fill out your copyright notice in the Description page of Project Settings.


#include "SocketManager.h"
#include "Sockets.h"
#include "terse/utils/Endianness.h"
#include "Serialization/ArrayWriter.h"
#include "Common/TcpSocketBuilder.h"
#include "NetworkMessage.h"
#include "TCP/PacketMaker.h"

bool SocketManager::Connect(FSocket* Socket, FString IPAddress, int PortNumber)
{
	FIPv4Address IPAddr;
	if (!FIPv4Address::Parse(IPAddress, IPAddr))
	{
		UE_LOG(LogTemp, Error, TEXT("Ill-formed IP Address"));
		return false;
	}

	FIPv4Endpoint Endpoint = FIPv4Endpoint(IPAddr, PortNumber);

	if (Socket->Connect(*Endpoint.ToInternetAddr()))
	{
		return true;
	}
	else
	{
		PrintSocketError(TEXT("[Connect]"));
		return false;
	}
}

// If data size is too big for just one recv, it needs to be called multi times.
bool SocketManager::Receive(FSocket* Socket, uint8* Results, int32 Size)
{
	int32 Offset = 0;
	while (Size > 0)
	{
		int32 NumRead = 0;
		Socket->Recv(Results + Offset, Size, NumRead);
		check(NumRead <= Size);
		// make sure we were able to read at least something (and not too much)
		if (NumRead <= 0)
		{
			return false;
		}

		// if we read a partial block, move along
		Offset += NumRead;
		Size -= NumRead;
	}
	return true;
}

bool SocketManager::Send(FSocket* Socket, const uint8* Buffer, int32 Size)
{
	while (Size > 0)
	{
		int32 BytesSent = 0;
		if (!Socket->Send(Buffer, Size, BytesSent))
		{
			return false;
		}

		Size -= BytesSent;
		Buffer += BytesSent;
	}

	return true;
}

bool SocketManager::SendPacket(FSocket* Socket, const EPacket& PacketType, const uint8* Payload, const int32& PayloadSize)
{
	FBufferArchive Buffer = (PayloadSize > 0) ? PacketMaker::MakePacket(PacketType, Payload, PayloadSize) : PacketMaker::MakePacket(PacketType);

	// Send it, and make sure it sent it all
	if (!Send(Socket, Buffer.GetData(), Buffer.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("Unable To Send."));
		PrintSocketError(TEXT("[Send]"));
		return false;
	}
	return true;
}

bool SocketManager::ReceivePacket(FSocket* Socket, TArray<uint8>& OutPayload)
{
	// It doesnt work, I dont know reason..
	//const int32 MaxPacketSize = 1024 * 1024;  // 1MB
	//uint32 PendingDataSize;

	//// If socket has pending data or PendingDataSize <= MaxPacketSize, Recv
	//if (!Socket->HasPendingData(PendingDataSize) || PendingDataSize <= MaxPacketSize)
	//{
	//	return false;
	//}

	// Header Size is Fixed 4
	TArray<uint8> HeaderBuffer;
	HeaderBuffer.AddZeroed(4);

	// Recv header
	int32 BytesRead = 0;
	if (!Receive(Socket, HeaderBuffer.GetData(), HeaderBuffer.Num()))
	{
		//PrintSocketError(TEXT("[Receive Header]"));
		return false;
	}

	uint16 RecvPayloadSize;
	uint16 RecvPacketType;

	// Get Size and Type from HeaderBuffer
	FMemory::Memcpy(&RecvPayloadSize, HeaderBuffer.GetData(), sizeof(uint16));
	FMemory::Memcpy(&RecvPacketType, HeaderBuffer.GetData() + sizeof(uint16), sizeof(uint16));

	RecvPayloadSize = ntoh(RecvPayloadSize);
	RecvPacketType = ntoh(RecvPacketType);

	// UE_LOG(LogTemp, Warning, TEXT("Received Payload Size: %d, Packet Type: %d"), RecvPayloadSize, RecvPacketType);

	// Recv Payload
	if (RecvPayloadSize > 0)
	{
		OutPayload.SetNumZeroed(RecvPayloadSize);

		if (!Receive(Socket, OutPayload.GetData(), OutPayload.Num()))
		{
			return false;
		}
	}

	return true;
}

void SocketManager::PrintSocketError(const FString& Text)
{
	ESocketErrors SocketErrorCode = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode();
	const TCHAR* SocketError = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetSocketError(SocketErrorCode);

	UE_LOG(LogSockets, Error, TEXT("%s  SocketError : %s"), *Text, SocketError);
}

void SocketManager::Connect()
{
	int32 RecvBufferSize = 2 * 1024 * 1024;

	Socket = FTcpSocketBuilder(TEXT("ClientSocket"));
	Socket->SetNonBlocking(true);

	Socket->SetReceiveBufferSize(RecvBufferSize, RecvBufferSize);

	FString IPAddress = TEXT("127.0.0.1");
	uint16 PortNumber = 11233;

	if (Connect(Socket, IPAddress, PortNumber))
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket Connected"));
	}
	else
	{
		DestroySocket();
	}
}

void SocketManager::DestroySocket()
{
	if (Socket)
	{
		if (Socket->GetConnectionState() == SCS_Connected)
		{
			Socket->Close();
		}

		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get();
		SocketSubsystem->DestroySocket(Socket);

		Socket = nullptr;
		delete Socket;
	}
}

void SocketManager::Send(const EPacket& PacketType, const FString& MessageToSend)
{
	SCOPE_CYCLE_COUNTER(STAT_Send)
		FTCHARToUTF8 Convert(*MessageToSend);

	FArrayWriter WriterArray;
	WriterArray.Serialize((UTF8CHAR*)Convert.Get(), Convert.Length());

	if (SendPacket(Socket, PacketType, WriterArray.GetData(), WriterArray.Num()))
	{
		UE_LOG(LogTemp, Log, TEXT("Sent Text : %s  Size : %d"), *MessageToSend, WriterArray.Num());
	}
}

void SocketManager::Recv()
{
	SCOPE_CYCLE_COUNTER(STAT_Recv)
		TArray<uint8> Payload;

	if (ReceivePacket(Socket, Payload))
	{
		FString Data(Payload.Num(), (char*)Payload.GetData());
		UE_LOG(LogTemp, Error, TEXT("Recv data success!!!  data : %s  size : %d"), *Data, Data.Len());
	}
}
