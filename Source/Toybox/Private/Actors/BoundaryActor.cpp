// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/BoundaryActor.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Toybox/Toybox.h"


// Sets default values
ABoundaryActor::ABoundaryActor()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");


	if (!ensure(IsValid(StaticMeshComponent))) return;
	
	SetRootComponent(StaticMeshComponent);
}

// Called when the game starts or when spawned
void ABoundaryActor::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(BoundaryMaterial))
	{
		const TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial =
			UMaterialInstanceDynamic::Create(BoundaryMaterial, this);
		
		StaticMeshComponent->SetMaterial(0, DynamicMaterial);
		
		GetWorldTimerManager().SetTimer(PlayerPositionUpdateTimer, this, &ThisClass::UpdatePlayerPosition, TimerFrequency, true, 0.0f);
	}
}

bool ABoundaryActor::SetLocalPlayerCharacter()
{
	if (PlayerPawn.IsValid())
	{
		return true;
	}

	const TWeakObjectPtr<APlayerController> PlayerController = UGameplayStatics::GetPlayerController(this, LOCAL_USER_INDEX);

	check(PlayerController.Get());
	
	PlayerPawn = PlayerController->GetPawn();

	if (PlayerPawn.IsValid())
	{
		return true;
	}

	return false;
}

void ABoundaryActor::UpdatePlayerPosition()
{
	if (!SetLocalPlayerCharacter())
	{
		return;
	}
	
	StaticMeshComponent->SetVectorParameterValueOnMaterials(MaterialParameterName, PlayerPawn->GetActorLocation());
}

