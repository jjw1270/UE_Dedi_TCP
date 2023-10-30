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
	Button_Check->OnClicked.AddDynamic(this, &ULobbyWidget::Button_Check_Clicked);
	Button_CancelSignUp->OnClicked.AddDynamic(this, &ULobbyWidget::Button_CancelSignUp_Clicked);
	Button_CheckNickName->OnClicked.AddDynamic(this, &ULobbyWidget::Button_CheckNickName_Clicked);
	Button_CancelSignUpNickName->OnClicked.AddDynamic(this, &ULobbyWidget::Button_CancelSignUpNickName_Clicked);

	ClientLoginSubsystem = GetGameInstance()->GetSubsystem<UClientLoginSubsystem>();
	CHECK_VALID(ClientLoginSubsystem);

	EnableInputs(false);
}

void ULobbyWidget::NativeDestruct()
{
	Super::NativeDestruct();

	CHECK_VALID(LobbyGameMode);
	LobbyGameMode->LobbyInfoDelegate.Unbind();
}

void ULobbyWidget::OnLobbyInfoDelegate(const FString& InfoMessage, const int32& PacketType, bool bSuccess)
{
	switch (static_cast<ELoginPacket>(PacketType))
	{
	case ELoginPacket::S2C_ConnectSuccess:
		EnableInputs(true);
		break;
	case ELoginPacket::S2C_ResSignIn_Success:
		EnableInputs(false);
		// Login Success Logic Here!!
		//
		//
		break;
	case ELoginPacket::S2C_ResSignUpIDPwd_Success:
		Border_SignUp_NickName->SetVisibility(ESlateVisibility::Visible);
		break;
	case ELoginPacket::S2C_ResSignUpNickName_Success:
		Button_CancelSignUpNickName_Clicked();
		Button_CancelSignUp_Clicked();
		break;
	default:
		break;
	}

	TextBlock_Info->SetText(FText::FromString(InfoMessage));
	TextBlock_Info->SetColorAndOpacity(FSlateColor(bSuccess ? FColor::Green : FColor::Red));

	if (InfoTextHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(InfoTextHandle);
	}

	TextBlock_Info->SetVisibility(ESlateVisibility::Visible);
	GetWorld()->GetTimerManager().SetTimer(InfoTextHandle, [&]() {TextBlock_Info->SetVisibility(ESlateVisibility::Hidden); }, 5.f, false);
}

void ULobbyWidget::EnableInputs(bool bEnable)
{
	if (EditableTextBox_ID->GetIsEnabled() != bEnable)
	{
		EditableTextBox_ID->SetIsEnabled(bEnable);
		EditableTextBox_Password->SetIsEnabled(bEnable);
		Button_SignIn->SetIsEnabled(bEnable);
		Button_SignUp->SetIsEnabled(bEnable);
	}
}

void ULobbyWidget::Button_SignIn_Clicked()
{
	FString ID = EditableTextBox_ID->GetText().ToString();
	FString Pwd = EditableTextBox_Password->GetText().ToString();

	EditableTextBox_ID->SetText(FText::GetEmpty());
	EditableTextBox_Password->SetText(FText::GetEmpty());

	if (ID.Len() <= 0 || Pwd.Len() <= 0)
	{
		LobbyGameMode->LobbyInfoDelegate.ExecuteIfBound(TEXT("ID 또는 Password를 입력하세요."), 0, false);
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
	LobbyGameMode->LobbyInfoDelegate.ExecuteIfBound(TEXT("새로운 ID 또는 Password를 입력하세요."), 0, true);

	EditableTextBox_ID->SetText(FText::GetEmpty());
	EditableTextBox_Password->SetText(FText::GetEmpty());

	Border_SignUp->SetVisibility(ESlateVisibility::Visible);
}

void ULobbyWidget::Button_QuitGame_Clicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->ConsoleCommand("quit");
	}
}

void ULobbyWidget::Button_Check_Clicked()
{
	FString NewID = EditableTextBox_NewID->GetText().ToString();
	FString NewPwd = EditableTextBox_NewPassword->GetText().ToString();

	EditableTextBox_NewID->SetText(FText::GetEmpty());
	EditableTextBox_NewPassword->SetText(FText::GetEmpty());

	if (NewID.Len() <= 0 || NewPwd.Len() <= 0)
	{
		LobbyGameMode->LobbyInfoDelegate.ExecuteIfBound(TEXT("새로운 ID 또는 Password를 입력하세요"), 0, false);
		return;
	}

	// IDPwd = "ID:Pwd"
	FString NewIDPwd = NewID;
	NewIDPwd.Append(TEXT(":"));
	NewIDPwd.Append(NewPwd);

	FLoginPacketData PacketData(ELoginPacket::C2S_ReqSignUpIDPwd, NewIDPwd);
	bool bSend = ClientLoginSubsystem->Send(PacketData);
	if (!bSend)
	{
		ABLOG(Error, TEXT("Send Error"));
	}
}

void ULobbyWidget::Button_CancelSignUp_Clicked()
{
	EditableTextBox_NewID->SetText(FText::GetEmpty());
	EditableTextBox_NewPassword->SetText(FText::GetEmpty());

	Border_SignUp->SetVisibility(ESlateVisibility::Collapsed);
}

void ULobbyWidget::Button_CheckNickName_Clicked()
{
	FString NewNickName = EditableTextBox_NewNickName->GetText().ToString();

	EditableTextBox_NewNickName->SetText(FText::GetEmpty());

	if (NewNickName.Len() <= 0)
	{
		LobbyGameMode->LobbyInfoDelegate.ExecuteIfBound(TEXT("새로운 닉네임을 입력하세요"), 0, false);
		return;
	}

	FLoginPacketData PacketData(ELoginPacket::C2S_ReqSignUpNickName, NewNickName);
	bool bSend = ClientLoginSubsystem->Send(PacketData);
	if (!bSend)
	{
		ABLOG(Error, TEXT("Send Error"));
	}
}

void ULobbyWidget::Button_CancelSignUpNickName_Clicked()
{
	EditableTextBox_NewNickName->SetText(FText::GetEmpty());

	Border_SignUp_NickName->SetVisibility(ESlateVisibility::Collapsed);
}
