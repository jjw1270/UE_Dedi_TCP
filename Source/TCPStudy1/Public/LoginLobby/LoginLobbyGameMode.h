// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LoginLobbyGameMode.generated.h"

/**
 *
 */

UCLASS()
class TCPSTUDY1_API ALoginLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void StartPlay() override;

	virtual void Logout(AController* Exiting) override;

};
