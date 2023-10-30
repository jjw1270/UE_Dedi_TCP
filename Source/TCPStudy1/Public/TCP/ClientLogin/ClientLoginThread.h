// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class TCPSTUDY1_API FClientLoginThread : public FRunnable
{
public:
	//FClientLoginThread(class UClientLoginSubsystem*& NewClientLoginSubsystem);
	FClientLoginThread(class UClientLoginSubsystem* NewClientLoginSubsystem);

protected:
	virtual uint32 Run() override;

public:
	void StopThread();

private:
	class UClientLoginSubsystem* ClientLoginSubsystem;

private:
	bool bStopThread;

};
