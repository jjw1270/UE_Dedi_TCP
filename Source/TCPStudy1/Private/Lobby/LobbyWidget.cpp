// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyWidget.h"
#include "TCPStudy1.h"
#include "Kismet/GameplayStatics.h"
#include "Lobby/LobbyGameMode.h"
#include "ClientLogin/ClientLoginSubsystem.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	CHECK_VALID(LobbyGameMode);

	LobbyGameMode->LobbyInfoDelegate.BindUFunction(this, FName("OnLobbyInfoDelegate"));

	Button_SignIn->OnClicked.AddDynamic(this, &ULobbyWidget::Button_SignIn_Clicked);
	Button_SignUp->OnClicked.AddDynamic(this, &ULobbyWidget::Button_SignUp_Clicked);
	Button_QuitGame->OnClicked.AddDynamic(this, &ULobbyWidget::Button_QuitGame_Clicked);

	ClientLoginSubsystem = GetGameInstance()->GetSubsystem<UClientLoginSubsystem>();
	CHECK_VALID(ClientLoginSubsystem);

}

void ULobbyWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	CHECK_VALID(LobbyGameMode);
	LobbyGameMode->LobbyInfoDelegate.Unbind();
}

void ULobbyWidget::OnLobbyInfoDelegate(const FString& InfoMessage, bool bSuccess)
{
	TextBlock_Info->SetVisibility(ESlateVisibility::Visible);
	TextBlock_Info->SetText(FText::FromString(InfoMessage));
	TextBlock_Info->SetColorAndOpacity(FSlateColor(bSuccess ? FColor::Green : FColor::Red));

	if (InfoMessage == TEXT("로그인 서버 접속 실패"))
	{
		// Disable Login Inputs
		EditableTextBox_ID->SetIsEnabled(false);
		EditableTextBox_Password->SetIsEnabled(false);
		Button_SignIn->SetIsEnabled(false);
		Button_SignUp->SetIsEnabled(false);
	}
	else
	{
		if (!EditableTextBox_ID->GetIsEnabled())
		{
			EditableTextBox_ID->SetIsEnabled(true);
			EditableTextBox_Password->SetIsEnabled(true);
			Button_SignIn->SetIsEnabled(true);
			Button_SignUp->SetIsEnabled(true);
		}
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

void ULobbyWidget::Button_SignIn_Clicked()
{
	FString ID = EditableTextBox_ID->GetText().ToString();
	FString Pwd = EditableTextBox_Password->GetText().ToString();

	if (ID.Len() <= 0 || Pwd.Len() <= 0)
	{
		LobbyGameMode->LobbyInfoDelegate.Execute(TEXT("ID 또는 Password를 입력하세요."), false);
		return;
	}

	// IDPwd = "ID:Pwd"
	FString IDPwd = ID;
	IDPwd.Append(TEXT(":"));
	IDPwd.Append(Pwd);

	FLoginPacketData PacketData(ELoginPacket::C2S_ReqSignIn, IDPwd);
	bool bSend = ClientLoginSubsystem->Send(PacketData);
	if (!bSend)
	{
		ABLOG(Error, TEXT("Send Error"));
	}
}

void ULobbyWidget::Button_SignUp_Clicked()
{
	FLoginPacketData PacketData(ELoginPacket::C2S_Ping, TEXT("HELLO"));
	bool bSend = ClientLoginSubsystem->Send(PacketData);
	if (!bSend)
	{
		ABLOG(Error, TEXT("Send Error"));
	}
}

void ULobbyWidget::Button_QuitGame_Clicked()
{
	FLoginPacketData PacketData(ELoginPacket::C2S_ReqSignIn, TEXT("123가나다라!@#"));
	bool bSend = ClientLoginSubsystem->Send(PacketData);
	if (!bSend)
	{
		ABLOG(Error, TEXT("Send Error"));
	}
}
