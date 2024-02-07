// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ToyboxGameMode.h"

#include "Kismet/GameplayStatics.h"
#include "Toybox/Toybox.h"

#if WITH_EDITORONLY_DATA
static FAutoConsoleCommand CVarEndMatch(TEXT("Toybox.EndMatch"),
	TEXT("StartMatch\n"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		const FWorldContext* Context = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
		const TWeakObjectPtr<UWorld> World = Context->World();
		if (!World.IsValid()) return;

		const TWeakObjectPtr<AGameMode> GameMode = Cast<AGameMode>(UGameplayStatics::GetGameMode(World.Get()));
		if (GameMode == nullptr)
		{
			const TWeakObjectPtr<APlayerController> PlayerController = UGameplayStatics::GetPlayerController(World.Get(), LOCAL_USER_INDEX);
			if (!PlayerController.IsValid()) return;
			PlayerController->ServerExec("Toybox.EndMatch");
		}
		else
		{
			GameMode->EndMatch();
		}
	}),
	ECVF_Default);

static FAutoConsoleCommand CVarStartMatch(TEXT("Toybox.StartMatch"),
	TEXT("StartMatch\n"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		const FWorldContext* Context = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
		const TWeakObjectPtr<UWorld> World = Context->World();
		if (!World.IsValid()) return;

		const TWeakObjectPtr<AGameMode> GameMode = Cast<AGameMode>(UGameplayStatics::GetGameMode(World.Get()));
		if (GameMode == nullptr)
		{
			const TWeakObjectPtr<APlayerController> PlayerController = UGameplayStatics::GetPlayerController(World.Get(), LOCAL_USER_INDEX);
			if (!PlayerController.IsValid()) return;
			PlayerController->ServerExec("Toybox.StartMatch");
		}
		else
		{
			GameMode->StartMatch();
		}
	}),
	ECVF_Default);
#endif