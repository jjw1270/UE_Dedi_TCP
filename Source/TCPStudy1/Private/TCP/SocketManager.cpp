// Fill out your copyright notice in the Description page of Project Settings.


#include "SocketManager.h"
#include "TCPStudy1.h"

#include "Sockets.h"

#include "terse/utils/Endianness.h"
#include "Serialization/ArrayWriter.h"

#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "IPAddress.h"

//#include "Common/TcpSocketBuilder.h"
//#include "NetworkMessage.h"


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

//bool FSocketManager::Send(FSocket* Socket, const uint8* Buffer, int32 Size)
//{
//	while (Size > 0)
//	{
//		int32 BytesSent = 0;
//		if (!Socket->Send(Buffer, Size, BytesSent))
//		{
//			return false;
//		}
//
//		Size -= BytesSent;
//		Buffer += BytesSent;
//	}
//
//	return true;
//}

void FSocketManager::PrintSocketError(const FString& Text)
{
	ESocketErrors SocketErrorCode = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode();
	const TCHAR* SocketError = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetSocketError(SocketErrorCode);

	UE_LOG(LogSockets, Error, TEXT("[%s]  SocketError : %s"), *Text, SocketError);
}

//bool FSocketManager::SendPacket(FSocket* Socket, const EPacket& PacketType, const uint8* Payload, const int32& PayloadSize)
//{
//	FBufferArchive Buffer = (PayloadSize > 0) ? PacketMaker::MakePacket(PacketType, Payload, PayloadSize) : PacketMaker::MakePacket(PacketType);
//
//	// Send it, and make sure it sent it all
//	if (!Send(Socket, Buffer.GetData(), Buffer.Num()))
//	{
//		UE_LOG(LogTemp, Error, TEXT("Unable To Send."));
//		PrintSocketError(TEXT("[Send]"));
//		return false;
//	}
//	return true;
//}

//bool FSocketManager::Send(const FPacketData& ToSendPacket)
//{
//
//
//
//	FTCHARToUTF8 Convert(*MessageToSend);
//
//	FArrayWriter WriterArray;
//	WriterArray.Serialize((UTF8CHAR*)Convert.Get(), Convert.Length());
//
//	if (SendPacket(Socket, PacketType, WriterArray.GetData(), WriterArray.Num()))
//	{
//		UE_LOG(LogTemp, Log, TEXT("Sent Text : %s  Size : %d"), *MessageToSend, WriterArray.Num());
//	}
//}

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
