// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/LobbyPreviewActor.h"

#include "Actors/LobbyGameState.h"
#include "Components/CapsuleComponent.h"
#include "HelperObjects/MeleeWeaponData.h"
#include "HelperObjects/RangedWeaponData.h"
#include "Components/WidgetComponent.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"
#include "Widget/LobbyPlayerInfoWidget.h"

// Sets default values
ALobbyPreviewActor::ALobbyPreviewActor()
{
	CapsuleRoot = CreateDefaultSubobject<UCapsuleComponent>("Capsule");
	ToyMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	RangedMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Ranged");
	MeleeMesh = CreateDefaultSubobject<UStaticMeshComponent>("Melee");
	PlayerInfoWidget = CreateDefaultSubobject<UWidgetComponent>("InfoWidget");

	if (!ensure(IsValid(CapsuleRoot))) return;
	if (!ensure(IsValid(ToyMesh))) return;
	if (!ensure(IsValid(RangedMesh))) return;
	if (!ensure(IsValid(MeleeMesh))) return;
	if (!ensure(IsValid(PlayerInfoWidget))) return;

	SetRootComponent(CapsuleRoot);
	ToyMesh->SetupAttachment(CapsuleRoot);
	RangedMesh->SetupAttachment(ToyMesh);
	MeleeMesh->SetupAttachment(ToyMesh);
	PlayerInfoWidget->SetupAttachment(CapsuleRoot);

	//This is false because we manually control the replication of this whole actor in the GameState,
	//we have to do it because of the delayed initial joining of players
	bReplicates = false;
}

void ALobbyPreviewActor::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	PlayerInfoWidget->InitWidget();
	LobbyInfoWidget = Cast<ULobbyPlayerInfoWidget>(PlayerInfoWidget->GetWidget());
}

void ALobbyPreviewActor::SetPlayer(const FText& PlayerName, const EChampion& ChampionToPreview, const bool bIsReady)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ALobbyPreviewActor::SetPlayer)
	UpdatePlayerName(PlayerName);
	UpdateChampion(ChampionToPreview);
	UpdateIsReady(bIsReady);
	
	bIsAssigned = true;
}

void ALobbyPreviewActor::ClearPreview()
{
	if (IsValid(RangedMesh))
	{
		RangedMesh->SetSkeletalMesh(nullptr);
	}
	
	if (IsValid(MeleeMesh))
	{
		MeleeMesh->SetStaticMesh(nullptr);
	}
	
	if (IsValid(ToyMesh))
	{
		ToyMesh->SetSkeletalMesh(nullptr);
	}
	
	if (LobbyInfoWidget.IsValid())
	{
		LobbyInfoWidget->ResetPlayerInfo();
	}
		
	CurrentChampion = EChampion::None;
	CurrentPlayerName = FText::GetEmpty();
	CurrentChampionData = nullptr;
	
	bIsAssigned = false;
}


// Called when the game starts or when spawned
void ALobbyPreviewActor::BeginPlay()
{
	Super::BeginPlay();

	check(GetWorld())
	
	const TWeakObjectPtr<ALobbyGameState> GameState = GetWorld()->GetGameState<ALobbyGameState>();
	if (GameState.IsValid())
	{
		GameState->RegisterPreviewActor(this);
	}
}

void ALobbyPreviewActor::SetNameInfoText() const
{
	if (LobbyInfoWidget.IsValid())
	{
		LobbyInfoWidget->SetPlayerNameInfo(CurrentPlayerName);
	}
}

void ALobbyPreviewActor::UpdatePlayerName(const FText& PlayerName)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ALobbyPreviewActor::UpdatePlayerName)

	if (CurrentPlayerName.EqualTo(PlayerName))
	{
		return;
	}
	CurrentPlayerName = PlayerName;
	
	SetNameInfoText();
}

void ALobbyPreviewActor::SetChampionWeaponMeshes() const
{
	if (!IsValid(RangedMesh) || !IsValid(MeleeMesh))
	{
		return;
	}

	if (CurrentChampionData == nullptr)
	{
		RangedMesh->SetSkeletalMeshAsset(nullptr);
		MeleeMesh->SetStaticMesh(nullptr);
		return;
	}
	if (!ensure(!CurrentChampionData->RangedWeaponData.IsNull())) return;
	if (!ensure(!CurrentChampionData->MeleeWeaponData.IsNull())) return;
	
	RangedMesh->SetSkeletalMeshAsset(CurrentChampionData->RangedWeaponData.LoadSynchronous()->RelatedRanged.LoadSynchronous());
	MeleeMesh->SetStaticMesh(CurrentChampionData->MeleeWeaponData.LoadSynchronous()->RelatedMelee.LoadSynchronous());

	FName RangedSocket;
	FName MeleeSocket;
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	if (CurrentChampionData->PreviewEquippedWeapon == NativeTags.Weapon_Ranged)
	{
		RangedSocket = TEXT("socket_rangedequip");
		MeleeSocket = TEXT("socket_meleehost");
	}
	else if (CurrentChampionData->PreviewEquippedWeapon == NativeTags.Weapon_Melee)
	{
		RangedSocket = TEXT("socket_rangedhost");
		MeleeSocket = TEXT("socket_meleeequip");
	}
	else
	{
		RangedSocket = TEXT("socket_rangedhost");
		MeleeSocket = TEXT("socket_meleehost");
	}
	
	if (!RangedMesh->AttachToComponent(ToyMesh, WeaponAttachmentRules, RangedSocket) ||
		!MeleeMesh->AttachToComponent(ToyMesh, WeaponAttachmentRules, MeleeSocket))
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket for Attaching Weapons was not valid, "
			       "check the names of the sockets, they are hardcoded, so if there was a change in the"
			       "skeleton this could have caused it"));
	}
}

void ALobbyPreviewActor::SetChampionMesh() const
{
	if (!IsValid(ToyMesh))
	{
		return;
	}
	
	if (CurrentChampionData == nullptr)
	{
		ToyMesh->SetSkeletalMeshAsset(nullptr);
	}
	else
	{
		ToyMesh->SetSkeletalMeshAsset(CurrentChampionData->RelatedMesh.LoadSynchronous());
		ToyMesh->PlayAnimation(CurrentChampionData->RelatedIdleAnimation.LoadSynchronous(), true);
	}
}

void ALobbyPreviewActor::SetChampionInfoText() const
{
	if (CurrentChampionData != nullptr && LobbyInfoWidget.IsValid())
	{
		LobbyInfoWidget->SetToylineInfo(CurrentChampionData->Toyline);
		LobbyInfoWidget->SetChampionInfo(CurrentChampionData->Champion);
	}
}

void ALobbyPreviewActor::UpdateChampion(const EChampion Champion)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ALobbyPreviewActor::UpdateChampion)
	
	if (CurrentChampion == Champion)
	{
		return;
	}
	CurrentChampion = Champion;
	CurrentChampionData = GetChampionData(Champion);
	
	SetChampionInfoText();	
	SetChampionMesh();
	SetChampionWeaponMeshes();
}

void ALobbyPreviewActor::SetReadyInfoText() const
{
	if (LobbyInfoWidget.IsValid())
	{
		LobbyInfoWidget->SetPlayerReadyInfo(bIsOwnerReady);
	}
}

void ALobbyPreviewActor::UpdateIsReady(const bool bIsReady)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ALobbyPreviewActor::UpdateIsReady)

	bIsOwnerReady = bIsReady;
	
	SetReadyInfoText();
}

FChampionData* ALobbyPreviewActor::GetChampionData(const EChampion Champion) const
{
	if (!IsValid(ChampionTable) || Champion == EChampion::None)
	{
		return nullptr;	
	}
	const FString ContextString = FString(TEXT("Preview Actor Tries to get ChampionData For Champion"));
	TArray<FChampionData*> Champions;
	ChampionTable->GetAllRows<FChampionData>(ContextString, Champions);

	for (FChampionData* ChampionData : Champions)
	{
		if (ChampionData != nullptr && ChampionData->Champion == Champion)
		{
			return ChampionData;
		}
	}
	return nullptr;
}


bool ALobbyPreviewActor::IsAssigned() const
{
	return bIsAssigned;
}

