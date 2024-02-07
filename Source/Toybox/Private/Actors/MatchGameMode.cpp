// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MatchGameMode.h"

#include "EngineUtils.h"
#include "Actors/ArenaActor.h" 
#include "Actors/MatchGameState.h"
#include "Actors/LobbyGameState.h"
#include "Actors/MatchPlayerState.h"
#include "Actors/MatchSetup.h"
#include "Actors/ToyboxPlayerController.h"
#include "Actors/ToyboxPlayerStart.h"
#include "Actors/ToyboxPlayerState.h"
#include "Engine/PlayerStartPIE.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Toybox/Toybox.h"



namespace BattleState
{
	const FName PreGame = FName(TEXT("PreGame"));
	const FName Preparation = FName(TEXT("Preparation"));
	const FName Battle = FName(TEXT("Battle"));
	const FName PostBattle = FName(TEXT("PostBattle"));
	const FName PostGame = FName(TEXT("PostGame"));
}

#if WITH_EDITORONLY_DATA
static FAutoConsoleCommand CVarStartPreparation(TEXT("Toybox.StartPreparation"),
	TEXT("StartMatch\n"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		const FWorldContext* Context = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
		// check(Context)
		const TWeakObjectPtr<UWorld> World = Context->World();
		if (!World.IsValid()) return;

		const TWeakObjectPtr<AMatchGameMode> MatchGameMode = Cast<AMatchGameMode>(UGameplayStatics::GetGameMode(World.Get()));
		if (MatchGameMode == nullptr)
		{
			const TWeakObjectPtr<APlayerController> PlayerController = UGameplayStatics::GetPlayerController(World.Get(), LOCAL_USER_INDEX);
			if (!PlayerController.IsValid()) return;
			PlayerController->ServerExec("Toybox.StartPreparation");
		}
		else
		{
			MatchGameMode->StartPreparation();
		}
	}),
	ECVF_Default);

static FAutoConsoleCommand CVarStartBattle(TEXT("Toybox.StartBattle"),
	TEXT("StartMatch\n"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		const FWorldContext* Context = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
		// check(Context)
		const TWeakObjectPtr<UWorld> World = Context->World();
		if (!World.IsValid()) return;

		const TWeakObjectPtr<AMatchGameMode> MatchGameMode = Cast<AMatchGameMode>(UGameplayStatics::GetGameMode(World.Get()));
		if (MatchGameMode == nullptr)
		{
			if (!ensureMsgf(World->GetNetMode() != ENetMode::NM_ListenServer, TEXT("GameMode was null but we are on the server"))) return;
			const TWeakObjectPtr<APlayerController> PlayerController = UGameplayStatics::GetPlayerController(World.Get(), LOCAL_USER_INDEX);
			if (!PlayerController.IsValid()) return;
			PlayerController->ServerExec("Toybox.StartBattle");
		}
		else
		{
			MatchGameMode->StartBattle();
		}
	}),
	ECVF_Default);

static FAutoConsoleCommand CVarEndBattle(TEXT("Toybox.EndBattle"),
	TEXT("StartMatch\n"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		const FWorldContext* Context = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
		// check(Context)
		const TWeakObjectPtr<UWorld> World = Context->World();
		if (!World.IsValid()) return;

		const TWeakObjectPtr<AMatchGameMode> MatchGameMode = Cast<AMatchGameMode>(UGameplayStatics::GetGameMode(World.Get()));
		if (MatchGameMode == nullptr)
		{
			const TWeakObjectPtr<APlayerController> PlayerController = UGameplayStatics::GetPlayerController(World.Get(), LOCAL_USER_INDEX);
			if (!PlayerController.IsValid()) return;
			PlayerController->ServerExec("Toybox.EndBattle");
		}
		else
		{
			MatchGameMode->NotifyBattleIsOver();
		}
	}),
	ECVF_Default);

static FAutoConsoleCommand CVarEndPostBattle(TEXT("Toybox.EndPostBattle"),
	TEXT("StartMatch\n"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		const FWorldContext* Context = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
		// check(Context)
		const TWeakObjectPtr<UWorld> World = Context->World();
		if (!World.IsValid()) return;

		const TWeakObjectPtr<AMatchGameMode> MatchGameMode = Cast<AMatchGameMode>(UGameplayStatics::GetGameMode(World.Get()));
		if (MatchGameMode == nullptr)
		{
			const TWeakObjectPtr<APlayerController> PlayerController = UGameplayStatics::GetPlayerController(World.Get(), LOCAL_USER_INDEX);
			if (!PlayerController.IsValid()) return;
			PlayerController->ServerExec("Toybox.EndPostBattle");
		}
		else
		{
			MatchGameMode->NotifyPostBattleIsOver();
		}
	}),
	ECVF_Default);

static FAutoConsoleCommand CVarEndGame(TEXT("Toybox.EndGame"),
	TEXT("StartMatch\n"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		const FWorldContext* Context = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
		// check(Context)
		const TWeakObjectPtr<UWorld> World = Context->World();
		if (!World.IsValid()) return;

		const TWeakObjectPtr<AMatchGameMode> MatchGameMode = Cast<AMatchGameMode>(UGameplayStatics::GetGameMode(World.Get()));
		if (MatchGameMode == nullptr)
		{
			const TWeakObjectPtr<APlayerController> PlayerController = UGameplayStatics::GetPlayerController(World.Get(), LOCAL_USER_INDEX);
			if (!PlayerController.IsValid()) return;
			PlayerController->ServerExec("Toybox.EndGame");
		}
		else
		{
			MatchGameMode->StartPostGame();
		}
	}),
	ECVF_Default);

static FAutoConsoleCommand CVarAssignTeams(TEXT("Toybox.AssignTeams"),
	TEXT("Assigns a Defender and three Attacker\n"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		const FWorldContext* Context = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
		// check(Context)
		const TWeakObjectPtr<UWorld> World = Context->World();
		if (!World.IsValid()) return;

		const TWeakObjectPtr<AMatchGameMode> MatchGameMode = Cast<AMatchGameMode>(UGameplayStatics::GetGameMode(World.Get()));
		if (MatchGameMode == nullptr)
		{
			const TWeakObjectPtr<APlayerController> PlayerController = UGameplayStatics::GetPlayerController(World.Get(), LOCAL_USER_INDEX);
			if (!PlayerController.IsValid()) return;
			PlayerController->ServerExec("Toybox.AssignTeams");
		}
		else
		{
			const TWeakObjectPtr<AMatchGameState> MatchGameState = Cast<AMatchGameState>(MatchGameMode->GameState);
			if (MatchGameState.IsValid() && MatchGameState->MatchSetup.IsValid())
			{
				MatchGameState->MatchSetup->AssignTeamsByDefender(UGameplayStatics::GetPlayerController(World.Get(), LOCAL_USER_INDEX));
			}
		}
	}),
	ECVF_Default);
#endif

AMatchGameMode::AMatchGameMode()
{
	bDelayedStart = false;
	bStartPlayersAsSpectators = false;
	BattleState = BattleState::PreGame;
}

void AMatchGameMode::SetBattleState(FName NewState)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AMatchGameMode::SetBattleState)
	
	if (BattleState == NewState || bBattleStateChangeIsPending)
	{
		return;
	}
	bBattleStateChangeIsPending = true;
	
	// Call change callbacks
	if (BattleState == BattleState::PreGame)
	{
		HandleEndPreGame();
	}
	else if (BattleState == BattleState::Preparation)
	{
		HandleEndPreparation();
	}
	else if (BattleState == BattleState::Battle)
	{
		HandleEndBattle();
	}
	else if (BattleState == BattleState::PostBattle)
	{
		HandleEndPostBattle();
	}

	UE_LOG(LogGameMode, Display, TEXT("Battle State Changed from %s to %s"), *BattleState.ToString(), *NewState.ToString());

	BattleState = NewState;
	
	K2_OnBattleStateSet(NewState);
	
	if (BattleState == BattleState::Preparation)
	{
		HandleStartPreparation();
	}
	else if (BattleState == BattleState::Battle)
	{
		HandleStartBattle();
	}
	else if (BattleState == BattleState::PostBattle)
	{
		HandleStartPostBattle();
	}
	else if (BattleState == BattleState::PostGame)
	{
		HandleStartPostGame();
	}
	
	if (MatchGameState.IsValid())
	{
		MatchGameState->SetBattleState(NewState);
	}
	
	bBattleStateChangeIsPending = false;
}

bool AMatchGameMode::HandleEndPreGame()
{
	return true;
}

bool AMatchGameMode::HandleStartPreparation()
{
	MatchSetup->ProcessPreparationPhaseStart();
	
	return true;
}

bool AMatchGameMode::HandleEndPreparation()
{
	DestroyAllPlayerPawns();
	return true;
}

void AMatchGameMode::RestartAllPlayers()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AMatchGameMode::RestartAllPlayers)
	
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerCanRestart(PlayerController))
		{
			RestartPlayer(PlayerController);
		}
	}
}

void AMatchGameMode::DestroyAllPlayerPawns()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		TWeakObjectPtr<APlayerController> PlayerController = Iterator->Get();
		if (PlayerController.IsValid() && PlayerCanRestart(PlayerController.Get()))
		{
			DestroyPlayerPawn(PlayerController);
		}
	}
}

void AMatchGameMode::DestroyPlayerPawn(TWeakObjectPtr<AController> PlayerController)
{
	if (IsValid(PlayerController->GetPawn()))
	{
		PlayerController->GetPawn()->Destroy();
	}
}

float AMatchGameMode::GetRespawnDelay(const int32 DeathCount) const
{
	if (!IsValid(AttackerRespawnDelay))
	{
		UE_LOG(LogTemp, Error, TEXT("AttackerRespawnDelay Curve is not filled out in MAtch Game Mode BP, cant get proper respawn delay"));
		return MinRespawnDelay;
	}
	return AttackerRespawnDelay->GetFloatValue(DeathCount);
}

bool AMatchGameMode::HandleStartBattle()
{
	MatchSetup->ProcessBattleStart();

	// restart players
	RestartAllPlayers();
	
	return true;
}

bool AMatchGameMode::HandleEndBattle()
{
	MatchSetup->ProcessBattleEnd();
	DestroyAllPlayerPawns();
	return true;
}

bool AMatchGameMode::HandleStartPostBattle()
{
	return true;
}

bool AMatchGameMode::HandleEndPostBattle()
{
	return true;
}

bool AMatchGameMode::HandleStartPostGame()
{
	return true;
}

bool AMatchGameMode::HandleEndPostGame()
{
	return true;
}

void AMatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
}

void AMatchGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

FString AMatchGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
	const FString& Options, const FString& Portal)
{
	MatchSetup->AddPlayer(NewPlayerController);
	
	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
}

void AMatchGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	//if we dont come over by seemless travel bDelayed Start will be false and then we can load in the default templates
	//of the arena actors
	if (!bDelayedStart)
	{
		MatchSetup->CacheArenaActors();
	}
}

void AMatchGameMode::StartPreparation()
{
	return SetBattleState(BattleState::Preparation);
}

void AMatchGameMode::HandleMatchHasStarted()
{
	if (bDelayedStart)
	{
		StartPreparation();
	}
	
	Super::HandleMatchHasStarted();
}

void AMatchGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	StartToLeaveMap();
}

void AMatchGameMode::HandleLeavingMap()
{
	Super::HandleLeavingMap();
}

void AMatchGameMode::GetAllPossibleStartSpots(AController* Player, TArray<TWeakObjectPtr<APlayerStart>>& UnOccupiedStartPoints, TArray<TWeakObjectPtr<APlayerStart>>& OccupiedStartPoints)
{
	const UClass* PawnClass = GetDefaultPawnClassForController(Player);
	const APawn* PawnToFit = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr;
	UWorld* World = GetWorld();
	for (TWeakObjectPtr<APlayerStart> PlayerStart : MatchSetup->GetCurrentAttackerStartSpots())
	{
		FVector ActorLocation = PlayerStart->GetActorLocation();
		const FRotator ActorRotation = PlayerStart->GetActorRotation();
		if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation))
		{
			UnOccupiedStartPoints.Add(PlayerStart);
		}
		else if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
		{
			OccupiedStartPoints.Add(PlayerStart);
		}
	}
}

AActor* AMatchGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AMatchGameMode::ChoosePlayerStart_Implementation)
	
	//implementing super in here instead of calling since we dont care about the whole
	//world but only about the ones MatchSetup gives us

	// Choose a player start
	TWeakObjectPtr<APlayerStart> FoundPlayerStart = nullptr;
	TArray<TWeakObjectPtr<APlayerStart>> UnOccupiedStartPoints;
	TArray<TWeakObjectPtr<APlayerStart>> OccupiedStartPoints;
	UWorld* World = GetWorld();
	
#if WITH_EDITOR
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (PlayerStart->IsA<APlayerStartPIE>())
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			FoundPlayerStart = PlayerStart;
			return FoundPlayerStart.Get();
		}
	}
#endif
	GetAllPossibleStartSpots(Player, UnOccupiedStartPoints, OccupiedStartPoints);

	if (UnOccupiedStartPoints.Num() > 0)
	{
		FoundPlayerStart = UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
	}
	else if (OccupiedStartPoints.Num() > 0)
	{
		FoundPlayerStart = OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
	}
	
	return FoundPlayerStart.IsValid() ? FoundPlayerStart.Get() : Super::ChoosePlayerStart_Implementation(Player);
}

void AMatchGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	//This is being called by Post login at one point and by PostSeamless travel,
	//if we are not coming from seamless travel bDelayed start is false and we can handle players as usual
	if (!bDelayedStart)
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

void AMatchGameMode::PreInitializeComponents()
{
	// Currently we dont want to save the Match Setup in a different Map
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;

	const TWeakObjectPtr<UWorld> World = GetWorld();
	MatchSetup = World->SpawnActor<AMatchSetup>(AMatchSetup::StaticClass(), SpawnInfo);
	check(MatchSetup);
	MatchSetup->SetAvailableTemplates(Templates);
	
	Super::PreInitializeComponents();
}

void AMatchGameMode::InitGameState()
{
	Super::InitGameState();

	MatchGameState = GetGameState<AMatchGameState>();
	if (MatchGameState.IsValid())
	{
		// cache it in GameState so it's easily accessible client-side
		MatchGameState->MatchSetup = MatchSetup; 
	}
}

void AMatchGameMode::PostSeamlessTravel()
{
	/*
	 * When this is being called we know that the world is done, after this function world.cpp calls BeginPlay()
	 * So we can tell our MatchSetup to Cache all ArenaActors now, in super we then init the players, so the MatchSetup
	 * can react to every player being add without waiting for Arena Actors
	 */
	
	/*
	 * IGNORE THOSE NOTES; JUST MY THOUGHT; SO I DONT FORGET IT
	 * New Player Controller State to handle PrePlay Phase
	 * Set it in InitPlayerPostSeamlessTravel (also setting default start locations depending on arena)
	 * When its received clientside we want to ask MatchSetup where to go to (I think)
	 * We would then Add a Widget to template and then swap state to Play, then we can load GAS
	 * Important Node: We have to stop a lot of init stuff in the PC or move it to PC::StartPlay
	 * Biggest challenge: Keep play in editor playable
	 */
	bDelayedStart = true;
	bStartPlayersAsSpectators = true;
	MatchSetup->CacheArenaActors();
	Super::PostSeamlessTravel();
}

void AMatchGameMode::SetSeamlessTravelViewTarget(APlayerController* PC)
{
	const TWeakObjectPtr<AActor> NewViewTarget = MatchSetup->GetAssignedArenaActor(PC);
	if (NewViewTarget.IsValid())
	{
		PC->SetViewTarget(NewViewTarget.Get());
		PC->bAutoManageActiveCameraTarget = false;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("We arrived from Seamless travel but had no Arena Actor to set as view Target %s() "), *FString(__FUNCTION__));
		Super::SetSeamlessTravelViewTarget(PC);
	}
}

AActor* AMatchGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	const TWeakObjectPtr<AMatchPlayerState> PlayerState = Player->GetPlayerState<AMatchPlayerState>();
	check(PlayerState.Get())
	TWeakObjectPtr<AActor> PlayerStart;
	if (BattleState == BattleState::Preparation)
	{
		PlayerStart = MatchSetup->GetDefenderStartSpot(Cast<APlayerController>(Player)).Get();
	}
	else if (BattleState == BattleState::Battle)
	{
		if (PlayerState->IsDefenderTeam())
		{
			PlayerStart = MatchSetup->GetDefenderStartSpot(Cast<APlayerController>(Player)).Get();
		}
		else if (PlayerState->IsAttackerTeam())
		{
			PlayerStart = ChoosePlayerStart(Player);			
		}
	}
	return PlayerStart.IsValid() ? PlayerStart.Get() : Super::FindPlayerStart_Implementation(Player, IncomingName);
}

void AMatchGameMode::InitStartSpot_Implementation(AActor* StartSpot, AController* NewPlayer)
{
	Super::InitStartSpot_Implementation(StartSpot, NewPlayer);

	//This is only for the defender start spots and we make sure to set the start spot not twice...
	//eventhough it woudnt matter
	const TWeakObjectPtr<AToyboxPlayerStart> PlayerStart = Cast<AToyboxPlayerStart>(StartSpot);
	if (!PlayerStart.IsValid() || PlayerStart->IsPlayerStartInitialized())
	{
		return;
	}

	const TWeakObjectPtr<AMatchPlayerState> PlayerState = NewPlayer->GetPlayerState<AMatchPlayerState>();

	PlayerStart->SetMesh(PlayerState->GetSpawnPointMesh());
}

void AMatchGameMode::RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AMatchGameMode::RestartPlayerAtPlayerStart)
	
	if (NewPlayer == nullptr || NewPlayer->IsPendingKillPending())
	{
		return;
	}

	if (!StartSpot)
	{
		UE_LOG(LogGameMode, Warning, TEXT("RestartPlayerAtPlayerStart: Player start not found"));
		return;
	}

	FRotator SpawnRotation = StartSpot->GetActorRotation();

	UE_LOG(LogGameMode, Verbose, TEXT("RestartPlayerAtPlayerStart %s"), (NewPlayer && NewPlayer->PlayerState) ? *NewPlayer->PlayerState->GetPlayerName() : TEXT("Unknown"));
	
	if (MustSpectate(Cast<APlayerController>(NewPlayer)))
	{
		UE_LOG(LogGameMode, Verbose, TEXT("RestartPlayerAtPlayerStart: Tried to restart a spectator-only player!"));
		return;
	}

	if (NewPlayer->GetPawn() != nullptr)
	{
		DestroyPlayerPawn(NewPlayer);
	}
	if (GetDefaultPawnClassForController(NewPlayer) != nullptr)
	{
		// Try to create a pawn to use of the default class for this player
		APawn* NewPawn = SpawnDefaultPawnFor(NewPlayer, StartSpot);
		if (IsValid(NewPawn))
		{
			NewPlayer->SetPawn(NewPawn);
		}
	}
	
	if (!IsValid(NewPlayer->GetPawn()))
	{
		FailedToRestartPlayer(NewPlayer);
	}
	else
	{
		// Tell the start spot it was used
		InitStartSpot(StartSpot, NewPlayer);

		FinishRestartPlayer(NewPlayer, SpawnRotation);
	}
}

float AMatchGameMode::GetMaxTimeTemplateChoice() const
{
	return TemplateChoiceTime;
}

float AMatchGameMode::GetPreparationTime() const
{
	return PreparationTime;
}

float AMatchGameMode::GetBattleTime() const
{
	return BattleTime;
}

float AMatchGameMode::GetTimeBetweenBattles() const
{
	return TimeBetweenBattles;
}

float AMatchGameMode::GetTimeBeforeReturningToMainMenu() const
{
	return TimeBeforeReturningToMainMenu;
}

void AMatchGameMode::StartBattle()
{
	SetBattleState(BattleState::Battle);
}

void AMatchGameMode::NotifyBattleIsOver()
{
	SetBattleState(BattleState::PostBattle);
}

void AMatchGameMode::StartPostGame()
{
	SetBattleState(BattleState::PostGame);
}

void AMatchGameMode::NotifyPostBattleIsOver()
{
	if (!MatchSetup->WasEveryPlayerDefender())
	{
		SetBattleState(BattleState::Battle);
	}
	else
	{
		StartPostGame();
	}
}

void AMatchGameMode::NotifyPostGameIsOver()
{
	EndMatch();
}

bool AMatchGameMode::ReadyToStartMatch_Implementation()
{
	//	for now we keep play in editor alive when we are not coming from SeamlessTravel
	//Super tries to start the match, we dont want that, if we are ready to start,
	//we want it to basically only return true when every player has chosen their Template...
	//Tick checks this constantly
	
	if (!bDelayedStart)
	{
		return Super::ReadyToStartMatch_Implementation();
	}

	return MatchGameState->PlayerReadyCount == NumPlayers;
}

void AMatchGameMode::InitSeamlessTravelPlayer(AController* NewController)
{
	//Super is implemented in here because we change its behaviour a bit, so dont call Super
	const TWeakObjectPtr<APlayerController> NewPC = Cast<APlayerController>(NewController);

	/*FString ErrorMessage;
	if (!UpdatePlayerStartSpot(NewController, TEXT(""), ErrorMessage))
	{
		UE_LOG(LogGameMode, Warning, TEXT("InitSeamlessTravelPlayer: %s"), *ErrorMessage);
	}*/

	if (NewPC != nullptr)
	{
		NewPC->PostSeamlessTravel();

		
		if (MustSpectate(NewPC.Get()))
		{
			NewPC->StartSpectatingOnly();
		}
		else
		{
			MatchSetup->AddPlayer(NewPC);
			SetSeamlessTravelViewTarget(NewPC.Get());

			NewPC->bPlayerIsWaiting = true;
			NewPC->ChangeState(NAME_PrePlay);
			NewPC->ClientGotoState(NAME_PrePlay);
			NumPlayers++;
			NumTravellingPlayers--;
		}
	}
	else
	{
		NumBots++;
	}
}

bool AMatchGameMode::UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage)
{
	return Super::UpdatePlayerStartSpot(Player, Portal, OutErrorMessage);
}
