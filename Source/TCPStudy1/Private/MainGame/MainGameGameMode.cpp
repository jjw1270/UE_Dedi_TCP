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
}
