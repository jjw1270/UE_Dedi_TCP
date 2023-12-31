// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainLobbyWidget.generated.h"

/**
 * 
 */
UCLASS()
class TCPSTUDY1_API UMainLobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_UserNickName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* Button_MatchMaking;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* Button_QuitGame;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* Border_MatchMaking;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_Timer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* Button_CancelMatchMaking;

private:
	FDelegateHandle RecvPacketDelegateHandle;

	UFUNCTION()
	void OnRecvPacketDelegate(const FString& InfoMessage, const int32& PacketCode, bool bSuccess);

// MatchMaking
	int32 Timer;

	FTimerHandle MatchMakingTimerHandle;

	void MatchMakingTimer();

// Button Events
private:
	UFUNCTION()
	void Button_MatchMaking_Clicked();

	UFUNCTION()
	void Button_CancelMatchMaking_Clicked();

	UFUNCTION()
	void Button_QuitGame_Clicked();

private:
	UPROPERTY()
	class UClientLoginSubsystem* ClientLoginSubsystem;

};
