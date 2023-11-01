// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TransitionPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TCPSTUDY1_API ATransitionPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UUserWidget> TransitionWidgetClass;

	UPROPERTY()
	class UUserWidget* TransitionWidget;
	
};
