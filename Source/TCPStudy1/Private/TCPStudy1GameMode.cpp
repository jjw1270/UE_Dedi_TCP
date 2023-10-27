// Copyright Epic Games, Inc. All Rights Reserved.

#include "TCPStudy1GameMode.h"
#include "TCPStudy1.h"
#include "TCPStudy1Character.h"
#include "MyGameInstance.h"
#include "RecvThread.h"

void ATCPStudy1GameMode::StartPlay()
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

	if (!GI->GetSocketManager()->Connect())
	{
		return;
	}

	GI->StartRecvThread();
	GI->StartSendThread();

	GI->SendPacket(EPacket::C2S_Ping);
	GI->SendPacket(EPacket::C2S_Chat, TEXT("HIHIHIHIHIHI"));

}

void ATCPStudy1GameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

}
