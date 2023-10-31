// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainLobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TCPSTUDY1_API AMainLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UMainLobbyWidget> MainLobbyWidgetClass;

	UPROPERTY()
	class UMainLobbyWidget* MainLobbyWidget;
};
