// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
DECLARE_DELEGATE_OneParam(FDele_LobbyInfo, const FString&);

UCLASS()
class TCPSTUDY1_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	virtual void StartPlay() override;

public:
	FDele_LobbyInfo LobbyInfoDelegate;

};
