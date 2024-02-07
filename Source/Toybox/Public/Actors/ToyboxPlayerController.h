// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "ToyboxPlayerController.generated.h"

class AMatchPlayerState;
class AMatchGameState;
class AMatchSetup;
class AToyboxCharacter;
class UToyboxEnhancedInputComponent;
class UInputMappingContext;
class UToyboxInputConfig;
class ULevelStreamingDynamic;
struct FInputActionValue;

/**
 * 
 */

DECLARE_MULTICAST_DELEGATE_OneParam(FNewTemplateLoadAction, TWeakObjectPtr<ULevelStreamingDynamic>);

UCLASS()
class TOYBOX_API AToyboxPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AToyboxPlayerController();

	virtual void PreInitializeComponents() override;

	virtual void StartSpectatingOnly() override;
	
	virtual void ChangeState(FName NewState) override;

	virtual void OnRep_PlayerState() override;

	void LoadPreparationWidget();
	void LoadBattleWidget();
	void LoadPostBattleWidget();
	void LoadPostGameWidget();

	void RemoveActiveWidget();

	UFUNCTION(Reliable, Server)
	void Server_LoadPreviousTemplate();

	UFUNCTION(Reliable, Server)
	void Server_LoadNextTemplate();

	void EnableDefenderOutline() const;
	
	void EnableAttackerOutline() const;

	FNewTemplateLoadAction OnNewTemplateLoadAction;

	TArray<TWeakObjectPtr<UInputMappingContext>> CurrentAdditionalContexts = {nullptr};

protected:
	/** Input mapping context */	
	UPROPERTY(EditDefaultsOnly, Category = "Input|Context")
	TSoftObjectPtr<UInputMappingContext> IMC_KBM_Base = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Input|Context")
	TSoftObjectPtr<UInputMappingContext> IMC_GamePad_Base = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Input|Context")
	TSoftObjectPtr<UInputMappingContext> IMC_KBM_Preparation = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Input|Context")
	TSoftObjectPtr<UInputMappingContext> IMC_GamePad_Preparation = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Input|Context")
	TSoftObjectPtr<UInputMappingContext> IMC_KBM_Combat = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Input|Context")
	TSoftObjectPtr<UInputMappingContext> IMC_GamePad_Combat = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Widget|Overlay")
	TSubclassOf<UUserWidget> GameOverlayWidgetClass = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Widget|Overlay")
	TSubclassOf<UUserWidget> PreparationWidgetClass = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Widget|Overlay")
	TSubclassOf<UUserWidget> PostBattleWidgetClass = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Widget|Overlay")
	TSubclassOf<UUserWidget> PostGameWidgetClass = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Widget|Overlay")
	TSubclassOf<UUserWidget> TemplateSelectionWidget = {nullptr};

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	virtual void OnRep_Pawn() override;
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void BeginPrePlayState();
	virtual void EndPrePlayState();

	virtual void BeginPlayingState() override;
	
	void UpdateInput(TWeakObjectPtr<UToyboxInputConfig> NewInputConfig);
	/** Called to bind functionality to input */
	virtual void SetupInputComponent() override;

	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	
	/** Handles moving forward/backward */
	void Input_Move(const FInputActionValue& InputActionValue);
 
	/** Handles mouse and stick look */
	void Input_Look(const FInputActionValue& InputActionValue);

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
	void AddInputMapping(const TSoftObjectPtr<UInputMappingContext>& NewContext, int32 Priority);
	void AddBaseInputMapping();
	void AddCombatInputMapping();
	void AddPreparationInputMapping();

	void RemoveActiveMappings() const;
	void RemoveActiveMapping(TWeakObjectPtr<UInputMappingContext> ContextToRemove) const;
	TArray<TWeakObjectPtr<UInputMappingContext>> ActiveInputMappings;

	void OnGameStateSet(AGameStateBase* NewGameState);

	void OnBattleStateChanged(FName NewState);
	
	TWeakObjectPtr<AMatchGameState> MatchGameState = {nullptr};
	
private:
	TWeakObjectPtr<UToyboxEnhancedInputComponent> ToyboxEnhancedInputComponent = {nullptr};
	TWeakObjectPtr<AToyboxCharacter> ToyboxCharacter = {nullptr};	
	TWeakObjectPtr<AMatchSetup> MatchSetup = {nullptr};
	TWeakObjectPtr<UUserWidget> ActiveWidget = {nullptr};
	
	TArray<uint32> NativeBindHandles;
	TArray<uint32> AbilityBindHandles;

public:
};
