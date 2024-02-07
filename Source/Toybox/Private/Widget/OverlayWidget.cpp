// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OverlayWidget.h"

#include "AbilitySystemComponent.h"
#include "Actors/MatchGameState.h"
#include "Actors/MatchPlayerState.h"
#include "Components/DynamicEntryBox.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Widget/AbilityWidget.h"
#include "Widget/HealthWidget.h"

void UOverlayWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (!ensure(IsValid(DefenderHealthWidget))) return;
	DefenderHealthWidget->SetVisibility(ESlateVisibility::Hidden);
	
	MatchGameState = GetWorld()->GetGameState<AMatchGameState>();
	if (!ensure(MatchGameState.IsValid())) return;

	const TWeakObjectPtr<AMatchPlayerState> MatchPlayerState = GetOwningPlayerState<AMatchPlayerState>();
	if (!ensure(MatchPlayerState.IsValid())) return;

	AbilitySystemComponent = MatchPlayerState->GetAbilitySystemComponent();
	if (!ensure(AbilitySystemComponent.IsValid())) return;

	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	
	if (MatchPlayerState->GetTeamTag() != FGameplayTag::EmptyTag)
	{
		OnTeamTagChanged(MatchPlayerState->GetTeamTag(), 1);
	}
	else
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(NativeTags.Team_Attacker).AddUObject(this, &ThisClass::OnTeamTagChanged);
		AbilitySystemComponent->RegisterGameplayTagEvent(NativeTags.Team_Defender).AddUObject(this, &ThisClass::OnTeamTagChanged);
	}
}

void UOverlayWidget::NativeDestruct()
{
	Super::NativeDestruct();
	MarkAsGarbage();
}

void UOverlayWidget::InitializeHealthWidget() const
{
	if (OwningPawnHealthWidget != nullptr)
	{
		OwningPawnHealthWidget->SetRelatedPlayerState(GetOwningPlayerState());
	}
}

void UOverlayWidget::OnTeamTagChanged(FGameplayTag ChangedTeamTag, const int32 NewCount)
{
	if (NewCount <= 0)
	{
		return;
	}
	
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	
	if (ChangedTeamTag == NativeTags.Team_Attacker)
	{
		InitializeHealthWidget();
		InitializeAbilityWidgets();
		UpdateTeamHealthWidget();
		UpdateDefenderHealthWidget();
		MatchGameState->OnNewAttacker.AddUObject(this, &ThisClass::OnNewAttacker);
		MatchGameState->OnNewDefender.AddUObject(this, &ThisClass::OnNewDefender);
	}
	else if (ChangedTeamTag == NativeTags.Team_Defender)
	{
		InitializeAbilityWidgets();
		InitializeHealthWidget();
	}
	K2_OnTeamTagChanged(ChangedTeamTag);
}

void UOverlayWidget::UpdateTeamHealthWidget() const
{
	TArray<TWeakObjectPtr<AMatchPlayerState>> Attackers;
	MatchGameState->GetAllAttackers(Attackers);

	CreateWidgetsForAttackers(Attackers);
}

void UOverlayWidget::UpdateDefenderHealthWidget() const
{
	TArray<TWeakObjectPtr<AMatchPlayerState>> Attackers;
	OnNewDefender(MatchGameState->GetDefender());
}

void UOverlayWidget::CreateWidgetsForAttackers(TArray<TWeakObjectPtr<AMatchPlayerState>>& Attackers) const
{
	for (const TWeakObjectPtr<AMatchPlayerState> Attacker : Attackers)
	{
		OnNewAttacker(Attacker);
	}
}

void UOverlayWidget::OnNewAttacker(const TWeakObjectPtr<AMatchPlayerState> PlayerState) const
{
	if (PlayerState != GetOwningPlayerState())
	{
		const TWeakObjectPtr<UHealthWidget> HealthWidget = TeamsHealth->CreateEntry<UHealthWidget>();
		HealthWidget->SetRelatedPlayerState(PlayerState);
	}
}

void UOverlayWidget::OnNewDefender(const TWeakObjectPtr<AMatchPlayerState> PlayerState) const
{
	if (!PlayerState.IsValid())
	{
		return;
	}
	
	if (PlayerState != GetOwningPlayerState())
	{
		DefenderHealthWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		DefenderHealthWidget->SetRelatedPlayerState(PlayerState);
	}
}

void UOverlayWidget::InitializeAbilityWidgets() const
{
	const TWeakObjectPtr<AMatchPlayerState> MatchPlayerState = GetOwningPlayerState<AMatchPlayerState>();

	if (!MatchPlayerState.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Passed Playerstate to Ability Widgets is not valid"));
		return;
	}

	const FChampionData* ChampionData = MatchPlayerState->GetChampionData();
	
	if (ThrowableWidget != nullptr)
	{
		ThrowableWidget->SetCooldownTag(ChampionData->ThrowableUiIcons.CooldownTag);
		ThrowableWidget->SetAbilityIcons(ChampionData->ThrowableUiIcons.ActiveIcon,
			ChampionData->ThrowableUiIcons.InactiveIcon);
		ThrowableWidget->SetRelatedPlayerState(GetOwningPlayerState());
	}
	
	if (SpecialWidget != nullptr)
	{
		SpecialWidget->SetCooldownTag(ChampionData->SpecialUiIcons.CooldownTag);
		SpecialWidget->SetAbilityIcons(ChampionData->SpecialUiIcons.ActiveIcon,
			ChampionData->SpecialUiIcons.InactiveIcon);
		SpecialWidget->SetRelatedPlayerState(GetOwningPlayerState());
	}
}
