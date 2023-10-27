// Fill out your copyright notice in the Description page of Project Settings.


#include "SocketManager.h"
#include "TCPStudy1.h"

#include "PacketMaker.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "IPAddress.h"

#include "terse/utils/Endianness.h"
//#include "Serialization/ArrayWriter.h"

bool FSocketManager::Connect()
{
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("TCPClientSocket"), false);

	FString IP = TEXT("127.0.0.1");
	FIPv4Address IPv4Address;
	FIPv4Address::Parse(IP, IPv4Address);

	TSharedPtr<FInternetAddr> SocketAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	SocketAddress->SetPort(11233);
	SocketAddress->SetIp(IPv4Address.Value);

	if (Socket->Connect(*SocketAddress))
	{
		ABLOG(Log, TEXT("Connect TCP Success!"));

		return true;
	}
	else
	{
		ABLOG(Error, TEXT("Connect TCP Fail."));
		PrintSocketError(TEXT("Connect"));

		DestroySocket();

		return false;
	}
}

void FSocketManager::DestroySocket()
{
	if (Socket)
	{
		if (Socket->GetConnectionState() == SCS_Connected)
		{
			Socket->Close();
		}

		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	}
}

void FSocketManager::PrintSocketError(const FString& Text)
{
	ESocketErrors SocketErrorCode = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode();
	const TCHAR* SocketError = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetSocketError(SocketErrorCode);

	UE_LOG(LogSockets, Error, TEXT("[%s]  SocketError : %s"), *Text, SocketError);
}

bool FSocketManager::Send(const FPacketData& ToSendPacket)
{
	TArray<uint8_t> SendBuffer;
	if (ToSendPacket.Payload.IsEmpty())
	{
		ABLOG(Warning, TEXT("Payload Empty"));
		SendBuffer = PacketMaker::MakePacket(ToSendPacket.PacketType);
	}
	else
	{
		SendBuffer = PacketMaker::MakePacket(ToSendPacket.PacketType, ToSendPacket.Payload);
	}

	int32 SendBufferSize = SendBuffer.Num();
	ABLOG(Warning, TEXT("SendBuffer Size : %d"), SendBufferSize);

	int32 BytesSent = 0;
	bool bSendBuffer = Socket->Send(SendBuffer.GetData(), SendBufferSize, BytesSent);

	if (!bSendBuffer || BytesSent != SendBufferSize)
	{
		PrintSocketError(TEXT("Send"));
		return false;
	}
	
	return true;
}

bool FSocketManager::Recv(FPacketData& OutRecvPacket)
{
	if (!Socket)
	{
		ABLOG(Error, TEXT("Socket is null"));
		return false;
	}

	uint32 PendingDataSize = 0;
	if (Socket->HasPendingData(PendingDataSize))
	{
		// ABLOG(Log, TEXT("PendingDataSize : %u"), PendingDataSize);

		// Header Size : 4 bytes
		TArray<uint8_t> HeaderBuffer;
		HeaderBuffer.AddZeroed(HeaderSize);

		// Recv Header
		int BytesRead = 0;
		bool bRecvHeader = Socket->Recv(HeaderBuffer.GetData(), HeaderSize, BytesRead);

		if (!bRecvHeader || BytesRead != HeaderSize)
		{
			PrintSocketError(TEXT("Receive Header"));
			return false;
		}

		uint16 RecvPayloadSize;
		uint16 RecvPacketType;

		// Get Size and Type from HeaderBuffer
		FMemory::Memcpy(&RecvPayloadSize, HeaderBuffer.GetData(), sizeof(uint16_t));
		FMemory::Memcpy(&RecvPacketType, HeaderBuffer.GetData() + sizeof(uint16_t), sizeof(uint16_t));

		RecvPayloadSize = ntoh(RecvPayloadSize);
		RecvPacketType = ntoh(RecvPacketType);

		OutRecvPacket.PacketType = static_cast<EPacket>(RecvPacketType);

		// Recv Payload
		if (RecvPayloadSize > 0)
		{
			TArray<uint8_t> PayloadBuffer;
			PayloadBuffer.AddZeroed(RecvPayloadSize);

			BytesRead = 0;
			bool bRecvPayload = Socket->Recv(PayloadBuffer.GetData(), RecvPayloadSize, BytesRead);

			if (!bRecvPayload || BytesRead != RecvPayloadSize)
			{
				PrintSocketError(TEXT("Receive Payload"));
				return false;
			}

			//Utf8 to FStirng
			FString PayloadString;
			PayloadString = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(PayloadBuffer.GetData())));

			OutRecvPacket.Payload = PayloadString;
		}

		return true;
	}
	else
	{
		return false;
	}
}
