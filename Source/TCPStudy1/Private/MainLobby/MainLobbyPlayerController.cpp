// Fill out your copyright notice in the Description page of Project Settings.


#include "MainLobbyPlayerController.h"
#include "MainLobbyWidget.h"

void AMainLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (MainLobbyWidgetClass)
	{
		MainLobbyWidget = CreateWidget<UMainLobbyWidget>(this, MainLobbyWidgetClass);
		if (MainLobbyWidget)
		{
			MainLobbyWidget->AddToViewport();
		}
	}
}
