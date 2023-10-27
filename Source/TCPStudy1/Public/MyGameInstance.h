// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SocketManager.h"
#include "MyGameInstance.generated.h"

/**
 *
 */
UCLASS()
class TCPSTUDY1_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

private:
	TSharedPtr<FSocketManager> SocketManager;

	//class FRecvThread* RecvThread;
	//class FSendThread* SendThread;

protected:
	virtual void Shutdown() override;

public:
	void InitSocketManager();

	FORCEINLINE TSharedPtr<FSocketManager> GetSocketManager() { return SocketManager; };

	FString RecvClientLoginSocket();

	//void StartRecvThread();
	//void StartSendThread();

	//void SendPacket(const EPacket& PacketType, const FString& Payload);
	//void SendPacket(const EPacket& PacketType);
};
