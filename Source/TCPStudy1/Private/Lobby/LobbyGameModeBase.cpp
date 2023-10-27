// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyGameModeBase.h"
#include "TCPStudy1.h"
#include "MyGameInstance.h"

void ALobbyGameModeBase::StartPlay()
{
	Super::StartPlay();

	ABLOG(Warning, TEXT("GameInstance Start, Connect to TCP"));

	GI = GetGameInstance<UMyGameInstance>();
	if (!GI)
	{
		ABLOG(Error, TEXT("GI is null"));
		return;
	}

	GI->InitSocketManager();

	// Connect to ClientLogin TCP Server
	if (!GI->GetSocketManager()->Connect(8881))
	{
		return;
	}

}
