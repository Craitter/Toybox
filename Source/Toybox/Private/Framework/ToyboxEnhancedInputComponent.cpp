// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ToyboxEnhancedInputComponent.h"


UToyboxEnhancedInputComponent::UToyboxEnhancedInputComponent()
{
	//Todo: looking at Lyra here is one more functionality included, which is really interesting;
	//Todo: Add/Remove InputMappings, which goes hand in hand with player mapped input, this might be something for the future
}

void UToyboxEnhancedInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (const uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
