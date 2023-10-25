// Copyright Epic Games, Inc. All Rights Reserved.

#include "TCPStudy1GameMode.h"
#include "TCPStudy1.h"
#include "TCPStudy1Character.h"
#include "UObject/ConstructorHelpers.h"

void ATCPStudy1GameMode::StartPlay()
{
	Super::StartPlay();

	ABLOG(Warning, TEXT("GameInstance Start, Connect to TCP"));

	TCPClient.Connect();
}
