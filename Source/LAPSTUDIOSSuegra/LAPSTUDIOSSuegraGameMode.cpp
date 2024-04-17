// Copyright Epic Games, Inc. All Rights Reserved.

#include "LAPSTUDIOSSuegraGameMode.h"
#include "LAPSTUDIOSSuegraCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALAPSTUDIOSSuegraGameMode::ALAPSTUDIOSSuegraGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
