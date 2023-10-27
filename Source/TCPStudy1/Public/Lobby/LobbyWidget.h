// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

/**
 * 
 */

class UBorder;
class UButton;
class UEditableTextBox;

UCLASS()
class TCPSTUDY1_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_Info;

	UPROPERTY(meta = (BindWidget))
	UBorder* Border_OnConnect;

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

private:
	UFUNCTION()
	void OnLobbyInfoDelegate(const FString& InfoMessage);

	FTimerHandle InfoTextHandle;

	UFUNCTION()
	void HideInfoText();


};
