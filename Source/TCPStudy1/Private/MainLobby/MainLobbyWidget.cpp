// Fill out your copyright notice in the Description page of Project Settings.


#include "MainLobbyWidget.h"
#include "TCPStudy1.h"
#include "ClientLoginSubsystem.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

void UMainLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button_MatchMaking->OnClicked.AddDynamic(this, &UMainLobbyWidget::Button_MatchMaking_Clicked);
	Button_QuitGame->OnClicked.AddDynamic(this, &UMainLobbyWidget::Button_QuitGame_Clicked);

	UGameInstance* GI = GetGameInstance();
	CHECK_VALID(GI);

	ClientLoginSubsystem = GI->GetSubsystem<UClientLoginSubsystem>();
	CHECK_VALID(ClientLoginSubsystem);

	ClientLoginSubsystem->RecvPacketDelegate.AddUFunction(this, FName("OnRecvPacketDelegate"));

	FString UserNickNameText = FString::Printf(TEXT("%s 님"), *ClientLoginSubsystem->GetUserNickName());
	TextBlock_UserNickName->SetText(FText::FromString(UserNickNameText));
}

void UMainLobbyWidget::NativeDestruct()
{
	if (ClientLoginSubsystem)
	{
		ClientLoginSubsystem->RecvPacketDelegate.Clear();
	}

	Super::NativeDestruct();
}

void UMainLobbyWidget::OnRecvPacketDelegate(const FString& InfoMessage, const int32& PacketCode, bool bSuccess)
{
	//UWorld* World = GetWorld();
	//CHECK_VALID(World);

	//switch (static_cast<ELoginPacket>(PacketCode))
	//{
	//default:
	//	break;
	//}
}

void UMainLobbyWidget::Button_MatchMaking_Clicked()
{
	FLoginPacketData PacketData(ELoginPacket::C2S_ReqMatchMaking);
	bool bSend = ClientLoginSubsystem->Send(PacketData);
	if (!bSend)
	{
		ABLOG(Error, TEXT("Send Error"));
	}

	// Start Match Making Timer Widget
}

void UMainLobbyWidget::Button_QuitGame_Clicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->ConsoleCommand("quit");
	}
}
