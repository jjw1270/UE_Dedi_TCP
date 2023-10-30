// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class TCPSTUDY1_API FClientLoginThread : public FRunnable
{
public:
	FClientLoginThread(class UClientLoginSubsystem*& NewClientLoginSubsystem);
	~FClientLoginThread();

protected:
	virtual bool Init() override;

	virtual uint32 Run() override;

public:
	virtual void Stop() override;

private:
	class UClientLoginSubsystem* ClientLoginSubsystem;

private:
	bool bStopThread;

	FRunnableThread* Thread;
};
