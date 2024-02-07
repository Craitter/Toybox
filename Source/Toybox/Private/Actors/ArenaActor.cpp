// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ArenaActor.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/LevelStreamingDynamic.h"

// Sets default values
AArenaActor::AArenaActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bFindCameraComponentWhenViewTarget = true;
	ArenaSpace = CreateDefaultSubobject<UBoxComponent>("ArenaSpace");
	Info = CreateDefaultSubobject<UTextRenderComponent>("Info");
	TemplateSelectionCamera = CreateDefaultSubobject<UCameraComponent>("TemplateSelectionCamera");

	if (!ensure(ArenaSpace != nullptr)) return;
	if (!ensure(Info != nullptr)) return;
	if (!ensure(TemplateSelectionCamera != nullptr)) return;

	SetRootComponent(ArenaSpace);
	Info->SetupAttachment(ArenaSpace);
	TemplateSelectionCamera->SetupAttachment(ArenaSpace);

	bReplicates = true;
}

bool AArenaActor::IsAssigned() const
{
	return bIsAssigned;
}

void AArenaActor::SetIsAssigned(const bool bNewIsAssigned)
{
	bIsAssigned = bNewIsAssigned;
}

TSoftObjectPtr<UWorld> AArenaActor::GetDefaultTemplate() const
{
	return DefaultTemplate;
}

TArray<TObjectPtr<APlayerStart>> AArenaActor::GetAttackerPlayerStarts() const
{
	return AttackerPlayerStarts;
}


