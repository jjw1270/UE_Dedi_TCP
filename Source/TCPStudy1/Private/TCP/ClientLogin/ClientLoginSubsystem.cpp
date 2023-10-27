// Fill out your copyright notice in the Description page of Project Settings.


#include "TCP/ClientLogin/ClientLoginSubsystem.h"
#include "TCPStudy1.h"

#include "Sockets.h"
#include "PacketMaker.h"

#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "IPAddress.h"

UClientLoginSubsystem::UClientLoginSubsystem()
{
}

void UClientLoginSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	ABLOG(Error, TEXT("Initialize ClientLoginSubsystem"));
}

void UClientLoginSubsystem::Deinitialize()
{
	ABLOG(Error, TEXT("Deinitialize ClientLoginSubsystem"));

	if (IsConnect())
	{
		DestroySocket();
	}
}

bool UClientLoginSubsystem::Connect(const int32& PortNum, const FString& IP)
{
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("TCPClientLoginSocket"), false);
	if (!Socket)
	{
		ABLOG(Error, TEXT("CreateSocket Failure"));
		return false;
	}

	FIPv4Address IPv4Address;
	FIPv4Address::Parse(IP, IPv4Address);

	TSharedPtr<FInternetAddr> SocketAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	SocketAddress->SetPort(PortNum);
	SocketAddress->SetIp(IPv4Address.Value);

	if (Socket->Connect(*SocketAddress))
	{
		ABLOG(Warning, TEXT("Connect TCP Success!"));

		return true;
	}
	else
	{
		ABLOG(Error, TEXT("Connect TCP Fail"));
		PrintSocketError(TEXT("Connect"));

		DestroySocket();

		return false;
	}
}

void UClientLoginSubsystem::DestroySocket()
{
	if (Socket)
	{
		if (Socket->GetConnectionState() == SCS_Connected)
		{
			Socket->Close();
			ABLOG(Warning, TEXT("Close Socket"));
		}

		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		ABLOG(Warning, TEXT("Destroy Socket"));

		Socket = nullptr;
		delete Socket;
	}
}

void UClientLoginSubsystem::PrintSocketError(const FString& Text)
{
	ESocketErrors SocketErrorCode = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode();
	const TCHAR* SocketError = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetSocketError(SocketErrorCode);

	UE_LOG(LogSockets, Error, TEXT("[%s]  SocketError : %s"), *Text, SocketError);
}

bool UClientLoginSubsystem::Recv(FClientLoginPacketData& OutRecvPacket)
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

		//RecvPayloadSize = ntoh(RecvPayloadSize);
		//RecvPacketType = ntoh(RecvPacketType);

		OutRecvPacket.PacketType = static_cast<EClientLoginPacket>(RecvPacketType);

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

bool UClientLoginSubsystem::IsConnect()
{
	if (Socket && (Socket->GetConnectionState() == ESocketConnectionState::SCS_Connected))
	{
		return true;
	}

	return false;
}
