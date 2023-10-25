// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"

/**
 * 
 */

class FSocket;

class TCPSTUDY1_API FRecvThread : public FRunnable
{
public:
	FRecvThread(TSharedPtr<class FSocketManager> SocketManager);
	~FRecvThread();

protected:
	bool Init() override;
	uint32 Run() override;
	void Exit() override;

private:
	TSharedPtr<class FSocketManager> SocketManager;

	FRunnableThread* Thread;

};
