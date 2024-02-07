// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoundaryActor.generated.h"

UCLASS()
class TOYBOX_API ABoundaryActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABoundaryActor();

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMaterialInterface> BoundaryMaterial = {nullptr};
	
	UPROPERTY(EditDefaultsOnly)
	FName MaterialParameterName = TEXT("Player Position");

	UPROPERTY(EditDefaultsOnly)
	float TimerFrequency = 0.2f;
	
protected:
	FTimerHandle PlayerPositionUpdateTimer;

	TWeakObjectPtr<APawn> PlayerPawn = {nullptr};
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool SetLocalPlayerCharacter();
	
public:
	void UpdatePlayerPosition();
};
