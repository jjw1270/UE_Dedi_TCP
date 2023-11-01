// Fill out your copyright notice in the Description page of Project Settings.


#include "DediServerSubsystem.h"
/* Custom Includes Here--------------------------------*/
#include "TCPStudy1.h"
/*-----------------------------------------------------*/

#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "IPAddress.h"

UDediServerSubsystem::UDediServerSubsystem()
{
}

void UDediServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	ABLOG(Warning, TEXT("Initialize DediServerSubsystem"));
}

void UDediServerSubsystem::Deinitialize()
{
	ABLOG(Warning, TEXT("Deinitialize DediServerSubsystem"));

	if (IsConnect())
	{
		DestroySocket();
	}
}

bool UDediServerSubsystem::Connect(const int32& PortNum, const FString& IP)
{
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("TCPDediServerSocket"), false);
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

void UDediServerSubsystem::ConnectToTCPDediServer()
{
	UWorld* World = GetWorld();
	CHECK_VALID(World);

	bool bConnect = Connect(11233, TEXT("127.0.0.1"));
	if (!bConnect)
	{
		if (RecvPacketDelegate.IsBound())
		{
			RecvPacketDelegate.Broadcast(TEXT("데디 TCP 서버 접속 실패"), 0);
		}

		// Reconnect to login server
		FTimerHandle ReconnectLoginServerHandle;
		World->GetTimerManager().SetTimer(ReconnectLoginServerHandle, this, &UDediServerSubsystem::ConnectToTCPDediServer, 5.f, false);
	}
	else
	{
		World->GetTimerManager().SetTimer(ManageRecvPacketHandle, this, &UDediServerSubsystem::ManageRecvPacket, 0.1f, true);

		// Start Client Login Thread
		DediServerThread = new FDediServerThread(this);
		DediServerThreadHandle = FRunnableThread::Create(DediServerThread, TEXT("DediServerThread"));
	}
}

void UDediServerSubsystem::DestroySocket()
{	
	// Clean Thread
	if (DediServerThread)
	{
		DediServerThread->StopThread();

		if (DediServerThreadHandle)
		{
			DediServerThreadHandle->WaitForCompletion();
			delete DediServerThreadHandle;
			DediServerThreadHandle = nullptr;
		}

		delete DediServerThread;
		DediServerThread = nullptr;

		ABLOG(Warning, TEXT("CleanUp Thread"));
	}

	// Clean Socket
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

bool UDediServerSubsystem::Recv(FDediPacketData& OutRecvPacket)
{
	if (!Socket)
	{
		ABLOG(Error, TEXT("Socket is null"));
		return false;
	}

	if (Socket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromSeconds(.5f)))
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

		OutRecvPacket.PacketType = static_cast<EDediPacket>(RecvPacketType);

		// Recv Payload
		if (RecvPayloadSize > 0)
		{
			uint8_t* PayloadBuffer = new uint8_t[RecvPayloadSize + 1];

			BytesRead = 0;
			bool bRecvPayload = Socket->Recv(PayloadBuffer, RecvPayloadSize, BytesRead);

			if (!bRecvPayload)
			{
				PrintSocketError(TEXT("Receive Payload"));
				return false;
			}
			PayloadBuffer[RecvPayloadSize] = '\0';

			//Utf8 to FStirng
			FString PayloadString;
			PayloadString = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(PayloadBuffer)));

			OutRecvPacket.Payload = PayloadString;

			delete[] PayloadBuffer;
			PayloadBuffer = nullptr;
		}

		ABLOG(Warning, TEXT(" [Recv] PacketType : %d, PayloadSize : %d"), RecvPacketType, RecvPayloadSize);
	}
	return true;
}

bool UDediServerSubsystem::Send(const FDediPacketData& SendPacket)
{
	if (!Socket)
	{
		ABLOG(Error, TEXT("Socket is null"));
		return false;
	}

	uint8_t* PayloadBuffer = nullptr;
	uint16_t PayloadSize = 0;

	if (!SendPacket.Payload.IsEmpty())
	{
		// FString to UTF8 const char* type buffer
		ANSICHAR* PayloadCharBuf = TCHAR_TO_UTF8(*SendPacket.Payload);
		PayloadSize = strlen(PayloadCharBuf);
		PayloadBuffer = reinterpret_cast<uint8_t*>(PayloadCharBuf);
	}

	// Send Header
	ABLOG(Warning, TEXT("Payload Size : %d"), (int32)PayloadSize);

	const uint16_t Type = static_cast<uint16_t>(SendPacket.PacketType);

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

	if (PayloadBuffer != nullptr)
	{
		BytesSent = 0;
		bSendBuffer = Socket->Send(PayloadBuffer, PayloadSize, BytesSent);
		if (!bSendBuffer)
		{
			PrintSocketError(TEXT("Send"));
			return false;
		}
	}

	return true;
}

bool UDediServerSubsystem::IsConnect()
{
	if (Socket && (Socket->GetConnectionState() == ESocketConnectionState::SCS_Connected))
	{
		return true;
	}

	return false;
}

void UDediServerSubsystem::PrintSocketError(const FString& Text)
{
	ESocketErrors SocketErrorCode = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode();
	const TCHAR* SocketError = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetSocketError(SocketErrorCode);

	UE_LOG(LogSockets, Error, TEXT("[%s]  SocketError : %s"), *Text, SocketError);
}

void UDediServerSubsystem::ManageRecvPacket()
{
	if (!DediServerThreadHandle)
	{
		GetWorld()->GetTimerManager().ClearTimer(ManageRecvPacketHandle);
		return;
	}

	if (RecvPacketDelegate.IsBound())
	{
		int32 PacketCode = static_cast<int32>(RecvPacketData.PacketType);

		switch (RecvPacketData.PacketType)
		{
		case EDediPacket::S2C_ConnectSuccess:
			RecvPacketDelegate.Broadcast(TEXT("데디 TCP 서버 접속 성공"), PacketCode);
			break;
		default:
			break;
		}
	}

	RecvPacketData = FDediPacketData();
}

FDediServerThread::FDediServerThread(UDediServerSubsystem* NewDediServerSubsystem)
	: DediServerSubsystem(NewDediServerSubsystem)
{
	bStopThread = false;
}

uint32 FDediServerThread::Run()
{
	while (!bStopThread)
	{
		FDediPacketData PacketData;
		bool RecvByte = DediServerSubsystem->Recv(PacketData);
		if (!RecvByte)
		{
			ABLOG(Error, TEXT("Recv Error, Stop Thread"));
			break;
		}

		if (PacketData.PacketType != EDediPacket::None)
		{
			DediServerSubsystem->SetRecvPacket(PacketData);
		}
	}

	return 0;
}

void FDediServerThread::StopThread()
{
	bStopThread = true;
}
