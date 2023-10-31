// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TCPSTUDY1_API AMainLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	virtual void StartPlay() override;

};
