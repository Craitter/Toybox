// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AnimationNotifies/ThrowableLetGoNotify.h"

#include "Actors/ToyboxCharacter.h"

void UThrowableLetGoNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	const TWeakObjectPtr<AToyboxCharacter> OwnerCharacter = Cast<AToyboxCharacter>(MeshComp->GetOwner());
	
	if (!OwnerCharacter.IsValid())
	{
		return;
	}
	
	OwnerCharacter->OnAnimNotifyEvent.Broadcast(AnimNotifyNames::LetGoThrowableNotify, this);
}
