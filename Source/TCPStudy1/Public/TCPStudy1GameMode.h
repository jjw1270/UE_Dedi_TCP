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

	virtual void Tick(float DeltaSeconds) override;

public:
	FORCEINLINE const SocketManager& GetSocketManager() const { return SocketManager; }

private:
	SocketManager SocketManager;

	TSharedPtr<class RecvThread> pRecvThread;
};
