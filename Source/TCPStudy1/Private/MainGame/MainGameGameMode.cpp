// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameGameMode.h"
#include "TCPStudy1.h"
#include "DediServerSubsystem.h"

void AMainGameGameMode::StartPlay()
{
	Super::StartPlay();

	UGameInstance* GI = GetGameInstance();
	CHECK_VALID(GI);

	UDediServerSubsystem* DediServerSubsystem = GI->GetSubsystem<UDediServerSubsystem>();
	CHECK_VALID(DediServerSubsystem);

	DediServerSubsystem->ConnectToTCPDediServer();

	FDediPacketData PacketData(EDediPacket::C2S_ConnectSuccess);
	bool bSend = DediServerSubsystem->Send(PacketData);
	if (!bSend)
	{
		ABLOG(Error, TEXT("Send Error"));
	}
}

void AMainGameGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ABLOG(Warning, TEXT("Player Logout"));

	if (GetNumPlayers() == 1)
	{
		ABLOG(Warning, TEXT("RequestExit"));
		FGenericPlatformMisc::RequestExit(false);
	}
}

void AMainGameGameMode::EndMainGame()
{
	FString MainLobbyLevel = TEXT("MainLobbyLevel");
	ProcessClientTravel(MainLobbyLevel, true, true);
}
