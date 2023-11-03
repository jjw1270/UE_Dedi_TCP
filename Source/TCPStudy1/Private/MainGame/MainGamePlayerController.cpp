// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGamePlayerController.h"

void AMainGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);
}
