// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginLobbyGameMode.h"
#include "TCPStudy1.h"
#include "MyGameInstance.h"
#include "ClientLoginThread.h"

void ALoginLobbyGameMode::StartPlay()
{
	Super::StartPlay();

	UGameInstance* GI = GetGameInstance();
	CHECK_VALID(GI);

	ClientLoginSubsystem = GI->GetSubsystem<UClientLoginSubsystem>();

	ConnectToLoginServer();
}

void ALoginLobbyGameMode::ConnectToLoginServer()
{
	CHECK_VALID(ClientLoginSubsystem);

	bool bConnect = ClientLoginSubsystem->Connect(8881, TEXT("127.0.0.1"));
	if (!bConnect)
	{
		LoginLobbyInfoDelegate.ExecuteIfBound(TEXT("로그인 서버 접속 실패"), 0, false);

		// Reconnect to login server
		FTimerHandle ReconnectLoginServerHandle;
		GetWorld()->GetTimerManager().SetTimer(ReconnectLoginServerHandle, this, &ALoginLobbyGameMode::ConnectToLoginServer, 5.f, false);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(ProcessPacketHandle, this, &ALoginLobbyGameMode::ManageRecvPacket, 0.1f, true);

		// Start Client Login Thread
		ClientLoginThread = new FClientLoginThread(ClientLoginSubsystem);
		ClientLoginThreadHandle = FRunnableThread::Create(ClientLoginThread, TEXT("ClientLoginThread"));
	}
}

void ALoginLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

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
}

void ALoginLobbyGameMode::ManageRecvPacket()
{
	if (!ClientLoginThreadHandle || !ClientLoginSubsystem)
	{
		GetWorld()->GetTimerManager().ClearTimer(ProcessPacketHandle);
	}

	if (PacketToProcess.PacketType != ELoginPacket::None)
	{
		int32 PacketCode = static_cast<int32>(PacketToProcess.PacketType);

		switch (PacketToProcess.PacketType)
		{
		case ELoginPacket::S2C_ConnectSuccess:
			LoginLobbyInfoDelegate.ExecuteIfBound(TEXT("로그인 서버 접속 성공"), PacketCode, true);
			break;
		case ELoginPacket::S2C_ResSignIn_Fail_InValidID:
			LoginLobbyInfoDelegate.ExecuteIfBound(TEXT("등록되지 않은 아이디 입니다"), PacketCode, false);
			break;
		case ELoginPacket::S2C_ResSignIn_Fail_InValidPassword:
			LoginLobbyInfoDelegate.ExecuteIfBound(TEXT("비밀번호가 일치하지 않습니다"), PacketCode, false);
			break;
		case ELoginPacket::S2C_ResSignIn_Success:
			LoginLobbyInfoDelegate.ExecuteIfBound(TEXT("로그인 성공! 접속중.."), PacketCode, true);
			break;
		case ELoginPacket::S2C_ResSignUpIDPwd_Success:
			LoginLobbyInfoDelegate.ExecuteIfBound(TEXT("새로운 닉네임을 입력하세요"), PacketCode, true);
			break;
		case ELoginPacket::S2C_ResSignUpIDPwd_Fail_ExistID:
			LoginLobbyInfoDelegate.ExecuteIfBound(TEXT("아이디가 이미 존재합니다"), PacketCode, false);
			break;
		case ELoginPacket::S2C_ResSignUpNickName_Success:
			LoginLobbyInfoDelegate.ExecuteIfBound(TEXT("등록되었습니다!"), PacketCode, true);
			break;
		case ELoginPacket::S2C_ResSignUpNickName_Fail_ExistNickName:
			LoginLobbyInfoDelegate.ExecuteIfBound(TEXT("닉네임이 이미 존재합니다"), PacketCode, false);
			break;
		default:
			break;
		}

		PacketToProcess = FLoginPacketData();
	}
}
