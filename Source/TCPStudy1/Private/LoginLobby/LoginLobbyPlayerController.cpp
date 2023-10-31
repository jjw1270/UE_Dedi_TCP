// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginLobbyPlayerController.h"
#include "LoginLobbyWidget.h"

void ALoginLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (LoginLobbyWidgetClass)
	{
		LoginLobbyWidget = CreateWidget<ULoginLobbyWidget>(this, LoginLobbyWidgetClass);
		if (LoginLobbyWidget)
		{
			LoginLobbyWidget->AddToViewport();

			SetInputMode(FInputModeUIOnly());
			SetShowMouseCursor(true);
		}
	}
}
