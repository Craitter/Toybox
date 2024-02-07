// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "ToyboxPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API AToyboxPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	AToyboxPlayerStart(const FObjectInitializer& ObjectInitializer);
	
	void SetMesh(TWeakObjectPtr<UStaticMesh> DefenderSpawnMesh);

	bool IsPlayerStartInitialized() const;
	
protected:
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<UStaticMeshComponent> ToylineDefenderSpawnMesh = {nullptr};

	bool bIsInitialized = false;
};
