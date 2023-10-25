// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"

/**
 * 
 */

class FSocket;

class TCPSTUDY1_API RecvThread : public FRunnable
{
public:
	RecvThread(class SocketManager* NewSocketManager);
	~RecvThread();

protected:
	bool Init() override;
	uint32 Run() override;
	void Exit() override;

private:
	class SocketManager* pSocketManager;

	FRunnableThread* Thread;

};
