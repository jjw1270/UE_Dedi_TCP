// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

public:
	FDele_LobbyInfo LobbyInfoDelegate;

protected:
	UPROPERTY()
	UGameInstance* GI;

};
