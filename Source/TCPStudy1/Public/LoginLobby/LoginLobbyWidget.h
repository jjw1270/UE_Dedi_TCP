// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoginLobbyWidget.generated.h"

/**
 * 
 */

class UButton;
class UEditableTextBox;

UCLASS()
class TCPSTUDY1_API ULoginLobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_Info;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* EditableTextBox_ID;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* EditableTextBox_Password;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_SignIn;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_SignUp;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_QuitGame;

	UPROPERTY(meta = (BindWidget))
	class UBorder* Border_SignUp;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* EditableTextBox_NewID;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* EditableTextBox_NewPassword;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* EditableTextBox_ConfirmNewPassword;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Check;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_CancelSignUp;

	UPROPERTY(meta = (BindWidget))
	class UBorder* Border_SignUp_NickName;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* EditableTextBox_NewNickName;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_CheckNickName;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_CancelSignUpNickName;

private:
	FDelegateHandle RecvPacketDelegateHandle;

	UFUNCTION()
	void OnRecvPacketDelegate(const FString& InfoMessage, const int32& PacketCode, bool bSuccess);

	FTimerHandle InfoTextHandle;

	void EnableInputs(bool bEnable);

	void GotoMainLobby();

// Button Events
private:
	UFUNCTION()
	void Button_SignIn_Clicked();

	UFUNCTION()
	void Button_SignUp_Clicked();

	UFUNCTION()
	void Button_QuitGame_Clicked();

	UFUNCTION()
	void Button_Check_Clicked();

	UFUNCTION()
	void Button_CancelSignUp_Clicked();

	UFUNCTION()
	void Button_CheckNickName_Clicked();

	UFUNCTION()
	void Button_CancelSignUpNickName_Clicked();

private:
	UPROPERTY()
	class UClientLoginSubsystem* ClientLoginSubsystem;

};
