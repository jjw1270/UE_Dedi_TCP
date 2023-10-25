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

	//pRecvThread = MakeShared<RecvThread, ESPMode::ThreadSafe>(&SocketManager);

	//_sleep(1000);
	//SocketManager.Send(EPacket::C2S_Chat, TEXT("HIHI"));
}

void ATCPStudy1GameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

}
