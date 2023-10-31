// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginLobbyGameMode.h"
#include "TCPStudy1.h"
#include "ClientLoginSubsystem.h"

void ALoginLobbyGameMode::StartPlay()
{
	Super::StartPlay();

	UGameInstance* GI = GetGameInstance();
	CHECK_VALID(GI);

	UClientLoginSubsystem* ClientLoginSubsystem = GI->GetSubsystem<UClientLoginSubsystem>();
	CHECK_VALID(ClientLoginSubsystem);

	ClientLoginSubsystem->ConnectToLoginServer();
}

void ALoginLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}
