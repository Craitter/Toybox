// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HelperObjects/ToyboxDataTypes.h"
#include "ToyboxPlayerState.generated.h"

class AToyboxGameState;


/**
 * 
 */
UCLASS()
class TOYBOX_API AToyboxPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AToyboxPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Server_TogglePlayerIsReady();
	
	virtual void BeginPlay() override;

	virtual void CopyProperties(APlayerState* PlayerState) override;

	UFUNCTION()
	virtual void OnRep_SelectedChampion() {}

	void SetSelectedChampion(const EChampion NewChampion);
protected:
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_SelectedChampion)
	EChampion SelectedChampion = EChampion::None;
	
public:
	TWeakObjectPtr<AToyboxGameState> GameState = {nullptr};

	bool bPreviewActorAssigned = false;
};
