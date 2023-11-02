// Fill out your copyright notice in the Description page of Project Settings.


#include "MainLobbyWidget.h"
#include "TCPStudy1.h"
#include "ClientLoginSubsystem.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UMainLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button_MatchMaking->OnClicked.AddDynamic(this, &UMainLobbyWidget::Button_MatchMaking_Clicked);
	Button_QuitGame->OnClicked.AddDynamic(this, &UMainLobbyWidget::Button_QuitGame_Clicked);
	Button_CancelMatchMaking->OnClicked.AddDynamic(this, &UMainLobbyWidget::Button_CancelMatchMaking_Clicked);

	UGameInstance* GI = GetGameInstance();
	CHECK_VALID(GI);
	ClientLoginSubsystem = GI->GetSubsystem<UClientLoginSubsystem>();
	CHECK_VALID(ClientLoginSubsystem);
	RecvPacketDelegateHandle = ClientLoginSubsystem->RecvPacketDelegate.AddUFunction(this, FName("OnRecvPacketDelegate"));

	FString UserNickNameText = FString::Printf(TEXT("%s 님"), *ClientLoginSubsystem->GetUserNickName());
	TextBlock_UserNickName->SetText(FText::FromString(UserNickNameText));
}

void UMainLobbyWidget::NativeDestruct()
{
	if (ClientLoginSubsystem && RecvPacketDelegateHandle.IsValid())
	{
		ClientLoginSubsystem->RecvPacketDelegate.Remove(RecvPacketDelegateHandle);
	}

	if (MatchMakingTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(MatchMakingTimerHandle);
	}

	Super::NativeDestruct();
}

void UMainLobbyWidget::OnRecvPacketDelegate(const FString& InfoMessage, const int32& PacketCode, bool bSuccess)
{
	UWorld* World = GetWorld();
	CHECK_VALID(World);

	switch (static_cast<ELoginPacket>(PacketCode))
	{
	case ELoginPacket::S2C_ResMatchMaking_DediIP:
	{
		ABLOG(Warning, TEXT("Dedi Server IP : %s"), *InfoMessage);
		UGameplayStatics::OpenLevel(World, FName(InfoMessage));
	}
	break;
	default:
		break;
	}
}

void UMainLobbyWidget::MatchMakingTimer()
{
	Timer++;

	FString TimerFormat = FString::Printf(TEXT("%02d : %02d"), Timer / 60, Timer % 60);
	TextBlock_Timer->SetText(FText::FromString(TimerFormat));
}

void UMainLobbyWidget::Button_MatchMaking_Clicked()
{
	FLoginPacketData PacketData(ELoginPacket::C2S_ReqMatchMaking);
	bool bSend = ClientLoginSubsystem->Send(PacketData);
	if (!bSend)
	{
		ABLOG(Error, TEXT("Send Error"));
	}

	// Start Match Making Timer
	Border_MatchMaking->SetVisibility(ESlateVisibility::Visible);
	GetWorld()->GetTimerManager().SetTimer(MatchMakingTimerHandle, this, &UMainLobbyWidget::MatchMakingTimer, 1.f, true);
}

void UMainLobbyWidget::Button_CancelMatchMaking_Clicked()
{
	FLoginPacketData PacketData(ELoginPacket::C2S_ReqCancelMatchMaking);
	bool bSend = ClientLoginSubsystem->Send(PacketData);
	if (!bSend)
	{
		ABLOG(Error, TEXT("Send Error"));
	}

	GetWorld()->GetTimerManager().ClearTimer(MatchMakingTimerHandle);
	Border_MatchMaking->SetVisibility(ESlateVisibility::Collapsed);
	Timer = 0;
	TextBlock_Timer->SetText(FText::FromString(TEXT("00 : 00")));
}

void UMainLobbyWidget::Button_QuitGame_Clicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->ConsoleCommand("quit");
	}
}
