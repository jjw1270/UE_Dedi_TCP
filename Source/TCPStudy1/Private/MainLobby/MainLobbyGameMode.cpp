// Fill out your copyright notice in the Description page of Project Settings.


#include "MainLobbyGameMode.h"
#include "TCPStudy1.h"
#include "ClientLoginSubsystem.h"

void AMainLobbyGameMode::StartPlay()
{
	Super::StartPlay();

	UGameInstance* GI = GetGameInstance();
	CHECK_VALID(GI);

	UClientLoginSubsystem* ClientLoginSubsystem = GI->GetSubsystem<UClientLoginSubsystem>();
	CHECK_VALID(ClientLoginSubsystem);

	if (!ClientLoginSubsystem->IsConnect())
	{
		ClientLoginSubsystem->ConnectToLoginServer();

		// ReLogin
		if (!ClientLoginSubsystem->GetIDPwd().IsEmpty())
		{
			FLoginPacketData PacketData(ELoginPacket::C2S_ReqSignIn, ClientLoginSubsystem->GetIDPwd());
			bool bSend = ClientLoginSubsystem->Send(PacketData);
			if (!bSend)
			{
				ABLOG(Error, TEXT("Send Error"));
			}
		}
	}
}
