// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientLoginSubsystem.h"
/* Custom Includes Here--------------------------------*/
#include "TCPStudy1.h"
/*-----------------------------------------------------*/

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
	ABLOG(Warning, TEXT("Initialize ClientLoginSubsystem"));
}

void UClientLoginSubsystem::Deinitialize()
{
	ABLOG(Warning, TEXT("Deinitialize ClientLoginSubsystem"));

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

void UClientLoginSubsystem::ConnectToLoginServer()
{
	UWorld* World = GetWorld();
	CHECK_VALID(World);

	bool bConnect = Connect(8881, TEXT("127.0.0.1"));
	if (!bConnect)
	{
		if (RecvPacketDelegate.IsBound())
		{
			RecvPacketDelegate.Broadcast(TEXT("로그인 서버 접속 실패"), 0, false);
		}

		// Reconnect to login server
		FTimerHandle ReconnectLoginServerHandle;
		World->GetTimerManager().SetTimer(ReconnectLoginServerHandle, this, &UClientLoginSubsystem::ConnectToLoginServer, 5.f, false);
	}
	else
	{
		World->GetTimerManager().SetTimer(ManageRecvPacketHandle, this, &UClientLoginSubsystem::ManageRecvPacket, 0.1f, true);

		// Start Client Login Thread
		ClientLoginThread = new FClientLoginThread(this);
		ClientLoginThreadHandle = FRunnableThread::Create(ClientLoginThread, TEXT("ClientLoginThread"));
	}
}

void UClientLoginSubsystem::DestroySocket()
{
	// Clean Thread
	if (ClientLoginThread)
	{
		ClientLoginThread->StopThread();

		if (ClientLoginThreadHandle)
		{
			ClientLoginThreadHandle->WaitForCompletion();
			delete ClientLoginThreadHandle;
			ClientLoginThreadHandle = nullptr;
		}

		delete ClientLoginThread;
		ClientLoginThread = nullptr;

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

void UClientLoginSubsystem::PrintSocketError(const FString& Text)
{
	ESocketErrors SocketErrorCode = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode();
	const TCHAR* SocketError = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetSocketError(SocketErrorCode);

	UE_LOG(LogSockets, Error, TEXT("[%s]  SocketError : %s"), *Text, SocketError);
}

void UClientLoginSubsystem::ManageRecvPacket()
{
	if (!ClientLoginThreadHandle)
	{
		GetWorld()->GetTimerManager().ClearTimer(ManageRecvPacketHandle);
		return;
	}

	if (!RecvPacketDelegate.IsBound())
	{
		return;
	}

	int32 PacketCode = static_cast<int32>(RecvPacketData.PacketType);

	switch (RecvPacketData.PacketType)
	{
	case ELoginPacket::S2C_ConnectSuccess:
		RecvPacketDelegate.Broadcast(TEXT("로그인 서버 접속 성공"), PacketCode, true);
		break;
	case ELoginPacket::S2C_ResSignIn_Fail_InValidID:
		SetIDPwd("");
		RecvPacketDelegate.Broadcast(TEXT("등록되지 않은 아이디 입니다"), PacketCode, false);
		break;
	case ELoginPacket::S2C_ResSignIn_Fail_InValidPassword:
		RecvPacketDelegate.Broadcast(TEXT("비밀번호가 일치하지 않습니다"), PacketCode, false);
		break;
	case ELoginPacket::S2C_ResSignIn_Success:
		SetUserNickName(RecvPacketData.Payload);
		RecvPacketDelegate.Broadcast(FString::Printf(TEXT("환영합니다 %s 님!"), *RecvPacketData.Payload), PacketCode, true);
		break;
	case ELoginPacket::S2C_ResSignUpIDPwd_Success:
		RecvPacketDelegate.Broadcast(TEXT("새로운 닉네임을 입력하세요"), PacketCode, true);
		break;
	case ELoginPacket::S2C_ResSignUpIDPwd_Fail_ExistID:
		RecvPacketDelegate.Broadcast(TEXT("아이디가 이미 존재합니다"), PacketCode, false);
		break;
	case ELoginPacket::S2C_ResSignUpNickName_Success:
		RecvPacketDelegate.Broadcast(TEXT("등록되었습니다!"), PacketCode, true);
		break;
	case ELoginPacket::S2C_ResSignUpNickName_Fail_ExistNickName:
		RecvPacketDelegate.Broadcast(TEXT("닉네임이 이미 존재합니다"), PacketCode, false);
		break;
	default:
		break;
	}

	RecvPacketData = FLoginPacketData();
}

bool UClientLoginSubsystem::Recv(FLoginPacketData& OutRecvPacket)
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

		OutRecvPacket.PacketType = static_cast<ELoginPacket>(RecvPacketType);

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

bool UClientLoginSubsystem::Send(const FLoginPacketData& SendPacket)
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

bool UClientLoginSubsystem::IsConnect()
{
	if (Socket && (Socket->GetConnectionState() == ESocketConnectionState::SCS_Connected))
	{
		return true;
	}

	return false;
}

FClientLoginThread::FClientLoginThread(UClientLoginSubsystem* NewClientLoginSubsystem)
	: ClientLoginSubsystem(NewClientLoginSubsystem)
{
	bStopThread = false;
}

uint32 FClientLoginThread::Run()
{
	while (!bStopThread)
	{
		FLoginPacketData PacketData;
		bool RecvByte = ClientLoginSubsystem->Recv(PacketData);
		if (!RecvByte)
		{
			ABLOG(Error, TEXT("Recv Error, Stop Thread"));
			break;
		}

		if (PacketData.PacketType != ELoginPacket::None)
		{
			ClientLoginSubsystem->SetRecvPacket(PacketData);
		}
	}

	return 0;
}

void FClientLoginThread::StopThread()
{
	bStopThread = true;
}
