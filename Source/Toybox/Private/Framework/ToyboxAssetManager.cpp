// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ToyboxAssetManager.h"
#include "Framework/ToyboxAbilitySystemGlobals.h"
#include "HelperObjects/ToyboxGameplayTags.h"

UToyboxAssetManager::UToyboxAssetManager()
{
}

UToyboxAssetManager& UToyboxAssetManager::Get()
{
	check(GEngine);

	TObjectPtr<UToyboxAssetManager> ToyBoxAssetManager = Cast<UToyboxAssetManager>(GEngine->AssetManager);
	return *ToyBoxAssetManager.Get();
}

void UToyboxAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	//Load Native Tags
	FToyboxGameplayTags::InitializeNativeTags();
	UToyboxAbilitySystemGlobals::Get().InitGlobalData();
}
