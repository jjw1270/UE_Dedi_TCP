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

	bool Connect(const int32 PortNum);

private:
	FSocket* Socket;

};
