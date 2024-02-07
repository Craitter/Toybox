// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/Actor.h"
#include "ArenaActor.generated.h"

class APlayerStart;
class UCameraComponent;
class UTextRenderComponent;
class UBoxComponent;

UCLASS()
class TOYBOX_API AArenaActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AArenaActor();
	
	bool IsAssigned() const;

	void SetIsAssigned(const bool bNewIsAssigned);

	TSoftObjectPtr<UWorld> GetDefaultTemplate() const;

	TArray<TObjectPtr<APlayerStart>> GetAttackerPlayerStarts() const;
	
protected:
	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<UBoxComponent> ArenaSpace = {nullptr};

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextRenderComponent> Info = {nullptr};

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TemplateSelectionCamera = {nullptr};

	UPROPERTY(EditInstanceOnly)
	TSoftObjectPtr<UWorld> DefaultTemplate = {nullptr};

	UPROPERTY(EditInstanceOnly)
	TArray<TObjectPtr<APlayerStart>> AttackerPlayerStarts = {nullptr};
	
private:
	bool bIsAssigned = false;
	
};
