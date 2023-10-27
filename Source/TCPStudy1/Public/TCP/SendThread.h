// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PacketMaker.h"
#include "HAL/Runnable.h"

/**
 * 
 */
class TCPSTUDY1_API FSendThread : public FRunnable
{
public:
	FSendThread(TSharedPtr<class FSocketManager> SocketManager);
	~FSendThread();

	void SetSendPacket(const EPacket& PacketType, const FString& Payload);

protected:
	bool Init() override;
	uint32 Run() override;
	void Exit() override;

private:
	TSharedPtr<class FSocketManager> SocketManager;

	FRunnableThread* Thread;

	FPacketData SendPacket;

};
