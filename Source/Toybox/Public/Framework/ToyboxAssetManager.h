// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "ToyboxAssetManager.generated.h"

/**
 * Toybox AssetManager
 */
UCLASS()
class TOYBOX_API UToyboxAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:

	UToyboxAssetManager();

	// Returns the AssetManager singleton Object.
	static UToyboxAssetManager& Get();

protected:

	virtual void StartInitialLoading() override;
};
