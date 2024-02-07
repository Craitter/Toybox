// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ToyboxCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UToyboxCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

private:
	class FSavedMove_ToyboxCharacter : public FSavedMove_Character
	{
		typedef FSavedMove_Character Super;

		uint8 bRequestedSprint : 1;

		uint8 bRequestedSlow : 1;

		uint8 bRequestedRanged : 1;

		uint8 bRequestedMelee : 1;

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

		virtual void Clear() override;

		virtual uint8 GetCompressedFlags() const override;

		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

		virtual void PrepMoveFor(ACharacter* Character) override;
	};

	class FNetworkPredictionData_Client_ToyboxCharacter : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_ToyboxCharacter(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:
	UToyboxCharacterMovementComponent();

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual float GetMaxSpeed() const override;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual void SetDefaultMovementMode() override;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void StartSprint();
	
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void StopSprint();
	
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void ToggleSprint();
	
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	bool IsSprinting() const;

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void StartSlow();
	
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void StopSlow();

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	bool IsSlowed() const;

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void ResetWeaponSpeed();
	
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void StartRangedSpeed();
	
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void StopRangedSpeed();

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void StartMeleeSpeed();
	
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void StopMeleeSpeed();
	
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void DashStarted();

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void DashEnded();

public:
	UPROPERTY(Category="Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxSpeedWalking = 500.0f;
	
	UPROPERTY(Category="Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxSpeedSprinting = 800.0f;

	UPROPERTY(Category="Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.001", UIMin="0.001", ClampMax="3.000", UIMax="3.000"))
	float MeleeSpeedModifier = 1.0f;
	
	UPROPERTY(Category="Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.001", UIMin="0.001", ClampMax="3.000", UIMax="3.000"))
	float RangedSpeedModifier = 1.0f;

	UPROPERTY(Category="Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.001", UIMin="0.001", ClampMax="3.000", UIMax="3.000"))
	float SlowSpeedModifier = 0.5f;
	
	//This Value is one sided, so with 30° we have a 60° sprinting cone, means that if we move 45° to the right we would stop sprinting
	UPROPERTY(Category="Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ClampMax="180", UIMax="180", ForceUnits="Degrees"))
	float MaxDegreeSprintingForward = 30.0f;

	UPROPERTY(Category="Character Movement (Rotation Settings)", EditAnywhere, BlueprintReadWrite)
	FRotator RotationRateWalking = FRotator(0.0f, 360.0f, 0.0f);
	
	UPROPERTY(Category="Character Movement (Rotation Settings)", EditAnywhere, BlueprintReadWrite)
	FRotator RotationRateSprinting = FRotator(0.0f, 100.0f, 0.0f);

	UPROPERTY(Category="Character Movement: Dashing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float DashStrength = 2000.0f;
	
	UPROPERTY(Category="Character Movement: Dashing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="Seconds"))
	float DashDuration = 0.3f;

	UPROPERTY(Category="Character Movement: Dashing", EditAnywhere, BlueprintReadWrite)
	bool bDashIsAdditive = false;

	/** 
	 *  Strength of the force over time
	 *  Curve Y is 0 to 1 which is percent of full Strength parameter to apply
	 *  Curve X is 0 to 1 normalized time if this force has a limited duration (Duration > 0), or
	 *          is in units of seconds if this force has unlimited duration (Duration < 0)
	 */
	UPROPERTY(Category="Character Movement: Dashing", EditAnywhere, BlueprintReadWrite, meta = (XAxisName="Time 0-1%", YAxisName="Strength 0-1%"))
	TObjectPtr<UCurveFloat> DashStrengthOverTime = {nullptr};

	UPROPERTY(Category="Character Movement: Dashing", EditAnywhere, BlueprintReadWrite)
	bool bEnableGravity = true;
	
private:
	bool bRequestedSprint = false;

	bool bRequestedSlow = false;

	bool bRequestedRanged = false;

	bool bRequestedMelee = false;
};
