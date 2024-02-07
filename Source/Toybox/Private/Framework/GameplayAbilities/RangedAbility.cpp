// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/RangedAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueFunctionLibrary.h"
#include "GameplayCueManager.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Actors/RangedProjectile.h"
#include "Actors/RangedWeapon.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/DecalActor.h"
#include "Framework/ToyboxPlayerCameraManager.h"
#include "Framework/AttributeSets/RangedWeaponAttributeSet.h"
#include "Framework/CameraModifiers/RecoilCameraModifier.h"
#include "Framework/GameplayAbilities/ReloadAbility.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"

URangedAbility::URangedAbility()
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	
	ReloadAbilityTag = NativeTags.AbilityTag_Combat_Reload;

	DamageModiferTag = NativeTags.MetaTag_Damage;

	BulletSpreadModiferTag = NativeTags.MetaTag_BulletSpread;
	
	RecoilModifierTag = NativeTags.AbilityTag_Weapon_Recoil;
	
	bServerRespectsRemoteAbilityCancellation = false;
}

void URangedAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(URangedAbility::ActivateAbility)
	
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo) || ActorInfo == nullptr)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	if (!HasAuthority(&ActivationInfo))
	{
		Character = CastChecked<AToyboxCharacter>(ActorInfo->AvatarActor.Get());
		UpdateRangedAttributeSet();
	}

	const TWeakObjectPtr<APlayerController> Controller = ActorInfo->PlayerController.Get();
		
	if (!Controller.IsValid())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	RecoilModifer = GetRecoilCameraModifier(Controller);

	if (!ensure(RecoilModifer.IsValid()))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	
	if (!Character.IsValid())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	AbilitySystemComponent = Character->GetAbilitySystemComponent();

	Weapon = Character->GetEquippedRanged();

	if (!Weapon.IsValid() || !AbilitySystemComponent.IsValid())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	if (!ensure(IsValid(Weapon->GetFireAnimation())))
	{
		UE_LOG(LogTemp, Error, TEXT("Weapon doesnt have a Fire Animation"))
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}

	GunSocketVector = Weapon->GetWeaponMesh()->GetSocketLocation(BulletOriginSocket);

	if (!ensure(Weapon->GetDecalMaterial().IsValid()))
	{
		UE_LOG(LogTemp, Error, TEXT("Weapon doesnt have a decal material"))
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}

	if (!ensure(Weapon->GetWeaponMesh()->DoesSocketExist(BulletOriginSocket)))
	{
		UE_LOG(LogTemp, Error, TEXT("Weapon Mesh does not have a socket named: %s"), *BulletOriginSocket.ToString())
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	if (!ensure(IsValid(DealDamageGameplayEffect)))
	{
		UE_LOG(LogTemp, Error, TEXT("Deal Damage Gameplay Effect has not be set for ranged weapon gameplay ability"))
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	if (!ensure(IsValid(BulletSpreadGameplayEffect)))
	{
		UE_LOG(LogTemp, Error, TEXT("Bullet Spread Gameplay Effect has not be set for ranged weapon gameplay ability"))
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	
	if (!RangedAttributeSet.IsValid())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	if (!TrySetFireAnimation())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	CooldownDuration = RangedAttributeSet->GetCooldownAfterShotOrBurst();

	if (Weapon->GetOperationMode() == ERangedWeaponOperatingType::Burst && !CanStartBurstAbility())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	
	FireRangedWeapon();
	
	// End the ability here for now
}

void URangedAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	// Reset burst variables
	CurrentShotInBurst = 0;
	
	GetWorld()->GetTimerManager().ClearTimer(BurstShotTimer);

	if (!bWasCancelled)
	{
		ApplyCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
	}
}

bool URangedAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		RequestReloadRanged();
		return false;
	}

	return true;
}

void URangedAbility::CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	ApplyCost(Handle, ActorInfo, ActivationInfo);
}

void URangedAbility::OnPawnAvatarSet()
{
	Super::OnPawnAvatarSet();

	Character = CastChecked<AToyboxCharacter>(CurrentActorInfo->AvatarActor.Get());

	UpdateRangedAttributeSet();
}

void URangedAbility::OnAbilityInputRelease(float TimeWaited)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void URangedAbility::FireRangedWeapon()
{
	switch (Weapon->GetOperationMode())
	{
	case ERangedWeaponOperatingType::Continuous:
		{
			DoContinuousFire();
			break;
		}
	case ERangedWeaponOperatingType::Single:
		{
			DoSingleFire();
			break;
		}
	case ERangedWeaponOperatingType::Burst:
		{
			DoBurstFire();
			break;
		}
	default:
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			break;
		}
	}
}

void URangedAbility::RequestReloadRanged() const
{
	const TWeakObjectPtr<UAbilitySystemComponent> LocalAbilitySystemComponent = CurrentActorInfo->AbilitySystemComponent;

	if (!LocalAbilitySystemComponent.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to activate reload ability As athe ability system component isnt valid"))
		return;
	}

	if (!LocalAbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(ReloadAbilityTag)))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to activate reload ability as there are no abilities with the reload tag"))
	}
}

void URangedAbility::TraceShot(FHitResult& HitResult) const
{
	FVector TraceStart, TraceEnd;
	
	FHitResult CameraTraceHitResult;
	DoCameraLineTrace(CameraTraceHitResult);
	
	FHitResult GunTraceHitResult;
	GetGunTraceVectors(TraceStart, TraceEnd, CameraTraceHitResult);
	DoSingleLineTrace(GunTraceHitResult, TraceStart, TraceEnd);

	if (GunTraceHitResult.bBlockingHit)
	{
		HitResult = GunTraceHitResult;
	}
	else
	{
		HitResult = CameraTraceHitResult;
	}
}

void URangedAbility::DoSingleLineTrace(FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd) const
{
	FCollisionQueryParams TraceQueryParams;

	GetTraceQueryParams(TraceQueryParams);
	
	GetWorld()->LineTraceSingleByProfile(HitResult, TraceStart, TraceEnd, BULLET_COLLISION_PROFILE, TraceQueryParams);

	DebugTrace(HitResult, TraceStart, TraceEnd);
}

void URangedAbility::DoMultiLineTrace(FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd) const
{
	HitResult.ImpactPoint = TraceEnd;
	HitResult.TraceEnd = TraceEnd;
	
	TArray<FHitResult> HitResults;
	
	FCollisionQueryParams TraceQueryParams;

	GetTraceQueryParams(TraceQueryParams);
	
	GetWorld()->LineTraceMultiByProfile(HitResults, TraceStart, TraceEnd, PROFILE_OVERLAP_ALL, TraceQueryParams);

	// ReSharper disable once CppJoinDeclarationAndAssignment
	FVector HitLocation;
	
	for (int32 i = 0; i < HitResults.Num(); i++)
	{
		HitLocation = GetHitResultLocation(HitResults[i]);

		if (HitLocation.Equals(FVector::ZeroVector))
		{
			UE_LOG(LogTemp, Error, TEXT("Hit location when shooting is ZeroVector! :("))
			continue;
		}
		const float DotProduct = GetDotProductToLocation(HitLocation);

		if (DotProduct > 0.0f)
		{
			HitResult = HitResults[i];
			break;
		}
	}

	HitResult.bBlockingHit = true;
	
	DebugTrace(HitResult, TraceStart, TraceEnd);
}

void URangedAbility::DoCameraLineTrace(FHitResult& HitResult) const
{
	// Trace from Camera then from Gun
	FVector TraceStart, TraceEnd;
	GetCameraTraceVectors(TraceStart, TraceEnd);
	DoMultiLineTrace(HitResult, TraceStart, TraceEnd);
}

void URangedAbility::GetCameraTraceVectors(FVector& TraceStart, FVector& TraceEnd) const
{
	const float Range = RangedAttributeSet->GetMaxRange();
	
	FRotator TraceStartRotation;
	
	Character->GetController()->GetPlayerViewPoint(TraceStart, TraceStartRotation);

	ApplyBulletSpread(TraceStartRotation);
	
	const FVector NormalisedTraceDirection = TraceStartRotation.Vector();

	TraceEnd = TraceStart + (NormalisedTraceDirection * Range);
}

void URangedAbility::GetGunTraceVectors(FVector& TraceStart, FVector& TraceEnd, const FHitResult& HitResult) const
{	
	TraceStart = GunSocketVector;
	
	if (HitResult.bBlockingHit)
	{
		TraceEnd = HitResult.ImpactPoint;
	}
	else
	{
		TraceEnd = HitResult.TraceEnd;
	}
}


void URangedAbility::DebugTrace(const FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd) const
{
#if ENABLE_DRAW_DEBUG
	if (Character->bEnableWeaponDebug)
	{
		FColor DebugColor = FColor::Green;
		FVector DebugTraceStart = TraceStart;

		if (HitResult.bBlockingHit)
		{
			DrawDebugLine(
				GetWorld(),
				TraceStart,
				HitResult.ImpactPoint,
				DebugColor,
				false, 2.0f, 0,
				1.0f
			);

			DebugTraceStart = HitResult.ImpactPoint;
			DebugColor = FColor::Red;
		}

		DrawDebugLine(
			GetWorld(),
			DebugTraceStart,
			TraceEnd,
			DebugColor,
			false, 2.0f, 0,
			1.0f
		);
	}
#endif
}

void URangedAbility::DoContinuousFire()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(URangedAbility::DoContinuousFire)

	FireShot();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void URangedAbility::DoSingleFire()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(URangedAbility::DoSingleFire)

	FireShot();
	
	EndAbilityOnInputReleased();
}

void URangedAbility::DoBurstFire()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(URangedAbility::DoBurstFire)

	if (!CanBurst())
	{
		EndAbilityOnInputReleased();
		return;
	}

	if (CurrentShotInBurst > 0)
	{
		check(TrySetFireAnimation());
		
		ApplyCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
	}

	
	FireShot();
		
	if (++CurrentShotInBurst >= RangedAttributeSet->GetShotsPerBurst())
	{
		EndAbilityOnInputReleased();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(BurstShotTimer, this, &ThisClass::DoBurstFire,
			RangedAttributeSet->GetDelayBetweenBurstShots(), false);
	}
}

bool URangedAbility::CanBurst() const
{
	bool bCanBurst = true;
	
	if (CurrentShotInBurst > 0)
	{
		if (!CheckCost(CurrentSpecHandle, CurrentActorInfo, nullptr))
		{
			bCanBurst = false;
		}
	}
	
	return bCanBurst;
}

bool URangedAbility::CanStartBurstAbility() const
{
	bool bCanStart = true;
	
	if (RangedAttributeSet->GetDelayBetweenBurstShots() <= UE_FLOAT_NORMAL_THRESH)
	{
		UE_LOG(LogTemp, Error, TEXT("Delay Between Burst Shots for the ranged weapon is 0! It must be more than 0"))
		bCanStart = false;
	}

	return bCanStart;
}

void URangedAbility::EndAbilityOnInputReleased()
{
	AbilityInputReleasedTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);

	if (IsValid(AbilityInputReleasedTask))
	{
		AbilityInputReleasedTask->OnRelease.AddDynamic(this, &ThisClass::OnAbilityInputRelease);
		AbilityInputReleasedTask->ReadyForActivation();
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void URangedAbility::FireTraceShot() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(URangedAbility::FireTraceShot)

	FHitResult HitResult;
	
	TraceShot(HitResult);
	
	HandleHitResult(HitResult);

	ApplyRecoil();
}

void URangedAbility::FireProjectileShot() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(URangedAbility::FireProjectileShot())

	SpawnProjectile();

	ApplyRecoil();
}

void URangedAbility::SpawnProjectile() const
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}
	
	const FVector ProjectileTarget = GetProjectileTarget();

	const FVector ProjectileSpawn = Weapon->GetWeaponMesh()->GetSocketLocation(BulletOriginSocket);

	const FRotator Rotation = (ProjectileTarget - ProjectileSpawn).Rotation();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character.Get();

	const TWeakObjectPtr<ARangedProjectile> Projectile = GetWorld()->SpawnActor<ARangedProjectile>(
		Weapon->GetProjectile(), ProjectileSpawn, Rotation, SpawnParams);

	if (Projectile.IsValid())
	{
		Projectile->SetWeapon(Weapon.Get());

		Projectile->SetGameplayCueTag(Weapon->GetWallHitGameplayCueTag());
		
		Projectile->SetSpecHandle(GetDamageSpecHandle());
	}
}

void URangedAbility::FireShot() const
{
	// Play fire animation
	FireAnimationTask->ReadyForActivation();
	
	if (Weapon->IsProjectileBased())
	{
		FireProjectileShot();
	}
	else
	{
		FireTraceShot();
	}
}

void URangedAbility::ApplyRecoil() const
{
	RecoilModifer->ApplyRecoil(RangedAttributeSet->GetRecoil());
}

void URangedAbility::DrawDecal(const FVector& Location, const FRotator& Rotation) const 
{
	const TWeakObjectPtr<ADecalActor> Decal = GetWorld()->SpawnActor<ADecalActor>(Location, FRotator::ZeroRotator);
	if (Decal.IsValid())
	{
		Decal->SetActorEnableCollision(false);
		Decal->GetDecal()->FadeScreenSize = DECAL_FADE_SCREEN_SIZE;
		Decal->SetDecalMaterial(Weapon->GetDecalMaterial().Get());
		Decal->SetLifeSpan(Weapon->GetDecalLifeSpan());
		Decal->GetDecal()->DecalSize = Weapon->GetDecalSize();

		Decal->SetActorRelativeRotation(Rotation);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to spawn in decal but failed too! %s"), *FString(__FUNCTION__));
	}
}

void URangedAbility::UpdateRangedAttributeSet()
{
	const TWeakObjectPtr<UAbilitySystemComponent> LocalAbilitySystemComponent = CurrentActorInfo->AbilitySystemComponent;

	if (!LocalAbilitySystemComponent.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to activate reload ability As the ability system component isnt valid"))
		return;
	}
	
	RangedAttributeSet = LocalAbilitySystemComponent->GetSet<UWeaponAttributeSet>();
}

void URangedAbility::ApplyBulletSpread(FRotator& ShotRotation) const
{
	// First work out the starting spread. Then add any extra spread applied
	const float MaxSpread = RangedAttributeSet->GetMaxBulletSpreadInDegrees();
	const float MaxAccuracy = RangedAttributeSet->GetMaxAccuracy();
	const float MinAccuracy = RangedAttributeSet->GetMinAccuracy();
	
	const float BaseSpread = MaxSpread - (MaxSpread * MaxAccuracy);
	
	const float BulletSpread = BaseSpread + RangedAttributeSet->GetCurrentBulletSpread();

	ShotRotation += GetBulletSpreadRotator(BulletSpread);

	UpdateCurrentBulletSpread(BaseSpread, MinAccuracy, MaxSpread);
}

void URangedAbility::UpdateCurrentBulletSpread(const float BaseSpread, const float MinAccuracy, const float MaxSpread) const
{
	const float MinAccuracySpread = MaxSpread - (MaxSpread * MinAccuracy);
	const float SpreadToAdd = (MinAccuracySpread - BaseSpread) / RangedAttributeSet->GetBulletWhenLeastAccurate();
	
	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(BulletSpreadGameplayEffect,
		GetAbilityLevel(), AbilitySystemComponent->MakeEffectContext());

	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

	if (Spec != nullptr)
	{
		Spec->SetSetByCallerMagnitude(BulletSpreadModiferTag, SpreadToAdd);

		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec);
	}
}

FRotator URangedAbility::GetBulletSpreadRotator(const float Spread) const
{
	const float Pitch = FMath::FRandRange(-Spread, Spread);
	const float Yaw = FMath::FRandRange(-Spread, Spread);

	return FRotator(Pitch, Yaw, 0.0f);
}

FGameplayEffectSpecHandle URangedAbility::GetDamageSpecHandle() const
{
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DealDamageGameplayEffect,
		GetAbilityLevel(), AbilitySystemComponent->MakeEffectContext());

	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

	if (Spec != nullptr)
	{
		Spec->SetSetByCallerMagnitude(DamageModiferTag, RangedAttributeSet->GetDamage());
	}

	return SpecHandle;
}

FVector URangedAbility::GetProjectileTarget() const
{	
	FHitResult HitResult;
	DoCameraLineTrace(HitResult);

	if (HitResult.bBlockingHit)
	{
		return HitResult.ImpactPoint;
	}
	else
	{
		return HitResult.TraceEnd;
	}
}

TWeakObjectPtr<URecoilCameraModifier> URangedAbility::GetRecoilCameraModifier(TWeakObjectPtr<APlayerController> Controller) const
{
	if (RecoilModifer.IsValid())
	{
		return RecoilModifer;
	}
	
	const TWeakObjectPtr<AToyboxPlayerCameraManager> CameraManager = Cast<AToyboxPlayerCameraManager>(
		Controller->PlayerCameraManager);

	if (!ensure(CameraManager.IsValid()))
	{
		return nullptr;
	}
	
	const TWeakObjectPtr<URecoilCameraModifier> Modifier = Cast<URecoilCameraModifier>(CameraManager->
		FindCameraModifierByTag(RecoilModifierTag));
	
	return Modifier;
}

float URangedAbility::GetDotProductToLocation(const FVector& TargetLocation) const
{
	const FVector Dir = Character->GetFollowCamera()->GetForwardVector();
	FVector Offset = TargetLocation - GunSocketVector;
	Offset = Offset.GetSafeNormal();
	return FVector::DotProduct(Dir, Offset);
}

FVector URangedAbility::GetHitResultLocation(const FHitResult& HitResult) const
{
	if (HitResult.GetActor() != nullptr)
	{
		return HitResult.GetActor()->GetActorLocation();
	}

	if (HitResult.GetComponent() != nullptr)
	{
		return HitResult.GetComponent()->GetComponentLocation();
	}

	return FVector::ZeroVector;
}

bool URangedAbility::TrySetFireAnimation()
{
	FireAnimationTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("RangedAttackMontageTask"),
			Weapon->GetFireAnimation());
		
	if (!IsValid(FireAnimationTask))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create UAbilityTask_PlayMontageAndWait on ranged weapon firing"))
		return false;
	}

	return true;
}

void URangedAbility::GetTraceQueryParams(FCollisionQueryParams& TraceQueryParams) const
{
	TraceQueryParams.bTraceComplex = false;
	TraceQueryParams.bFindInitialOverlaps = true;
	TraceQueryParams.AddIgnoredActor(CurrentActorInfo->AvatarActor.Get());
}

void URangedAbility::HandleHitResult(const FHitResult& HitResult) const
{
	if (!HitResult.bBlockingHit)
	{
		return;
	}

	const TWeakObjectPtr<AActor> HitActor = HitResult.GetActor();
	if (!HitActor.IsValid())
	{
		return;
	}

	const IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(HitActor);

	if (AbilityInterface != nullptr)
	{
		DealDamage(AbilityInterface->GetAbilitySystemComponent(), HitResult);
	}
	else
	{
		DrawDecal(HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
		const FGameplayTag& GameplayCueTag = Weapon->GetWallHitGameplayCueTag();

		if (GameplayCueTag != FGameplayTag::EmptyTag)
		{
			UToyboxAbilitySystemComponent::CreateGameplayCueFromHitResult(Character, GameplayCueTag, HitResult);
		}
	}
}

void URangedAbility::DealDamage(const TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent, const FHitResult& HitResult) const
{
	if (!TargetAbilitySystemComponent.IsValid())
	{
		return;
	}

	const FGameplayEffectSpecHandle SpecHandle = GetDamageSpecHandle();

	const FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

	check(Spec != nullptr);
	
	Spec->GetContext().AddHitResult(HitResult);
	
	AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*Spec, TargetAbilitySystemComponent.Get());
}
