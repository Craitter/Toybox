// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "HelperObjects/ToyboxDataTypes.h"
#include "ToyboxCharacter.generated.h"

class AWeapon;
class AMeleeWeapon;
class ARangedWeapon;
class IAnimNotifyInterface;
class AMatchPlayerState;
class UHealthAttributeSet;
class UToyboxCharacterMovementComponent;
class UToyboxInputConfig;
class UToyboxAbilityTagRelationshipMapping;
class UToyboxCharacterData;
class UAbilitySystemComponent;
class UToyboxAbilitySystemComponent;
class USpringArmComponent;
class UCameraComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FAnimNotifyDelegate, const FName, IAnimNotifyInterface*)
DECLARE_DELEGATE(FDeathDelegate)

UCLASS()
class TOYBOX_API AToyboxCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AToyboxCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	
	//Begin Pawn Interface
	virtual void OnRep_PlayerState() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void Falling() override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	//End Pawn Interface

	void CleanupAirborneTag() const;
	
	virtual void Tick(float DeltaSeconds) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool InitializeAbilitySystem();

	void InitializeChampionData();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UToyboxCharacterMovementComponent> ToyboxCharacterMovementComponent = {nullptr};
	UPROPERTY()
	TObjectPtr<UChildActorComponent> RangedChildActor = {nullptr};

	TWeakObjectPtr<AMeleeWeapon> EquippedMelee = {nullptr};
	TWeakObjectPtr<ARangedWeapon> EquippedRanged = {nullptr};
	UPROPERTY()
	TObjectPtr<UChildActorComponent> MeleeChildActor = {nullptr};
	
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm = {nullptr};

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera = {nullptr};

	TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent = {nullptr};
	TWeakObjectPtr<AMatchPlayerState> ToyboxPlayerState = {nullptr};

	bool bIsAlive = true;

	FOnGameplayEffectTagCountChanged::FDelegate OnRangedTagChangedDelegate;
	FOnGameplayEffectTagCountChanged::FDelegate OnMeleeTagChangedDelegate;
	FOnGameplayEffectTagCountChanged::FDelegate OnUnarmedTagChangedDelegate;

	FOnGameplayEffectTagCountChanged::FDelegate OnSlowTagChangedDelegate;
	
	FGameplayTag CurrentWeaponTag = FGameplayTag::EmptyTag;

	FTimerHandle DeathAnimationHandle;

protected:
	void OnDeathEvent();

	void RagdollCharacter();
	
	virtual void Destroyed() override;
	
	void OnDeadTagChanged(FGameplayTag ChangedTag, int32 NewCount);
	
	void OnWeaponTagChanged(FGameplayTag ChangedTag, int32 NewCount);

	void OnSlowTagChanged(FGameplayTag ChangedTag, int32 NewCount);
	
	void UnEquipCurrentWeapon() const;
	
	void AttachWeapon(const TWeakObjectPtr<UChildActorComponent> WeaponActor, const FName& SocketName) const;

	void EquipCurrentWeapon() const;

	void EnableDefenderOutline() const;

	void EnableAttackerOutline() const;

	void EnableOutline() const;

	void DisableOutline() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Socket")
	FName RangedHostSocket = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Socket")
	FName RangedEquippedSocket = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Socket")
	FName MeleeHostSocket = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Socket")
	FName MeleeEquippedSocket = NAME_None;

	FChampionData* CharacterChampionData = {nullptr};

	UPROPERTY(ReplicatedUsing = OnRep_ShowDefenderOutline)
	bool bShowDefenderOutline = false;

	UPROPERTY(ReplicatedUsing = OnRep_ShowAttackerOutline)
	bool bShowAttackerOutline = false;
	
	UPROPERTY(ReplicatedUsing = OnRep_RagdollPlayer)
	bool bRagdollPlayer = false;

	/** Called when the Character starts to be slowed. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement", DisplayName = "OnSlowBegin")
	void K2_OnSlowBegin();

	/** Called when the Character  stops to be slowed. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement", DisplayName = "OnSlowEnd")
	void K2_OnSlowEnd();
	
public:
	UPROPERTY(EditAnywhere, Category="Weapon|Debug", meta = (AllowPrivateAccess = "true"))
	bool bEnableWeaponDebug = false;

	UPROPERTY(EditDefaultsOnly, Category="Outline")
	int32 DefenderStencilValue = 1;
	
	// Begin Simple Getters
	UFUNCTION(BlueprintCallable, Category = "Character|Components")
	USpringArmComponent* GetSpringArm() const { return SpringArm; }
	
	UFUNCTION(BlueprintCallable, Category = "Character|Components")
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable, Category = "Character|Components")
	UToyboxCharacterMovementComponent* GetToyboxCharacterMovement() const { return ToyboxCharacterMovementComponent; }

	UFUNCTION(BlueprintCallable, Category = "Character|Components|CharacterMovement")
	void StartSprint() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Components|CharacterMovement")
	void StopSprint() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Components|CharacterMovement")
	void ToggleSprint() const;

	FAnimNotifyDelegate OnAnimNotifyEvent;

	FDeathDelegate OnDeath;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UFUNCTION(BlueprintCallable, Category = "Ability")
	UToyboxAbilitySystemComponent* GetToyboxAbilitySystemComponent() const;
	
	TSubclassOf<AThrowableProjectile> GetThrowableProjectile() const;
	// End Simple Getters

	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Combat")
	bool IsAimingDownSights() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Combat")
	FGameplayTag GetCurrentWeapon() const;
	
	UFUNCTION(BlueprintCallable, Category = "Character|Combat")
	UMeshComponent* GetCurrentWeaponMesh() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Combat")
	bool HasRangedWeaponEquipped() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Combat")
	bool HasMeleeWeaponEquipped() const;

	TWeakObjectPtr<AMeleeWeapon> GetEquippedMelee() const;
	TWeakObjectPtr<ARangedWeapon> GetEquippedRanged() const;

	TWeakObjectPtr<UAnimMontage> GetSpecialAbilityMontage() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Combat")
	bool IsUnarmed() const;

	void SetShowDefenderOutline(bool NewValue);
	
	void SetShowAttackerOutline(bool NewValue);
	
	UFUNCTION()
	void OnRep_ShowDefenderOutline();

	UFUNCTION()
	void OnRep_ShowAttackerOutline();
	
	UFUNCTION()
	void OnRep_RagdollPlayer();

	UFUNCTION()
	void OnNewDefender(TWeakObjectPtr<AMatchPlayerState> DefenderState) const;
};
