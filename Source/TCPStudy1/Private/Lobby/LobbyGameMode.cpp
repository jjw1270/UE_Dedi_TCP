// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyGameMode.h"
#include "TCPStudy1.h"
#include "MyGameInstance.h"
#include "ClientLogin/ClientLoginThread.h"

void ALobbyGameMode::StartPlay()
{
	Super::StartPlay();

	GI = GetGameInstance();
	CHECK_VALID(GI);

	UClientLoginSubsystem* ClientLoginSubsystem = GI->GetSubsystem<UClientLoginSubsystem>();
	CHECK_VALID(ClientLoginSubsystem);

	// Connect to ClientLogin TCP Server
	bool bConnect = ClientLoginSubsystem->Connect(8881, TEXT("127.0.0.1"));
	if (!bConnect)
	{
		if (LobbyInfoDelegate.IsBound())
		{
			LobbyInfoDelegate.Execute(TEXT("로그인 서버 접속 실패"), false);
		}

		return;
	}

	// Start Client Login Thread
	ClientLoginThread = new FClientLoginThread(ClientLoginSubsystem);
}

void ALobbyGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (PacketToProcess.PacketType)
	{
		ABLOG_S(Warning);
	case ELoginPacket::S2C_ConnectSuccess:
		LobbyInfoDelegate.ExecuteIfBound(TEXT("로그인 서버 접속 성공"), true);
		break;
	default:
		break;
	}

	PacketToProcess = FLoginPacketData();
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	ClientLoginThread->Stop();

	delete ClientLoginThread;

	Super::Logout(Exiting);
}
