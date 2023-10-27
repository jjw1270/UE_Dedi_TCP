// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyGameMode.h"
#include "TCPStudy1.h"
#include "MyGameInstance.h"
#include "ClientLogin/ClientLoginSubsystem.h"

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

	ABLOG(Warning, TEXT("RECV"));

	FLoginPacketData PacketData;
	bool RecvByte = ClientLoginSubsystem->Recv(PacketData);
	if (RecvByte && PacketData.PacketType == ELoginPacket::S2C_ConnectSuccess)
	{
		LobbyInfoDelegate.Execute(TEXT("로그인 서버 접속 성공"), true);
	}
}
