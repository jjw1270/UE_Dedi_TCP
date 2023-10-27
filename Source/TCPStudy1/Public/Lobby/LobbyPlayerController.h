// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

/**
 *
 */
UCLASS()
class TCPSTUDY1_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UUserWidget> LobbyWidgetClass;

	UPROPERTY()
	class UUserWidget* LobbyWidget;

};
