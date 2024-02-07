// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ToyboxCharacterMovementComponent.h"

#include "Actors/ToyboxCharacter.h"
#include "Actors/ToyboxPlayerController.h"
#include "GameFramework/Character.h"

constexpr float GDefault_Max_Smooth_Net_Update_Dist = 92.0f;
constexpr float GDefault_No_Smooth_Net_Update_Dist = 140.0f;


bool UToyboxCharacterMovementComponent::FSavedMove_ToyboxCharacter::CanCombineWith(const FSavedMovePtr& NewMove,
                                                                                   ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_ToyboxCharacter* NewCharacterMove = static_cast<FSavedMove_ToyboxCharacter*>(NewMove.Get());
	if (bRequestedSprint != NewCharacterMove->bRequestedSprint)
	{
		return false;
	}
	if (bRequestedSlow != NewCharacterMove->bRequestedSlow)
	{
		return false;
	}
	if (bRequestedRanged != NewCharacterMove->bRequestedRanged)
	{
		return false;
	}
	if (bRequestedMelee != NewCharacterMove->bRequestedMelee)
	{
		return false;
	}
	
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UToyboxCharacterMovementComponent::FSavedMove_ToyboxCharacter::Clear()
{
	FSavedMove_Character::Clear();

	bRequestedSprint = false;

	bRequestedSlow = false;

	bRequestedRanged = false;

	bRequestedMelee = false;
}

uint8 UToyboxCharacterMovementComponent::FSavedMove_ToyboxCharacter::GetCompressedFlags() const
{
	uint8 Flags = Super::GetCompressedFlags();

	if (bRequestedSprint) Flags |= FLAG_Custom_0;

	if (bRequestedSlow) Flags |= FLAG_Custom_1;

	if (bRequestedRanged) Flags |= FLAG_Custom_2;

	if (bRequestedMelee) Flags |= FLAG_Custom_3;
	
	return Flags;
}

void UToyboxCharacterMovementComponent::FSavedMove_ToyboxCharacter::SetMoveFor(ACharacter* Character, float InDeltaTime,
	FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	const UToyboxCharacterMovementComponent* CharacterMovementComponent = Cast<UToyboxCharacterMovementComponent>(Character->GetCharacterMovement());
	
	bRequestedSprint = CharacterMovementComponent->bRequestedSprint;
	bRequestedSlow = CharacterMovementComponent->bRequestedSlow;
	bRequestedRanged = CharacterMovementComponent->bRequestedRanged;
	bRequestedMelee = CharacterMovementComponent->bRequestedMelee;
}

void UToyboxCharacterMovementComponent::FSavedMove_ToyboxCharacter::PrepMoveFor(ACharacter* Character)
{
	FSavedMove_Character::PrepMoveFor(Character);

	UToyboxCharacterMovementComponent* CharacterMovementComponent = Cast<UToyboxCharacterMovementComponent>(Character->GetCharacterMovement());

	CharacterMovementComponent->bRequestedSprint = bRequestedSprint;
	CharacterMovementComponent->bRequestedSlow = bRequestedSlow;
	CharacterMovementComponent->bRequestedRanged = bRequestedRanged;
	CharacterMovementComponent->bRequestedMelee = bRequestedMelee;
}

UToyboxCharacterMovementComponent::FNetworkPredictionData_Client_ToyboxCharacter::FNetworkPredictionData_Client_ToyboxCharacter(
	const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
}

FSavedMovePtr UToyboxCharacterMovementComponent::FNetworkPredictionData_Client_ToyboxCharacter::AllocateNewMove()
{
	//Super is doing it the same way so we dont make it a shared ptr
	return FSavedMovePtr(new FSavedMove_ToyboxCharacter());
}

UToyboxCharacterMovementComponent::UToyboxCharacterMovementComponent()
{
	MaxWalkSpeed = 99999999.0f;
	NavAgentProps.bCanCrouch = true;
}

FNetworkPredictionData_Client* UToyboxCharacterMovementComponent::GetPredictionData_Client() const
{
	check(IsValid(PawnOwner))

	if (ClientPredictionData == nullptr)
	{
		UToyboxCharacterMovementComponent* MutableThis = const_cast<UToyboxCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_ToyboxCharacter(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = GDefault_Max_Smooth_Net_Update_Dist;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = GDefault_No_Smooth_Net_Update_Dist;
	}
	return ClientPredictionData;
}

float UToyboxCharacterMovementComponent::GetMaxSpeed() const
{
	if (MovementMode == MOVE_Walking)
	{
		float CalculatedSpeed = MaxSpeedWalking;
		if (bRequestedSprint)
		{
			CalculatedSpeed = MaxSpeedSprinting;
		}

		if (bRequestedRanged)
		{
			CalculatedSpeed *= RangedSpeedModifier;
		}
		else if (bRequestedMelee)
		{
			CalculatedSpeed *= MeleeSpeedModifier;
		}

		if (bRequestedSlow)
		{
			CalculatedSpeed *= SlowSpeedModifier;
		}
		
		return CalculatedSpeed;
	}
	// Super takes MaxWalkSpeed on nearly all cases, so that should be alright for now
	return MaxSpeedWalking; 
}

void UToyboxCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bRequestedSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bRequestedSlow = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
	bRequestedRanged = (Flags & FSavedMove_Character::FLAG_Custom_2) != 0;
	bRequestedMelee = (Flags & FSavedMove_Character::FLAG_Custom_3) != 0;
}

void UToyboxCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
	const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	//Instead of calling GetMaxSpeed() I could also set the value directly, but then its DRY isnt it?
	MaxWalkSpeed = GetMaxSpeed();
	if (MovementMode == MOVE_Walking && bRequestedSprint)
	{
		RotationRate = RotationRateSprinting;
	}
	else
	{
		RotationRate = RotationRateWalking;
	}
}

void UToyboxCharacterMovementComponent::SetDefaultMovementMode()
{
	if (MovementMode == MOVE_Falling && MovementMode != DefaultLandMovementMode)
	{
		const TWeakObjectPtr<AToyboxCharacter> Character = Cast<AToyboxCharacter>(CharacterOwner);
		Character->CleanupAirborneTag();
	}
	
	Super::SetDefaultMovementMode();
}

void UToyboxCharacterMovementComponent::StartSprint()
{
	bRequestedSprint = true;
}

void UToyboxCharacterMovementComponent::StopSprint()
{
	bRequestedSprint = false;
}

void UToyboxCharacterMovementComponent::ToggleSprint()
{
	bRequestedSprint = !bRequestedSprint;
}

bool UToyboxCharacterMovementComponent::IsSprinting() const
{
	return Velocity.SizeSquared2D() > FMath::Square(MaxSpeedWalking + 10.0f);
}

void UToyboxCharacterMovementComponent::StartSlow()
{
	bRequestedSlow = true;
}

void UToyboxCharacterMovementComponent::StopSlow()
{
	bRequestedSlow = false;
}

bool UToyboxCharacterMovementComponent::IsSlowed() const
{
	return bRequestedSlow;
}

void UToyboxCharacterMovementComponent::ResetWeaponSpeed()
{
	StopRangedSpeed();
	StopMeleeSpeed();
}

void UToyboxCharacterMovementComponent::StartRangedSpeed()
{
	StopMeleeSpeed();
	bRequestedRanged = true;
}

void UToyboxCharacterMovementComponent::StopRangedSpeed()
{
	bRequestedRanged = false;
}

void UToyboxCharacterMovementComponent::StartMeleeSpeed()
{
	StopRangedSpeed();
	bRequestedMelee = true;
}

void UToyboxCharacterMovementComponent::StopMeleeSpeed()
{
	bRequestedMelee = false;
}

void UToyboxCharacterMovementComponent::DashStarted()
{
	bWantsToCrouch = true;
}

void UToyboxCharacterMovementComponent::DashEnded()
{
	bWantsToCrouch = false;
}


