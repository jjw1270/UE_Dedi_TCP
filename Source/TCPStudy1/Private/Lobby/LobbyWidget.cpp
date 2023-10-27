// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Lobby/LobbyGameMode.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (LobbyGameMode)
	{
		LobbyGameMode->LobbyInfoDelegate.BindUFunction(this, FName("OnLobbyInfoDelegate"));
	}
}

void ULobbyWidget::NativeDestruct()
{
	Super::NativeDestruct();

	ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (LobbyGameMode)
	{
		LobbyGameMode->LobbyInfoDelegate.Unbind();
	}
}

void ULobbyWidget::OnLobbyInfoDelegate(const FString& InfoMessage)
{
	TextBlock_Info->SetVisibility(ESlateVisibility::Visible);
	TextBlock_Info->SetText(FText::FromString(InfoMessage));

	if (InfoMessage == TEXT("로그인 서버 접속 실패"))
	{
		EditableTextBox_ID->SetIsEnabled(false);
		EditableTextBox_Password->SetIsEnabled(false);
		Button_SignIn->SetIsEnabled(false);
		Button_SignUp->SetIsEnabled(false);
		return;
	}

	if (InfoTextHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(InfoTextHandle);
	}
	GetWorld()->GetTimerManager().SetTimer(InfoTextHandle, this, &ULobbyWidget::HideInfoText, 5.f, false);
}

void ULobbyWidget::HideInfoText()
{
	TextBlock_Info->SetVisibility(ESlateVisibility::Hidden);
}
