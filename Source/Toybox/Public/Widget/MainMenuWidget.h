// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UCircularThrobber;
struct FServerInfo;
class UToyboxGameInstance;
class UEditableTextBox;
class UServerListEntryWidget;
class UScrollBox;
class UButton;
class UWidgetSwitcher;
/**
 * 
 */

UCLASS()
class TOYBOX_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;

	virtual void NativeDestruct() override;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> Switcher = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> MainMenuPanel = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> JoinMenuPanel = {nullptr};
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> ControlsMenuPanel = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinMenuButton = {nullptr};
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnButton = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RefreshButton = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ControlsButton = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnFromControlsButton = {nullptr};
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitGameButton = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ServerList = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> ServerName = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UServerListEntryWidget> ServerListEntryClass = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCircularThrobber> LoadingThrobber = {nullptr};

	//Begin Native Widget Callbacks
	UFUNCTION()
	void OnClickedHostButton();
	UFUNCTION()
	void OnClickedJoinMenuButton();
	UFUNCTION()
	void OnClickedJoinButton();
	UFUNCTION()
	void OnClickedReturnButton();
	UFUNCTION()
	void OnClickedRefreshButton();
	UFUNCTION()
	void OnClickedQuitGameButton();
	UFUNCTION()
	void OnClickedControlsMenuButton();
	UFUNCTION()
	void OnServerNameEntered(const FText& NewText);
	//End Native Widget Callbacks

	//Listening to callback from GameInstance, when search results are done
	void OnSessionSearchResultsReady(const TArray<FServerInfo>& SearchResults);
	//Listening to all Server List entries
	void OnServerSelected(const TWeakObjectPtr<UServerListEntryWidget> SelectedServerListEntryEntry);
private:
	//Cached Objects
	TWeakObjectPtr<UToyboxGameInstance> GameInstance = {nullptr};
	//This array is so we dont have to cast when going through all children and resetting them as not selected
	TArray<TWeakObjectPtr<UServerListEntryWidget>> ServerListEntries;
	FDelegateHandle SessionSearchDelegateHandle;
	
	//helper functions
	bool StartSessionSearch();
	void ClearServerList();

	void StartJoinLobbyLoading() const;
	void EndJoinLobbyLoading() const;
};


