// Fill out your copyright notice in the Description page of Project Settings.


#include "TransitionPlayerController.h"
#include "Blueprint/UserWidget.h"

void ATransitionPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (TransitionWidgetClass)
	{
		TransitionWidget = CreateWidget<UUserWidget>(this, TransitionWidgetClass);
		if (TransitionWidget)
		{
			TransitionWidget->AddToViewport();
		}
	}
}
