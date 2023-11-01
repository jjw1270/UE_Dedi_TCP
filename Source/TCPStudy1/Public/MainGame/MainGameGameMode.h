// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainGameGameMode.generated.h"

/**
 * ONLY RUN ON SERVER!!!!!!
 */
UCLASS()
class TCPSTUDY1_API AMainGameGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	virtual void StartPlay() override;

};
