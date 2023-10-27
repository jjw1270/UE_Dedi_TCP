// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SocketManager.h"
#include "TCPStudy1GameMode.generated.h"

UCLASS(minimalapi)
class ATCPStudy1GameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void StartPlay() override;

	//virtual void PostLogin(APlayerController* NewPlayer) override;
	//virtual void Logout(AController* Exiting) override;

private:
	UPROPERTY()
	class UMyGameInstance* GI;

	//TSharedPtr<class RecvThread> pRecvThread;
};
