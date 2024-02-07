// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ToyboxGameInstance.generated.h"

class IOnlineSubsystem;
/**
 * 
 */
USTRUCT()
struct FServerInfo
{
	GENERATED_BODY()

	FString ServerName = FString();
	
	int32 Index = 0;
};

//Create, Join, Host, Find is based of Unreal Documentation for the Session Interfaces, 
//https://docs.unrealengine.com/4.26/en-US/ProgrammingAndScripting/Online/
//https://docs.unrealengine.com/5.0/en-US/networking-and-multiplayer-in-unreal-engine/
//A lot of guides use the module "OnlineSubsystemUtils" for Helper functions, however right now this is getting along very well without
//The base setup is taken from an udemy course as a base and over the last month improved by me
//If different functionality gets added to the GI we have to make it a subsystem and extract the network stuff or the new stuff


DECLARE_MULTICAST_DELEGATE_OneParam(FOnSearchResultsReadyDelegate, const TArray<FServerInfo>&)

UCLASS()
class TOYBOX_API UToyboxGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UToyboxGameInstance();

	virtual void Init() override;
	
	UFUNCTION(exec)
	void Host(const FText& ServerName);

	UFUNCTION(exec)
	void Join(const int32 ServerIndex) const;
	
	UFUNCTION(exec)
	void FindSessions();

	void Refresh();

	void CreateSession();

	void OnCreateSessionComplete(FName SessionName, bool bSuccess) const;

	void OnDestroySessionComplete(FName SessionName, bool bSuccess);

	void OnFindSessionsComplete(bool bWasSuccessful) const;

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) const;

	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver,	ENetworkFailure::Type Type, const FString& String) const;


	FOnSearchResultsReadyDelegate OnSearchResultsReadyDelegate;

	void BackToMainMenu() const;
	
protected:
	FOnlineSessionSearchResult* GetOnlineSessionSearchResultByIndex(int32 SearchResultIndex) const;
	
	IOnlineSubsystem* OnlineSubsystem = {nullptr};
	IOnlineSessionPtr SessionInterface = {nullptr};
	TSharedPtr<FOnlineSessionSearch> SessionSearch = {nullptr};

private:
	FString CustomServerName = FString();

	FOnlineSessionSettings SessionSettings;
	
	FString GetLobbyLevel() const; 
};

