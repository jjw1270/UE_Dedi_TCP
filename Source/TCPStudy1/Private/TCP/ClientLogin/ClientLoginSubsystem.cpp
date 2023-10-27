// Fill out your copyright notice in the Description page of Project Settings.


#include "TCP/ClientLogin/ClientLoginSubsystem.h"
#include "TCPStudy1.h"

#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "IPAddress.h"
//#include "terse/utils/Endianness.h"

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

bool UClientLoginSubsystem::Recv(FLoginPacketData& OutRecvPacket)
{
	if (!Socket)
	{
		ABLOG(Error, TEXT("Socket is null"));
		return false;
	}

	if (Socket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromSeconds(5)))
	{
		TArray<uint8_t> HeaderBuffer;
		HeaderBuffer.AddZeroed(HeaderSize);

		// Recv Header
		int BytesRead = 0;
		bool bRecvHeader = Socket->Recv(HeaderBuffer.GetData(), HeaderSize, BytesRead);
		if (!bRecvHeader)
		{
			PrintSocketError(TEXT("Receive Header"));
			return false;
		}

		uint16 RecvPayloadSize;
		uint16 RecvPacketType;

		// Get Size and Type from HeaderBuffer
		FMemory::Memcpy(&RecvPayloadSize, HeaderBuffer.GetData(), sizeof(uint16_t));
		FMemory::Memcpy(&RecvPacketType, HeaderBuffer.GetData() + sizeof(uint16_t), sizeof(uint16_t));

		/* I Skip Network Byte Ordering because most of game devices use little endian */
		//RecvPayloadSize = ntoh(RecvPayloadSize);
		//RecvPacketType = ntoh(RecvPacketType);

		OutRecvPacket.PacketType = static_cast<ELoginPacket>(RecvPacketType);

		// Recv Payload
		if (RecvPayloadSize > 0)
		{
			TArray<uint8_t> PayloadBuffer;
			PayloadBuffer.AddZeroed(RecvPayloadSize);

			BytesRead = 0;
			bool bRecvPayload = Socket->Recv(PayloadBuffer.GetData(), RecvPayloadSize, BytesRead);

			if (!bRecvPayload)
			{
				PrintSocketError(TEXT("Receive Payload"));
				return false;
			}

			//Utf8 to FStirng
			FString PayloadString;
			PayloadString = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(PayloadBuffer.GetData())));

			OutRecvPacket.Payload = PayloadString;
		}

		ABLOG(Warning, TEXT("[Recv] PacketType : %d, PayloadSize : %d"), RecvPacketType, RecvPayloadSize);

		return true;
	}
	else
	{
		return false;
	}
}

bool UClientLoginSubsystem::Send(const FLoginPacketData& SendPacket)
{
	if (!Socket)
	{
		ABLOG(Error, TEXT("Socket is null"));
		return false;
	}

	// Send Header
	uint16_t PayloadSize = SendPacket.Payload.Len();
	ABLOG(Warning, TEXT("Payload Size : %d"), (int32)PayloadSize);

	const uint16_t Type = StaticCast<uint16_t>(SendPacket.PacketType);

	uint8_t HeaderBuffer[HeaderSize] = { 0, };

	FMemory::Memcpy(&HeaderBuffer, &PayloadSize, 2);
	FMemory::Memcpy(&HeaderBuffer[2], &Type, 2);

	int32 BytesSent = 0;
	bool bSendBuffer = Socket->Send(HeaderBuffer, HeaderSize, BytesSent);
	if (!bSendBuffer)
	{
		PrintSocketError(TEXT("Send"));
		return false;
	}
	
	ABLOG(Warning, TEXT("Send Header Success"));

	if (PayloadSize > 0)
	{
		uint8_t* PayloadBuffer = new uint8_t[PayloadSize];

		StringToBytes(SendPacket.Payload, PayloadBuffer, PayloadSize);

		BytesSent = 0;
		bSendBuffer = Socket->Send(PayloadBuffer, PayloadSize, BytesSent);
		if (!bSendBuffer)
		{
			PrintSocketError(TEXT("Send"));
			return false;
		}

		delete[] PayloadBuffer;
	}

	return true;


	//const int32 PacketSize = HeaderSize + SendPacket.Payload.Len();

	//uint8_t* SendBuffer = new uint8_t[PacketSize];

	//uint8_t* Packet = PacketMaker::MakeHeader(SendPacket.PacketType, SendPacket.Payload.Len());

	////FMemory::Memcpy(&SendBuffer, (const void*)&Packet, PacketSize);

	////TArray<uint8_t> SendBuffer = SendPacket.Payload.IsEmpty() ? PacketMaker::MakePacket(SendPacket.PacketType)
	////	: PacketMaker::MakePacket(SendPacket.PacketType, SendPacket.Payload);

	////int32 SendBufferSize = SendBuffer.Num();
	////ABLOG(Warning, TEXT("SendBuffer Size : %d"), SendBufferSize);

	//int32 BytesSent = 0;
	//bool bSendBuffer = Socket->Send(Packet, SendPacket.Payload.Len() + 4, BytesSent);
	//if (!bSendBuffer || BytesSent != SendPacket.Payload.Len() + 4)
	//{
	//	PrintSocketError(TEXT("Send"));
	//	return false;
	//}
	//
	//ABLOG(Warning, TEXT("Send Success"));
	//return true;
}

bool UClientLoginSubsystem::IsConnect()
{
	if (Socket && (Socket->GetConnectionState() == ESocketConnectionState::SCS_Connected))
	{
		return true;
	}

	return false;
}

//TArray<uint8_t> PacketMaker::MakePacket(const ELoginPacket& PacketType)
//{
//	// Header
//	//[][][][]
//
//	return MakeHeader(PacketType, 0);
//}
//
//TArray<uint8_t> PacketMaker::MakePacket(const ELoginPacket& PacketType, const FString& Payload)
//{
//	// Header      Payload
//	//[][][][] [Variable data]
//
//	//FString to Utf8
//	TArray<uint8_t> PayloadBuffer;
//	//FTCHARToUTF8 MyConverter(*Payload);
//	//PayloadBuffer.Append(reinterpret_cast<const uint8_t*>(MyConverter.Get()), MyConverter.Length());
//	StringToBytes(Payload, (uint8_t*)&PayloadBuffer, Payload.Len());
//
//	uint16_t PayloadSize = PayloadBuffer.Num();
//	TArray<uint8_t> PacketBuffer = MakeHeader(PacketType, PayloadSize);
//
//	PacketBuffer.Append(PayloadBuffer);
//
//	return PacketBuffer;
//}

//uint8_t* PacketMaker::MakeHeader(const ELoginPacket& PacketType, const uint16_t PayloadSize)
//{
//	//size Type
//	//[][] [][]
//
//	/* I Skip Network Byte Ordering because most of game devices use little endian */
//	//uint16_t Size = hton(PayloadSize);
//	//uint16_t Type = hton(StaticCast<uint16_t>(PacketType));
//
//	const uint16_t Type = StaticCast<uint16_t>(PacketType);
//
//	ABLOG(Warning, TEXT("MakeHeader Type : %d"), (int32)Type);
//	ABLOG(Warning, TEXT("MakeHeader Size : %d"), (int32)PayloadSize);
//
//	uint8_t Header[HeaderSize] = { 0, };
//
//	FMemory::Memcpy(&Header, (const void*)&PayloadSize, 2);
//	FMemory::Memcpy(&Header[2], (const void*)&Type, 2);
//
//	uint8_t* a = (uint8_t*)&Header;
//
//	return a;
//}
