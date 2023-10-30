// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyGameMode.h"
#include "TCPStudy1.h"
#include "MyGameInstance.h"
#include "ClientLogin/ClientLoginThread.h"

void ALobbyGameMode::StartPlay()
{
	Super::StartPlay();

}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UGameInstance* GI = GetGameInstance();
	CHECK_VALID(GI);

	ClientLoginSubsystem = GI->GetSubsystem<UClientLoginSubsystem>();

	ConnectToLoginServer();

	// Start Client Login Thread
	ClientLoginThread = new FClientLoginThread(ClientLoginSubsystem);
	ClientLoginThreadHandle = FRunnableThread::Create(ClientLoginThread, TEXT("ClientLoginThread"));

	FTimerHandle ProcessPacketHandle;
	GetWorld()->GetTimerManager().SetTimer(ProcessPacketHandle, this, &ALobbyGameMode::ManageRecvPacket, 0.1f, true);
}

void ALobbyGameMode::Logout(AController* Exiting)
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
	}

	Super::Logout(Exiting);
}

void ALobbyGameMode::ConnectToLoginServer()
{
	CHECK_VALID(ClientLoginSubsystem);

	bool bConnect = ClientLoginSubsystem->Connect(8881, TEXT("127.0.0.1"));
	if (!bConnect)
	{
		if (LobbyInfoDelegate.IsBound())
		{
			LobbyInfoDelegate.Execute(TEXT("로그인 서버 접속 실패"), false);
		}

		// Reconnect to login server
		FTimerHandle ReconnectLoginServerHandle;
		GetWorld()->GetTimerManager().SetTimer(ReconnectLoginServerHandle, this, &ALobbyGameMode::ConnectToLoginServer, 5.f, false);
	}
}

void ALobbyGameMode::ManageRecvPacket()
{
	CHECK_VALID(ClientLoginSubsystem);

	if (PacketToProcess.PacketType != ELoginPacket::None)
	{
		switch (PacketToProcess.PacketType)
		{
		case ELoginPacket::S2C_ConnectSuccess:
			LobbyInfoDelegate.ExecuteIfBound(TEXT("로그인 서버 접속 성공"), true);
			break;
		case ELoginPacket::S2C_ResSignIn_Fail_InValidID:
			LobbyInfoDelegate.ExecuteIfBound(TEXT("등록되지 않은 아이디 입니다."), false);
			break;
		case ELoginPacket::S2C_ResSignIn_Fail_InValidPassword:
			LobbyInfoDelegate.ExecuteIfBound(TEXT("비밀번호가 일치하지 않습니다."), false);
			break;
		case ELoginPacket::S2C_ResSignIn_Success:
			// Login Success!!!
			break;
		default:
			break;
		}

		PacketToProcess = FLoginPacketData();
	}
}
