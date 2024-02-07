// Fill out your copyright notice in the Description page of Project Settings.


#include "HelperObjects/ToyboxInputConfig.h"

#include "GameplayTagContainer.h"
#include "EnhancedInput/Public/InputAction.h"
#include "Toybox/Toybox.h"




// ReSharper disable once CppConstValueFunctionReturnType
TWeakObjectPtr<const UInputAction> UToyboxInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag,
	const bool bLogNotFound) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UToyboxInputConfig::FindNativeInputActionForTag)

	for (const FTaggedInputAction& TaggedInputAction : NativeInputActions)
	{
		if (IsValid(TaggedInputAction.InputAction) && TaggedInputAction.InputTag == InputTag)
		{
			return TaggedInputAction.InputAction;
		}
	}
	
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}
	return nullptr;
}

// ReSharper disable once CppConstValueFunctionReturnType
TWeakObjectPtr<const UInputAction> UToyboxInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag,
	const bool bLogNotFound) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UToyboxInputConfig::FindAbilityInputActionForTag)

	for (const FTaggedInputAction& TaggedInputAction : AbilityInputActions)
	{
		if (IsValid(TaggedInputAction.InputAction) && TaggedInputAction.InputTag == InputTag)
		{
			return TaggedInputAction.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}
	return nullptr;
}
