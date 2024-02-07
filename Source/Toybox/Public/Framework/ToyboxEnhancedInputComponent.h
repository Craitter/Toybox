// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "HelperObjects/ToyboxInputConfig.h"

#include "ToyboxEnhancedInputComponent.generated.h"


/**
 * 
 */
UCLASS()
class TOYBOX_API UToyboxEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	UToyboxEnhancedInputComponent();
	
	template <class UserClass, typename FuncType>
	void BindNativeAction(const TWeakObjectPtr<UToyboxInputConfig> InputConfig, const FGameplayTag& InputTag,
		ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound, TArray<uint32>& BindHandles);

	template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const TWeakObjectPtr<UToyboxInputConfig> InputConfig, UserClass* Object, PressedFuncType PressedFunc,
		ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

	void RemoveBinds(TArray<uint32>& BindHandles);
};


template <class UserClass, typename FuncType>
void UToyboxEnhancedInputComponent::BindNativeAction(const TWeakObjectPtr<UToyboxInputConfig> InputConfig, const FGameplayTag& InputTag,
	ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound, TArray<uint32>& BindHandles)
{
	check(InputConfig.Get());

	const TWeakObjectPtr<const UInputAction> InputAction = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound);
	if (InputAction.IsValid())
	{ 
		BindHandles.Add(BindAction(InputAction.Get(), TriggerEvent, Object, Func).GetHandle());
	}
}

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UToyboxEnhancedInputComponent::BindAbilityActions(const TWeakObjectPtr<UToyboxInputConfig> InputConfig,
	UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig.Get());

	for (const FTaggedInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (IsValid(Action.InputAction) && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction.Get(), ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
			}

			if (ReleasedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction.Get(), ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
			}
		}
	}
}
