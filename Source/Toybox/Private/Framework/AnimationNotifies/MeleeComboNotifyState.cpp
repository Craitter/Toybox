// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AnimationNotifies/MeleeComboNotifyState.h"

#include "Actors/ToyboxCharacter.h"

void UMeleeComboNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                         float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	const TWeakObjectPtr<AToyboxCharacter> OwnerCharacter = Cast<AToyboxCharacter>(MeshComp->GetOwner());
	if (!OwnerCharacter.IsValid())
	{
		return;
	}

	OwnerCharacter->OnAnimNotifyEvent.Broadcast(AnimNotifyNames::MeleeComboStartNotify, this);
}

void UMeleeComboNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	const TWeakObjectPtr<AToyboxCharacter> OwnerCharacter = Cast<AToyboxCharacter>(MeshComp->GetOwner());
	if (!OwnerCharacter.IsValid())
	{
		return;
	}

	OwnerCharacter->OnAnimNotifyEvent.Broadcast(AnimNotifyNames::MeleeComboEndNotify, this);
}
