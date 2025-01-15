// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiplayerPluginsGameMode.h"
#include "MultiplayerPluginsCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMultiplayerPluginsGameMode::AMultiplayerPluginsGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
