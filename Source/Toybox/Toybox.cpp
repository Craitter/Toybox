// Fill out your copyright notice in the Description page of Project Settings.

#include "Toybox.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, Toybox, "Toybox");

DEFINE_LOG_CATEGORY(LogToyboxOnlineService);
DEFINE_LOG_CATEGORY(LogToyboxCamera);
DEFINE_LOG_CATEGORY(LogToyboxAbilitySystem);

int32 LevelStreamIndex = 0;

const FString DefaultLevelInstanceName = FString(TEXT("StreamLevelDynamic"));

// a snippet I found in Lyra it will make logging easier on the long run
FString GetClientServerContextString(UObject* ContextObject)
{
	ENetRole Role = ROLE_None;

	if (const AActor* Actor = Cast<AActor>(ContextObject))
	{
		Role = Actor->GetLocalRole();
	}
	else if (const UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None)
	{
		return (Role == ROLE_Authority) ? TEXT("Server") : TEXT("Client");
	}
	else
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			return GPlayInEditorContextString;
		}
#endif
	}

	return TEXT("[]");
}

FString GetBoolAsString(const bool Value)
{
	return Value ? TEXT("true") : TEXT("false");
}

//Not thread safe
int32 GetUniqueLevelStreamingId()
{
	return LevelStreamIndex++;
}

FString MakeLevelStreamingName(const int32 Index)
{
	return DefaultLevelInstanceName + FString::Printf(TEXT("%d"), Index);
}
