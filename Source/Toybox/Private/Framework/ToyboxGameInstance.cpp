// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ToyboxGameInstance.h"

#include "GameMapsSettings.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Actors/MainMenuGameMode.h"
#include "Online/OnlineSessionNames.h"
#include "Toybox/Toybox.h"

const static FName SESSION_NAME = NAME_GameSession;
const static FName SESSION_NAME_KEY = TEXT("ServerName");
const static FName SUBSYSTEM_NULL = TEXT("NULL");
static constexpr uint32 MAX_SEARCH_RESULTS = 1000;
const static FString DEFAULT_MULTIPLAYER_MAP = TEXT("/Game/Levels/LVL_DefaultEditorMap?listen");


UToyboxGameInstance::UToyboxGameInstance()
{
	SessionSettings.bAllowInvites = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	SessionSettings.bAntiCheatProtected = false;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.bShouldAdvertise = true;
	// SessionSettings.BuildUniqueId //Not sure what this is doing
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bUsesStats = true;
	SessionSettings.NumPrivateConnections = 0.0f;
	SessionSettings.NumPublicConnections = 10.0f;
}

void UToyboxGameInstance::Init()
{
	Super::Init();

	OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem != nullptr)
	{
		if (OnlineSubsystem->GetSubsystemName() == SUBSYSTEM_NULL) //depending on Subsystem
		{
			//this if statement is mainly for testing since OSS:NULL is only used for LAN Matches
			SessionSettings.bIsLANMatch = true;
		}
		UE_LOG(LogToyboxOnlineService, Log, TEXT("Current Online Subsystem: %s"), *OnlineSubsystem->GetSubsystemName().ToString());
		SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UToyboxGameInstance::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UToyboxGameInstance::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UToyboxGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UToyboxGameInstance::OnJoinSessionComplete);
		}
		else
		{
			UE_LOG(LogToyboxOnlineService, Error, TEXT("%s() No SessionInterface found "), *FString(__FUNCTION__));
		}
	}
	else
	{
		UE_LOG(LogToyboxOnlineService, Error, TEXT("%s() No OnlineSubsystem found "), *FString(__FUNCTION__));
	}
	check(GetEngine())
	//GEngine->OnNetworkFailure().AddUObject(this, &UToyboxGameInstance::OnNetworkFailure);
}

void UToyboxGameInstance::Host(const FText& ServerName)
{
	if (SessionInterface.IsValid())
	{
		CustomServerName = ServerName.ToString();
		const FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (ExistingSession != nullptr)
		{
			SessionInterface->DestroySession(SESSION_NAME);
			UE_LOG(LogToyboxOnlineService, Log, TEXT("A Session (%s) was still running, destroying it and then create a new one to host"), *SESSION_NAME.ToString());
		}
		else
		{
			CreateSession();
		}
	}
}

void UToyboxGameInstance::Join(const int32 ServerIndex) const
{
	const FOnlineSessionSearchResult* Result = GetOnlineSessionSearchResultByIndex(ServerIndex);
	if (Result != nullptr && Result->IsValid())
	{
		UE_LOG(LogToyboxOnlineService, Verbose, TEXT("Starting to join Session"));
		SessionInterface->JoinSession(LOCAL_USER_INDEX, SESSION_NAME, *Result);
	}
	else
	{
		UE_LOG(LogToyboxOnlineService, Error, TEXT("Session Interface was Null %s"), *FString(__FUNCTION__));
	}
}

void UToyboxGameInstance::FindSessions()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		if (OnlineSubsystem != nullptr && OnlineSubsystem->GetSubsystemName() == SUBSYSTEM_NULL)
		{
			SessionSearch->bIsLanQuery = true;
		}
		else
		{
			SessionSearch->bIsLanQuery = false;
		}
		SessionSearch->MaxSearchResults = MAX_SEARCH_RESULTS;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		// SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
		SessionInterface->FindSessions(LOCAL_USER_INDEX, SessionSearch.ToSharedRef());
		UE_LOG(LogToyboxOnlineService, Verbose, TEXT("Starting to find Sessions"));
	}
	else
	{
		UE_LOG(LogToyboxOnlineService, Error, TEXT("Session Search was Null %s"), *FString(__FUNCTION__));
	}
}

void UToyboxGameInstance::Refresh()
{
	FindSessions();
}

void UToyboxGameInstance::CreateSession()
{
	if (SessionInterface.IsValid())
	{
		//Adding a key and a value to the settings for basic names, so its easier to overview and identify dead sessions that might be displayed
		SessionSettings.Set<FString>(SESSION_NAME_KEY, CustomServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		
		SessionInterface->CreateSession(LOCAL_USER_INDEX, SESSION_NAME, SessionSettings);
		UE_LOG(LogToyboxOnlineService, Log, TEXT("Creating Session with name %s "), *CustomServerName);
	}
	else
	{
		UE_LOG(LogToyboxOnlineService, Error, TEXT("No SessionInterface found %s()"), *FString(__FUNCTION__));
	}
}

void UToyboxGameInstance::OnCreateSessionComplete(const FName SessionName, const bool bSuccess) const
{
	check(GetWorld())
	if (!bSuccess)
	{
		UE_LOG(LogToyboxOnlineService, Error, TEXT("Creating Session %s failed %s()"), *FString(__FUNCTION__), *SessionName.ToString());
		return;
	}
	
	if (GetWorld()->ServerTravel(FString::Printf(TEXT("%s?listen"), *GetLobbyLevel())))
	{
		UE_LOG(LogToyboxOnlineService, Log, TEXT("Session %s created, Traveling to Map %s"), *SessionName.ToString(), *DEFAULT_MULTIPLAYER_MAP);
	}
	else
	{
		UE_LOG(LogToyboxOnlineService, Error, TEXT("Travel to Map %s with Session %s failed %s()"), *DEFAULT_MULTIPLAYER_MAP, *SessionName.ToString(), *FString(__FUNCTION__));
	}
}

void UToyboxGameInstance::OnDestroySessionComplete(const FName SessionName, const bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogToyboxOnlineService, Verbose, TEXT("Session (%s) got destroyed succesfull"), *SessionName.ToString());
		CreateSession();
	}
	else
	{
		UE_LOG(LogToyboxOnlineService, Error, TEXT("Online Session %s could not be destroyed %s"), *SessionName.ToString(), *FString(__FUNCTION__));
	}
}

void UToyboxGameInstance::OnFindSessionsComplete(bool bWasSuccessful) const
{
	UE_LOG(LogToyboxOnlineService, Log, TEXT("Find Sessions Completed"));

	if (bWasSuccessful && SessionSearch.IsValid())
	{
		TArray<FServerInfo> FoundSessionsServerInfos;
		for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
		{
			const FOnlineSessionSearchResult* Result = GetOnlineSessionSearchResultByIndex(i);
			if (Result != nullptr && Result->IsValid())
			{				
				FServerInfo ServerInfo;
				ServerInfo.Index = i;
				Result->Session.SessionSettings.Get(SESSION_NAME_KEY, ServerInfo.ServerName);
				if (ServerInfo.ServerName.IsEmpty())
				{
					UE_LOG(LogToyboxOnlineService, Warning, TEXT("Found Session at Index %d had no ServerName, falling back to SessionID %s"), ServerInfo.Index, *Result->GetSessionIdStr());
					ServerInfo.ServerName = Result->GetSessionIdStr();
				}
				else
				{
					UE_LOG(LogToyboxOnlineService, Log, TEXT("Found Session at Index %d with Name %s"), ServerInfo.Index, *ServerInfo.ServerName);
				}
				FoundSessionsServerInfos.Add(ServerInfo);
			}
			else
			{
				UE_LOG(LogToyboxOnlineService, Warning, TEXT("Found Session at Index %d Is Invalid!"), i);
			}
		}
		OnSearchResultsReadyDelegate.Broadcast(FoundSessionsServerInfos);
	}
	else
	{
		UE_LOG(LogToyboxOnlineService, Error, TEXT("Session Search was either not valid or not Successful %s"), *FString(__FUNCTION__));
	}
}

void UToyboxGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) const
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		//There is a way to display standard c++ enums as strings, https://forums.unrealengine.com/t/conversion-of-enum-to-string/337869/8
		UE_LOG(LogToyboxOnlineService, Error, TEXT("Couldnt Join Session %s, %s()"), *SessionName.ToString(), *FString(__FUNCTION__));
		return;
	}
	
	FString AddressToJoin;
	if (SessionInterface.IsValid() && SessionInterface->GetResolvedConnectString(SessionName, AddressToJoin))
	{
		const TWeakObjectPtr<APlayerController> PlayerController = GetFirstLocalPlayerController();
		if (!ensure(PlayerController.IsValid())) return;
		
		UE_LOG(LogToyboxOnlineService, Log, TEXT("Joining Session %s, AddressToJoin %s"), *SessionName.ToString(), *AddressToJoin);
		PlayerController->ClientTravel(AddressToJoin, TRAVEL_Absolute);
	}
	else
	{
		UE_LOG(LogToyboxOnlineService, Error, TEXT("Couldnt Join Session %s, Session Interface was Invalid or there was no connection String %s()"), *SessionName.ToString(), *FString(__FUNCTION__));
	}
}

void UToyboxGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type Type, const FString& String) const
{
	ensure(IsInGameThread());
	UE_LOG(LogToyboxOnlineService, Error, TEXT("NetworkError: In World %s, with NetDriver %s, Error Context: %s "), *World->GetName(), *NetDriver->GetName(), *String);
}

void UToyboxGameInstance::BackToMainMenu() const
{
	const TWeakObjectPtr<APlayerController> Controller = GetFirstLocalPlayerController();
	if (!ensure(Controller.IsValid())) return;

	Controller->ClientTravel(UGameMapsSettings::GetGameDefaultMap(), TRAVEL_Absolute, false);
}

FOnlineSessionSearchResult* UToyboxGameInstance::GetOnlineSessionSearchResultByIndex(int32 SearchResultIndex) const
{
	if (SessionInterface.IsValid() && SessionSearch.IsValid() && SessionSearch->SearchResults.IsValidIndex(SearchResultIndex))
	{
		return &SessionSearch->SearchResults[SearchResultIndex];
	}
	UE_LOG(LogToyboxOnlineService, Warning, TEXT("Unable to get Search Result for %d %s()"), SearchResultIndex, *FString(__FUNCTION__));
	return nullptr;
}

FString UToyboxGameInstance::GetLobbyLevel() const
{
	check(GetWorld())

	const TWeakObjectPtr<AMainMenuGameMode> MenuGameMode = Cast<AMainMenuGameMode>(GetWorld()->GetAuthGameMode());
	if (MenuGameMode.IsValid())
	{
		return MenuGameMode->GetLobbyMap().GetAssetName();
	}
	else
	{
		UE_LOG(LogToyboxOnlineService, Warning, TEXT("Unable to get the Lobby Map, make sure to fill out the MainMenuGameMode with a LobbyMap"));
	}
	return FString();
}
