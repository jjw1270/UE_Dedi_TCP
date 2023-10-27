// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ClientLoginSubsystem.generated.h"

/**
 * 
 */

UCLASS()
class TCPSTUDY1_API UClientLoginSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UClientLoginSubsystem();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;
	
public:
	bool Connect(const int32& PortNum, const FString& IP);

	void DestroySocket();

	bool Recv(struct FClientLoginPacketData& OutRecvPacket);

	bool IsConnect();

protected:
	void PrintSocketError(const FString& Text);

private:
	int32 HeaderSize{ 4 };

	FSocket* Socket;

};
