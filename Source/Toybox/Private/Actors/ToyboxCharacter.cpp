// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ToyboxCharacter.h"

#include "Actors/MatchGameMode.h"
#include "Actors/MatchGameState.h"
#include "Actors/MatchPlayerState.h"
#include "Actors/MeleeWeapon.h"
#include "Actors/RangedWeapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Framework/ToyboxAbilitySystemComponent.h"
#include "Framework/ToyboxCharacterMovementComponent.h"
#include "Framework/AttributeSets/HealthAttributeSet.h"
#include "GameFramework/SpringArmComponent.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Toybox/Toybox.h"

// Sets default values
AToyboxCharacter::AToyboxCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UToyboxCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
#if ENABLE_DRAW_DEBUG
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
#else
	PrimaryActorTick.bCanEverTick = false;
#endif
	
	bReplicates = true;

	ToyboxCharacterMovementComponent = Cast<UToyboxCharacterMovementComponent>(GetCharacterMovement());
	
	// Create components then ensure they're valid
	RangedChildActor = CreateDefaultSubobject<UChildActorComponent>("RangedWeaponActor");
	MeleeChildActor = CreateDefaultSubobject<UChildActorComponent>("MeleeWeaponActor");
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	

	if (!ensure(IsValid(RangedChildActor))) return;
	if (!ensure(IsValid(MeleeChildActor))) return;
	if (!ensure(IsValid(SpringArm))) return;
	if (!ensure(IsValid(FollowCamera))) return;

	RangedChildActor->SetChildActorClass(ARangedWeapon::StaticClass());
	RangedChildActor->SetupAttachment(GetMesh(), TEXT("socket_rangedhost"));
	RangedChildActor->SetIsReplicated(false);
	MeleeChildActor->SetChildActorClass(AMeleeWeapon::StaticClass());
	MeleeChildActor->SetupAttachment(GetMesh(), TEXT("socket_meleehost"));
	MeleeChildActor->SetIsReplicated(false);

	
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	SpringArm->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm


	//WeaponAttachmentRules = 
}

void AToyboxCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	OnDeath.BindUObject(this, &ThisClass::OnDeathEvent);
	
	if (IsValid(RangedChildActor))
	{
		EquippedRanged = Cast<ARangedWeapon>(RangedChildActor->GetChildActor());
	}
	if (IsValid(RangedChildActor))
	{
		EquippedMelee = Cast<AMeleeWeapon>(MeleeChildActor->GetChildActor());
	}
}

//Client Only
void AToyboxCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	ToyboxPlayerState = GetPlayerState<AMatchPlayerState>();
	if (!ToyboxPlayerState.IsValid())
	{
		// This gets replicated once on death but I dont know why!
		return;
	}
	InitializeChampionData();
	InitializeAbilitySystem();
}

//Server Only
void AToyboxCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	ToyboxPlayerState = GetPlayerState<AMatchPlayerState>();
	if (!ensure(ToyboxPlayerState.IsValid())) return;
	InitializeChampionData();
	InitializeAbilitySystem();
}

void AToyboxCharacter::UnPossessed()
{
	Super::UnPossessed();
}

void AToyboxCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
}

void AToyboxCharacter::Falling()
{
	if (ToyboxCharacterMovementComponent->bJustTeleported)
	{
		return;
	}
}

void AToyboxCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	check(ToyboxCharacterMovementComponent)
	if (PrevMovementMode == MOVE_Falling && ToyboxCharacterMovementComponent->IsMovingOnGround())
	{
		CleanupAirborneTag();
	}
	else if (ToyboxCharacterMovementComponent->IsFalling())
	{
		if (AbilitySystemComponent.IsValid())
		{
			const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
			if (IsLocallyControlled() || !HasAuthority())
			{
				AbilitySystemComponent->AddLooseGameplayTag(NativeTags.MovementState_Airborne);
			}
			else
			{
				AbilitySystemComponent->AddReplicatedLooseGameplayTag(NativeTags.MovementState_Airborne);
			}	
		}
	}
}

void AToyboxCharacter::CleanupAirborneTag() const
{
	if (AbilitySystemComponent.IsValid())
	{
		const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
		AbilitySystemComponent->SetLooseGameplayTagCount(NativeTags.MovementState_Airborne, 0);
		AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(NativeTags.MovementState_Airborne, 0);
	}
}

void AToyboxCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if ENABLE_DRAW_DEBUG
	// const TWeakObjectPtr<AMatchPlayerState> MatchPlayerState = GetPlayerState<AMatchPlayerState>();
	// if (MatchPlayerState.IsValid())
	// {
	// 	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	// 	FGameplayTag TeamTag = FGameplayTag::EmptyTag;
	// 	if (MatchPlayerState->IsAttackerTeam())
	// 	{
	// 		TeamTag = NativeTags.Team_Attacker;
	// 	}
	// 	else if (MatchPlayerState->IsDefenderTeam())
	// 	{
	// 		TeamTag = NativeTags.Team_Defender;
	// 	}
	//
	// 	if (TeamTag != FGameplayTag::EmptyTag)
	// 	{
	// 		DrawDebugString(GetWorld(), FVector(0, 0, 100), TeamTag.ToString(),
	// 			this, FColor::Green, DeltaSeconds);
	// 	}
	// }
	
	
#endif
}

void AToyboxCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, bShowDefenderOutline, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, bShowAttackerOutline, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, bRagdollPlayer, COND_None, REPNOTIFY_Always);
}

// Called when the game starts or when spawned
void AToyboxCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AToyboxCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AToyboxCharacter::OnDeathEvent()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AToyboxCharacter::OnDeath)
	
	if (IsValid(Controller))
	{
		Controller->UnPossess();
	}

	// Set life span of this character as it is now dead!
	SetLifeSpan(CharacterChampionData->DeadRagdollLifeSpan);

	RagdollCharacter();
	bRagdollPlayer = true;

	ToyboxCharacterMovementComponent->DisableMovement();
	GetCapsuleComponent()->Deactivate();
}

void AToyboxCharacter::RagdollCharacter()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AToyboxCharacter::RagdollCharacter)
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetActorEnableCollision(true);
	GetMesh()->SetAllBodiesBelowSimulatePhysics(NAME_None, true);
}

void AToyboxCharacter::Destroyed()
{
	Super::Destroyed();
}

void AToyboxCharacter::OnDeadTagChanged(FGameplayTag ChangedTag, const int32 NewCount)
{
	bIsAlive = NewCount <= 0;
	if (!bIsAlive)
	{
		OnDeathEvent();
	}
}

void AToyboxCharacter::OnWeaponTagChanged(const FGameplayTag ChangedTag, const int32 NewCount)
{
	if (ChangedTag == CurrentWeaponTag && NewCount <= 0)
	{
		UnEquipCurrentWeapon();
	}
	
	//New Count means that on the ASC is the Tag now X times
	if (NewCount > 0)
	{
		CurrentWeaponTag = ChangedTag;
		EquipCurrentWeapon();
	}
}

void AToyboxCharacter::OnSlowTagChanged(const FGameplayTag ChangedTag, const int32 NewCount)
{
	if (NewCount <= 0)
	{
		ToyboxCharacterMovementComponent->StopSlow();
		K2_OnSlowEnd();
	}
	else
	{
		ToyboxCharacterMovementComponent->StartSlow();
		K2_OnSlowBegin();
	}
}

void AToyboxCharacter::UnEquipCurrentWeapon() const
{
	if (HasRangedWeaponEquipped())
	{
		AttachWeapon(RangedChildActor, RangedHostSocket);
	}
	else if (HasMeleeWeaponEquipped())
	{
		AttachWeapon(MeleeChildActor, MeleeHostSocket);
	}
	ToyboxCharacterMovementComponent->ResetWeaponSpeed();
}

void AToyboxCharacter::AttachWeapon(const TWeakObjectPtr<UChildActorComponent> WeaponActor, const FName& SocketName) const
{
	const FAttachmentTransformRules WeaponAttachmentRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
	if (WeaponActor.IsValid())
	{
		if (!WeaponActor->AttachToComponent(GetMesh(), WeaponAttachmentRules, SocketName))
		{
			UE_LOG(LogTemp, Error, TEXT("Attachment failed"));
		}
	}
}


void AToyboxCharacter::EquipCurrentWeapon() const
{
	if (HasRangedWeaponEquipped())
	{
		AttachWeapon(RangedChildActor, RangedEquippedSocket);
		ToyboxCharacterMovementComponent->StartRangedSpeed();
	}
	else if (HasMeleeWeaponEquipped())
	{
		AttachWeapon(MeleeChildActor, MeleeEquippedSocket);
		ToyboxCharacterMovementComponent->StartMeleeSpeed();
	}
}

void AToyboxCharacter::EnableDefenderOutline() const
{
	if (ToyboxPlayerState->IsSpectatorTeam())
	{
		return;
	}
	
	EnableOutline();
	GetMesh()->SetCustomDepthStencilValue(DefenderStencilValue);
}

void AToyboxCharacter::EnableAttackerOutline() const
{
	if (ToyboxPlayerState->IsSpectatorTeam())
	{
		return;
	}
	
	const TWeakObjectPtr<AMatchGameState> MatchGameState = Cast<AMatchGameState>(
		UGameplayStatics::GetGameState(GetWorld()));

	MatchGameState->OnNewDefender.AddUObject(this, &ThisClass::OnNewDefender);
	
	if (!MatchGameState->IsLocalPlayerDefender())
	{
		EnableOutline();
		GetMesh()->SetCustomDepthStencilValue(CharacterChampionData->OutlineStencilValue);
	}
}

void AToyboxCharacter::EnableOutline() const
{
	if (HasAuthority() && !Controller->IsLocalController() ||
		GetLocalRole() == ENetRole::ROLE_SimulatedProxy)
	{
		GetMesh()->SetRenderCustomDepth(true);	
	}
}

void AToyboxCharacter::DisableOutline() const
{
	GetMesh()->SetRenderCustomDepth(false);
}

void AToyboxCharacter::StartSprint() const
{
	if (IsValid(ToyboxCharacterMovementComponent)) ToyboxCharacterMovementComponent->StartSprint();
}

void AToyboxCharacter::StopSprint() const
{
	if (IsValid(ToyboxCharacterMovementComponent)) ToyboxCharacterMovementComponent->StopSprint();
}

void AToyboxCharacter::ToggleSprint() const
{
	if (IsValid(ToyboxCharacterMovementComponent)) ToyboxCharacterMovementComponent->ToggleSprint();
}

UAbilitySystemComponent* AToyboxCharacter::GetAbilitySystemComponent() const
{
	return GetToyboxAbilitySystemComponent();
}

UToyboxAbilitySystemComponent* AToyboxCharacter::GetToyboxAbilitySystemComponent() const
{
	return ToyboxPlayerState.IsValid() ? ToyboxPlayerState->GetToyboxAbilitySystemComponent() : nullptr;
}

TSubclassOf<AThrowableProjectile> AToyboxCharacter::GetThrowableProjectile() const
{
	return CharacterChampionData->ThrowableProjectile;
}

bool AToyboxCharacter::InitializeAbilitySystem()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AToyboxCharacter::InitializeAbilitySystem)

	if (!ensure(IsValid(GetToyboxAbilitySystemComponent()))) return false;

	AbilitySystemComponent = GetToyboxAbilitySystemComponent();
	
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	
	OnMeleeTagChangedDelegate.BindUObject(this, &ThisClass::OnWeaponTagChanged);
	OnRangedTagChangedDelegate.BindUObject(this, &ThisClass::OnWeaponTagChanged);
	OnUnarmedTagChangedDelegate.BindUObject(this, &ThisClass::OnWeaponTagChanged);
	
	AbilitySystemComponent->RegisterAndCallGameplayTagEvent(NativeTags.Weapon_Ranged, OnRangedTagChangedDelegate);
	AbilitySystemComponent->RegisterAndCallGameplayTagEvent(NativeTags.Weapon_Unarmed, OnUnarmedTagChangedDelegate);
	AbilitySystemComponent->RegisterAndCallGameplayTagEvent(NativeTags.Weapon_Melee, OnMeleeTagChangedDelegate);

	OnSlowTagChangedDelegate.BindUObject(this, &ThisClass::OnSlowTagChanged);
	AbilitySystemComponent->RegisterAndCallGameplayTagEvent(NativeTags.State_Slowed, OnSlowTagChangedDelegate);

	return true;
}

void AToyboxCharacter::InitializeChampionData()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AToyboxCharacter::InitializeChampionData)

	CharacterChampionData = ToyboxPlayerState->GetChampionData();

	if (!ensure(EquippedMelee.IsValid())) return;
	if (!ensure(EquippedRanged.IsValid())) return;
	if (!ensure(CharacterChampionData != nullptr)) return;
	
	EquippedMelee->SetMeleeWeaponData(CharacterChampionData->MeleeWeaponData);
	EquippedRanged->SetRangedWeaponData(CharacterChampionData->RangedWeaponData);
	GetMesh()->SetSkeletalMesh(CharacterChampionData->RelatedMesh.LoadSynchronous(), false);
}

bool AToyboxCharacter::IsAlive() const
{
	//we are only adding this tag in HealthAttribute Set, so this looks like a much smarter choice
	//(means we dont need the Ptr to the Attribute Set on the character anymore)
	return bIsAlive;
}

bool AToyboxCharacter::IsAimingDownSights() const
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	//This could be optimized to be driven by a delegate, but in the ABP the ASC is null when BP Begin Play happens
	return AbilitySystemComponent.IsValid() ? AbilitySystemComponent->HasMatchingGameplayTag(NativeTags.State_AimDownSights) : false;
}

FGameplayTag AToyboxCharacter::GetCurrentWeapon() const
{
	if ((AbilitySystemComponent.IsValid() && AbilitySystemComponent->HasMatchingGameplayTag(CurrentWeaponTag)) || CurrentWeaponTag == FGameplayTag::EmptyTag)
	{
		return CurrentWeaponTag;
	}
	UE_LOG(LogTemp, Error, TEXT("Cached Weapon Tag is currently not on the Ability System Component, Replication Issue after weapon swapping?"));
	return FGameplayTag::EmptyTag;
}

UMeshComponent* AToyboxCharacter::GetCurrentWeaponMesh() const
{
	if (HasRangedWeaponEquipped())
	{
		return EquippedRanged.IsValid() ? EquippedRanged->GetWeaponMesh().Get() : nullptr;
	}
	if (HasMeleeWeaponEquipped())
	{
		return EquippedMelee.IsValid() ? EquippedMelee->GetWeaponMesh().Get() : nullptr;;
	}
	return nullptr;
}

bool AToyboxCharacter::HasRangedWeaponEquipped() const
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();

	return NativeTags.Weapon_Ranged == CurrentWeaponTag;
}

bool AToyboxCharacter::HasMeleeWeaponEquipped() const
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();

	return NativeTags.Weapon_Melee == CurrentWeaponTag;
}

TWeakObjectPtr<AMeleeWeapon> AToyboxCharacter::GetEquippedMelee() const
{
	return EquippedMelee;
}

TWeakObjectPtr<ARangedWeapon> AToyboxCharacter::GetEquippedRanged() const
{
	return EquippedRanged;
}

TWeakObjectPtr<UAnimMontage> AToyboxCharacter::GetSpecialAbilityMontage() const
{
	return CharacterChampionData->SpecialAbilityMontage.LoadSynchronous();
}

bool AToyboxCharacter::IsUnarmed() const
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();

	return NativeTags.Weapon_Unarmed == CurrentWeaponTag || FGameplayTag::EmptyTag == CurrentWeaponTag;
}

void AToyboxCharacter::SetShowDefenderOutline(bool NewValue)
{
	bShowDefenderOutline = true;
	EnableDefenderOutline();
}

void AToyboxCharacter::OnRep_ShowDefenderOutline()
{
	bShowDefenderOutline = true;
	EnableDefenderOutline();
}

void AToyboxCharacter::SetShowAttackerOutline(bool NewValue)
{
	bShowAttackerOutline = true;
	EnableAttackerOutline();
}

void AToyboxCharacter::OnRep_ShowAttackerOutline()
{
	bShowAttackerOutline = true;
	EnableAttackerOutline();
}

void AToyboxCharacter::OnRep_RagdollPlayer()
{
	bRagdollPlayer = true;
	RagdollCharacter();
}

void AToyboxCharacter::OnNewDefender(TWeakObjectPtr<AMatchPlayerState> DefenderState) const
{
	const TWeakObjectPtr<APlayerController> DefenderController = DefenderState->GetPlayerController();
	if (DefenderController.IsValid() && DefenderController->IsLocalController())
	{
		DisableOutline();
	}
}

