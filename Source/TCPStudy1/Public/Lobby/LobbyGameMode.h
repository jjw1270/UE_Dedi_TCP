// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ClientLogin/ClientLoginSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 *
 */
DECLARE_DELEGATE_TwoParams(FDele_LobbyInfo, const FString&, bool);

UCLASS()
class TCPSTUDY1_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void StartPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void Logout(AController* Exiting) override;

public:
	FORCEINLINE void SetProccessPacket(FLoginPacketData PacketData) { PacketToProcess = PacketData; }

public:
	FDele_LobbyInfo LobbyInfoDelegate;

protected:
	UPROPERTY()
	UGameInstance* GI;

	class FClientLoginThread* ClientLoginThread;

	FLoginPacketData PacketToProcess;
};
