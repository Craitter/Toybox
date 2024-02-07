// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ToyboxPlayerController.h"
#include "Framework/ToyboxPlayerCameraManager.h"
#include "EnhancedInputSubsystems.h"
#include "Actors/MatchGameMode.h"
#include "Actors/MatchGameState.h"
#include "Actors/MatchPlayerState.h"
#include "Actors/MatchSetup.h"
#include "Actors/ToyboxCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Framework/ToyboxAbilitySystemComponent.h"
#include "Framework/ToyboxEnhancedInputComponent.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"


AToyboxPlayerController::AToyboxPlayerController()
{
	PlayerCameraManagerClass = AToyboxPlayerCameraManager::StaticClass();
}

void AToyboxPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	
	AddBaseInputMapping();
	AddCombatInputMapping();
	
	const TWeakObjectPtr<AGameStateBase> GameState = GetWorld()->GetGameState();
	if (GameState.IsValid())
	{
		OnGameStateSet(GameState.Get());
	}
	else
	{
		GetWorld()->GameStateSetEvent.AddUObject(this, &AToyboxPlayerController::OnGameStateSet);
	}
}

void AToyboxPlayerController::StartSpectatingOnly()
{
	Super::StartSpectatingOnly();

	if (GetSpectatorPawn() == nullptr)
	{
		BeginSpectatingState();
	}	
}

void AToyboxPlayerController::ChangeState(FName NewState)
{
	if (NewState != StateName)
	{
		// end current state
		if (StateName == NAME_PrePlay)
		{
			EndPrePlayState();
		}
		// Will set StateName, also handles Inactive, Play and spectating state
		Super::ChangeState(NewState); 

		// start new state
		if (StateName == NAME_PrePlay)
		{
			BeginPrePlayState();
		}
	}
}

//Server Only
void AToyboxPlayerController::OnPossess(APawn* InPawn)
{
	bAutoManageActiveCameraTarget = true;
	
	Super::OnPossess(InPawn);
	
	ToyboxCharacter = Cast<AToyboxCharacter>(InPawn);

	//UE_LOG(LogTemp, Warning, TEXT("IsDefender = %s"), *GetBoolAsString())
	
	if (GetPlayerState<AMatchPlayerState>()->IsDefenderTeam())
	{
		EnableDefenderOutline();
	}
	else if (GetPlayerState<AMatchPlayerState>()->IsAttackerTeam())
	{
		EnableAttackerOutline();
	}
}

void AToyboxPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	ToyboxCharacter = nullptr;
}

void AToyboxPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	ToyboxCharacter = Cast<AToyboxCharacter>(GetPawn());
}

//Client Only
void AToyboxPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (IsLocalController())
	{
		const TWeakObjectPtr<AMatchPlayerState> ToyboxPlayerState = GetPlayerState<AMatchPlayerState>();
		if (ToyboxPlayerState.IsValid())
		{
			ToyboxPlayerState->OnInputConfigSet.BindUObject(this, &ThisClass::UpdateInput);
			UpdateInput(ToyboxPlayerState->GetInputConfig());
		}
	}
}

void AToyboxPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AToyboxPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ToyboxEnhancedInputComponent.IsValid())
	{
		ToyboxEnhancedInputComponent->RemoveBinds(NativeBindHandles);
		ToyboxEnhancedInputComponent->RemoveBinds(AbilityBindHandles);
	}
	Super::EndPlay(EndPlayReason);
}

void AToyboxPlayerController::BeginPrePlayState()
{
	if (IsLocalController())
	{
		ActiveWidget = CreateWidget<UUserWidget>(this, TemplateSelectionWidget);
		if (ActiveWidget.IsValid())
		{
			ActiveWidget->AddToViewport();
			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(ActiveWidget->TakeWidget());
			SetInputMode(Mode);
			SetShowMouseCursor(true);
		}
	}
}

void AToyboxPlayerController::EndPrePlayState()
{
}

void AToyboxPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();
}

void AToyboxPlayerController::UpdateInput(const TWeakObjectPtr<UToyboxInputConfig> NewInputConfig)
{
	if (!ensure(ToyboxEnhancedInputComponent.IsValid())) return;
	
	if (!NewInputConfig.IsValid())
	{
		ToyboxEnhancedInputComponent->RemoveBinds(NativeBindHandles);
		ToyboxEnhancedInputComponent->RemoveBinds(AbilityBindHandles);
	}
	else
	{
		ToyboxEnhancedInputComponent->BindAbilityActions(NewInputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, AbilityBindHandles);

		const FToyboxGameplayTags& GameplayTags = FToyboxGameplayTags::Get();
		//Bind Native Input actions by tag
		ToyboxEnhancedInputComponent->BindNativeAction(NewInputConfig, GameplayTags.InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, false, NativeBindHandles);
		ToyboxEnhancedInputComponent->BindNativeAction(NewInputConfig, GameplayTags.InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_Look, false, NativeBindHandles);
		ToyboxEnhancedInputComponent->BindNativeAction(NewInputConfig, GameplayTags.InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_Look, false, NativeBindHandles);
	}
}

void AToyboxPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	//Make sure to set your input component class in the InputSettings->DefaultClasses
	ToyboxEnhancedInputComponent = Cast<UToyboxEnhancedInputComponent>(InputComponent);

	if (IsLocalController() && HasAuthority())
	{
		const TWeakObjectPtr<AMatchPlayerState> ToyboxPlayerState = GetPlayerState<AMatchPlayerState>();
		if (ToyboxPlayerState.IsValid())
		{
			ToyboxPlayerState->OnInputConfigSet.BindUObject(this, &ThisClass::UpdateInput);
			UpdateInput(ToyboxPlayerState->GetInputConfig());
		}
	}
}

void AToyboxPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);

	if (ToyboxCharacter.IsValid())
	{
		const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent = ToyboxCharacter->GetToyboxAbilitySystemComponent();
		if (AbilitySystemComponent.IsValid())
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(AToyboxPlayerController::PostProcessInput)

			AbilitySystemComponent->ProcessAbilityInput(DeltaTime, bGamePaused);
		}
	}
}


// ReSharper disable once CppMemberFunctionMayBeConst
void AToyboxPlayerController::Input_Move(const FInputActionValue& InputActionValue)
{
	if (ToyboxCharacter.IsValid() && ToyboxCharacter->IsAlive())
	{
		const FVector2D MoveValue = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, GetControlRotation().Yaw, 0.0f);

		if (MoveValue.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			ToyboxCharacter->AddMovementInput(MovementDirection, MoveValue.X);
		}

		if (MoveValue.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			ToyboxCharacter->AddMovementInput(MovementDirection, MoveValue.Y);
		}
	}
}
 
void AToyboxPlayerController::Input_Look(const FInputActionValue& InputActionValue)
{
	if (ToyboxCharacter.IsValid() && ToyboxCharacter->IsAlive())
	{
		// input is a Vector2D
		const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

		// add yaw and pitch input
		AddYawInput(LookAxisVector.X);
		AddPitchInput(LookAxisVector.Y);
	}
}


// ReSharper disable once CppMemberFunctionMayBeConst
void AToyboxPlayerController::Input_AbilityInputTagPressed(const FGameplayTag InputTag)
{
	if (ToyboxCharacter.IsValid())
	{
		const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent = ToyboxCharacter->GetToyboxAbilitySystemComponent();
		if (AbilitySystemComponent.IsValid())
		{
			AbilitySystemComponent->OnAbilityInputTagTriggered(InputTag, true);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s %s() AbilitySystemComponent of the Character is not Valid"), *GetClientServerContextString(), *FString(__FUNCTION__));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s %s() No Valid Character to Forward Input to"), *GetClientServerContextString(), *FString(__FUNCTION__));
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AToyboxPlayerController::Input_AbilityInputTagReleased(const FGameplayTag InputTag)
{
	if (ToyboxCharacter.IsValid())
	{
		const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent = ToyboxCharacter->GetToyboxAbilitySystemComponent();
		if (AbilitySystemComponent.IsValid())
		{
			AbilitySystemComponent->OnAbilityInputTagTriggered(InputTag, false);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s %s() AbilitySystemComponent of the Character is not Valid"), *GetClientServerContextString(), *FString(__FUNCTION__));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s %s() No Valid Character to Forward Input to"), *GetClientServerContextString(), *FString(__FUNCTION__));
	}
}

void AToyboxPlayerController::AddInputMapping(const TSoftObjectPtr<UInputMappingContext>& NewContext,
                                              const int32 Priority) 
{
	using EnhInputSub = UEnhancedInputLocalPlayerSubsystem;
	const TWeakObjectPtr<EnhInputSub> Subsystem = ULocalPlayer::GetSubsystem<EnhInputSub>(GetLocalPlayer());
	if (Subsystem.IsValid())
	{
		if (NewContext.IsNull())
		{
			UE_LOG(LogTemp, Error, TEXT("One or more input mapping context(s) not defined! Check BP controller"));
		}
		else
		{
			Subsystem->AddMappingContext(ActiveInputMappings.Add_GetRef(NewContext.LoadSynchronous()).Get(), Priority);
		}
	}
}

void AToyboxPlayerController::AddBaseInputMapping()
{
	AddInputMapping(IMC_KBM_Base.LoadSynchronous(), 0);
	AddInputMapping(IMC_GamePad_Base.LoadSynchronous(), 0);
}

void AToyboxPlayerController::AddCombatInputMapping()
{
	AddInputMapping(IMC_KBM_Combat.LoadSynchronous(), 1);
	AddInputMapping(IMC_GamePad_Combat.LoadSynchronous(), 1);
}

void AToyboxPlayerController::AddPreparationInputMapping()
{
	AddInputMapping(IMC_KBM_Preparation.LoadSynchronous(), 1);
	AddInputMapping(IMC_GamePad_Preparation.LoadSynchronous(), 1);
}

void AToyboxPlayerController::RemoveActiveMappings() const
{
	for (const TWeakObjectPtr<UInputMappingContext> InputMapping : ActiveInputMappings)
	{
		RemoveActiveMapping(InputMapping);
	}
}

void AToyboxPlayerController::RemoveActiveMapping(const TWeakObjectPtr<UInputMappingContext> ContextToRemove) const
{
	using EnhInputSub = UEnhancedInputLocalPlayerSubsystem;
	const TWeakObjectPtr<EnhInputSub> Subsystem = ULocalPlayer::GetSubsystem<EnhInputSub>(GetLocalPlayer());
	if (Subsystem.IsValid())
	{
		if (ContextToRemove.IsValid())
		{
			Subsystem->RemoveMappingContext(ContextToRemove.Get());
		}
	}
}

void AToyboxPlayerController::OnGameStateSet(AGameStateBase* NewGameState)
{
	MatchGameState = CastChecked<AMatchGameState>(NewGameState);
	if (MatchGameState.IsValid())
	{
		MatchGameState->OnBattleStateChanged.AddUObject(this, &AToyboxPlayerController::OnBattleStateChanged);
	}
	MatchSetup = MatchGameState->MatchSetup;

	if (IsValid(PlayerState) && PlayerState->IsOnlyASpectator())
	{
		BeginSpectatingState();
	}
}

void AToyboxPlayerController::OnBattleStateChanged(FName NewState)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AToyboxPlayerController::OnBattleStateChanged)

	if (!IsLocalController())
	{
		return;
	}
	RemoveActiveMappings();
	RemoveActiveWidget();

	if (IsValid(PlayerState) && PlayerState->IsOnlyASpectator())
	{
		LoadPreparationWidget();
		return;
	}
	
	const FName CurrentBattleState = MatchGameState->GetBattleState();
	if (CurrentBattleState == BattleState::Preparation)
	{
		AddBaseInputMapping();
		AddPreparationInputMapping();
		
		LoadPreparationWidget();
	}
	else if (CurrentBattleState == BattleState::Battle)
	{
		AddBaseInputMapping();
		AddCombatInputMapping();
		
		LoadBattleWidget();
	}
	else if (CurrentBattleState == BattleState::PostBattle)
	{
		LoadPostBattleWidget();
	}
	else if (CurrentBattleState == BattleState::PostGame)
	{
		LoadPostGameWidget();
	}
}

void AToyboxPlayerController::LoadPreparationWidget()
{
	ActiveWidget = CreateWidget(this, PreparationWidgetClass);
	if (ActiveWidget.IsValid())
	{
		ActiveWidget->AddToViewport();
		FInputModeGameOnly GameOnly;
		GameOnly.SetConsumeCaptureMouseDown(true);
		SetInputMode(GameOnly);
	}
}

void AToyboxPlayerController::LoadBattleWidget()
{
	ActiveWidget = CreateWidget(this, GameOverlayWidgetClass);
	if (ActiveWidget.IsValid())
	{
		ActiveWidget->AddToViewport();
		FInputModeGameOnly GameOnly;
		GameOnly.SetConsumeCaptureMouseDown(true);
		SetInputMode(GameOnly);
	}
}

void AToyboxPlayerController::LoadPostBattleWidget()
{
	ActiveWidget = CreateWidget(this, PostBattleWidgetClass);
	if (ActiveWidget.IsValid())
	{
		ActiveWidget->AddToViewport();
		FInputModeGameOnly GameOnly;
		GameOnly.SetConsumeCaptureMouseDown(true);
		SetInputMode(GameOnly);
	}
}

void AToyboxPlayerController::LoadPostGameWidget()
{
	ActiveWidget = CreateWidget(this, PostGameWidgetClass);
	if (ActiveWidget.IsValid())
	{
		ActiveWidget->AddToViewport();
		FInputModeGameOnly GameOnly;
		GameOnly.SetConsumeCaptureMouseDown(true);
		SetInputMode(GameOnly);
	}
}

void AToyboxPlayerController::RemoveActiveWidget()
{
	if (ActiveWidget.IsValid())
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr;
		SetShowMouseCursor(false);
	}
}

void AToyboxPlayerController::EnableDefenderOutline() const
{
	if (ToyboxCharacter.IsValid())
	{
		ToyboxCharacter->SetShowDefenderOutline(true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No character exists for this controller! :("))
	}
}

void AToyboxPlayerController::EnableAttackerOutline() const
{
	if (ToyboxCharacter.IsValid())
	{
		ToyboxCharacter->SetShowAttackerOutline(true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No character exists for this controller! :("))
	}
}

void AToyboxPlayerController::Server_LoadPreviousTemplate_Implementation()
{
	MatchSetup->LoadPreviousTemplate(this);
}

void AToyboxPlayerController::Server_LoadNextTemplate_Implementation()
{
	MatchSetup->LoadNextTemplate(this);
}


