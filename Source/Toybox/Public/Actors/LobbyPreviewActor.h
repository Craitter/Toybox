// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HelperObjects/ToyboxDataTypes.h"
#include "LobbyPreviewActor.generated.h"

class ULobbyPlayerInfoWidget;
class UWidgetComponent;
enum class EChampion;
struct FChampionData;
class UTextRenderComponent;
class UCapsuleComponent;

UCLASS()
class TOYBOX_API ALobbyPreviewActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALobbyPreviewActor();

	virtual void PreInitializeComponents() override;

	void SetPlayer(const FText& PlayerName, const EChampion& ChampionToPreview, bool bIsReady);
	
	void ClearPreview();

	bool IsAssigned() const;

	void UpdatePlayerName(const FText& PlayerName);
	void UpdateChampion(const EChampion Champion);
	void UpdateIsReady(const bool bIsReady);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void SetNameInfoText() const;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USkeletalMeshComponent> ToyMesh = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USkeletalMeshComponent> RangedMesh = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> MeleeMesh = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCapsuleComponent> CapsuleRoot = {nullptr};

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> PlayerInfoWidget = {nullptr};

	TWeakObjectPtr<ULobbyPlayerInfoWidget> LobbyInfoWidget = {nullptr}; 

private:
	void SetReadyInfoText() const;
	void SetChampionMesh() const;
	void SetChampionWeaponMeshes() const;
	void SetChampionInfoText() const;

	EChampion CurrentChampion = EChampion::None;
	FText CurrentPlayerName = FText::GetEmpty();
	FChampionData* CurrentChampionData = {nullptr};
	
	bool bIsAssigned = false;
	bool bIsOwnerReady = false;

	const FAttachmentTransformRules WeaponAttachmentRules =
			FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
	
	FChampionData* GetChampionData(const EChampion Champion) const;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> ChampionTable = {nullptr};
};


