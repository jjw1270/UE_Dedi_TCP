// Copyright Epic Games, Inc. All Rights Reserved.

#include "TCPStudy1GameMode.h"
#include "TCPStudy1Character.h"
#include "UObject/ConstructorHelpers.h"

ATCPStudy1GameMode::ATCPStudy1GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
