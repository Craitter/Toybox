// Fill out your copyright notice in the Description page of Project Settings.



#include "Widget/MainMenuWidget.h"

#include "Components/Button.h"
#include "Components/CircularThrobber.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/WidgetSwitcher.h"
#include "Framework/ToyboxGameInstance.h"
#include "Widget/ServerListEntryWidget.h"


void UMainMenuWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	//Checking if we can access the Objects if not there is an issue and we log it
	if (!ensure(IsValid(Switcher))) return;
	if (!ensure(IsValid(HostButton))) return;
	if (!ensure(IsValid(JoinMenuButton))) return;
	if (!ensure(IsValid(JoinButton))) return;
	if (!ensure(IsValid(ControlsButton))) return;
	if (!ensure(IsValid(ReturnButton))) return;
	if (!ensure(IsValid(QuitGameButton))) return;
	if (!ensure(IsValid(ServerList))) return;
	if (!ensure(IsValid(ServerName))) return;
	if (!ensure(IsValid(LoadingThrobber))) return;
	
	//Binding Delegates and Setting default behaviour
	HostButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedHostButton);
	HostButton->SetIsEnabled(false);
	JoinMenuButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedJoinMenuButton);
	JoinButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedJoinButton);
	JoinButton->SetIsEnabled(false);
	ReturnButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedReturnButton);
	ReturnFromControlsButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedReturnButton);
	RefreshButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedRefreshButton);
	QuitGameButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedQuitGameButton);
	ServerName->OnTextChanged.AddDynamic(this, &ThisClass::OnServerNameEntered);
	LoadingThrobber->SetVisibility(ESlateVisibility::Hidden);
	ControlsButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedControlsMenuButton);
	
	//Caching our Custom GameInstance (should not change during Game)
	GameInstance = Cast<UToyboxGameInstance>(GetGameInstance());
}

void UMainMenuWidget::NativeDestruct()
{
	//Clearing all delegates
	ClearServerList();
	if (IsValid(Switcher)) HostButton->OnClicked.Clear();
	if (IsValid(JoinMenuButton)) JoinMenuButton->OnClicked.Clear();
	if (IsValid(JoinButton)) JoinButton->OnClicked.Clear();
	if (IsValid(ReturnButton)) ReturnButton->OnClicked.Clear();
	if (IsValid(RefreshButton)) RefreshButton->OnClicked.Clear();
	if (IsValid(QuitGameButton)) QuitGameButton->OnClicked.Clear();
	if (IsValid(ServerName)) ServerName->OnTextChanged.Clear();
	
	Super::NativeDestruct();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMainMenuWidget::OnClickedHostButton()
{
	if (!GameInstance.IsValid() || !IsValid(ServerName))
	{
		return;
	}
	GameInstance->Host(ServerName->GetText());
}

void UMainMenuWidget::OnClickedJoinMenuButton()
{
	Switcher->SetActiveWidget(JoinMenuPanel);
	check(StartSessionSearch())
}

void UMainMenuWidget::OnClickedJoinButton()
{
	if (!GameInstance.IsValid())
	{
		return;
	}

	StartJoinLobbyLoading();
	ReturnButton->SetIsEnabled(false);
	
	for (const TWeakObjectPtr<UServerListEntryWidget> ServerListEntry : ServerListEntries)
	{
		if (ServerListEntry.IsValid() && ServerListEntry->IsSelectedEntry())
		{
			GameInstance->Join(ServerListEntry->GetCachedIndex());
			return;
		}
	}
}

void UMainMenuWidget::OnClickedReturnButton()
{
	if (!IsValid(ServerList) || !IsValid(MainMenuPanel) || !IsValid(Switcher))
	{
		return;
	}
	LoadingThrobber->SetVisibility(ESlateVisibility::Hidden);
	Switcher->SetActiveWidget(MainMenuPanel);
	ClearServerList();
}

void UMainMenuWidget::OnClickedRefreshButton()
{
	StartSessionSearch();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMainMenuWidget::OnClickedQuitGameButton()
{
	const TWeakObjectPtr<APlayerController> PlayerController = GetOwningPlayer();
	if (!PlayerController.IsValid())
	{
		return;
	}
	PlayerController->ConsoleCommand(TEXT("quit"));
}

void UMainMenuWidget::OnClickedControlsMenuButton()
{
	if (!IsValid(Switcher))
	{
		return;
	}
	Switcher->SetActiveWidget(ControlsMenuPanel);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UMainMenuWidget::OnServerNameEntered(const FText& NewText)
{
	if (!IsValid(HostButton))
	{
		return;
	}
	if (NewText.IsEmpty())
	{
		HostButton->SetIsEnabled(false);
	}
	else
	{
		HostButton->SetIsEnabled(true);
	}
}

void UMainMenuWidget::OnSessionSearchResultsReady(const TArray<FServerInfo>& SearchResults)
{
	if (!GameInstance.IsValid() || !IsValid(ServerList) || !IsValid(JoinButton))
	{
		return;
	}
	EndJoinLobbyLoading();
	
	if (GameInstance->OnSearchResultsReadyDelegate.Remove(SessionSearchDelegateHandle))
	{
		SessionSearchDelegateHandle.Reset();
	}
	for (const FServerInfo& Result : SearchResults)
	{
		TObjectPtr<UServerListEntryWidget> NewEntry = CreateWidget<UServerListEntryWidget>(ServerList, ServerListEntryClass);
		if (!IsValid(NewEntry))
		{
			continue;
		}
		NewEntry->SetServerListEntry(Result.Index, FText::FromString(Result.ServerName));
		NewEntry->OnServerSelected.BindUObject(this, &ThisClass::OnServerSelected);
		ServerList->AddChild(NewEntry);
		ServerListEntries.Add(NewEntry);
	}
}

void UMainMenuWidget::OnServerSelected(const TWeakObjectPtr<UServerListEntryWidget> SelectedServerListEntryEntry)
{
	if (!IsValid(ServerList) || ServerListEntries.IsEmpty())
	{
		return;
	}
	for (TWeakObjectPtr<UServerListEntryWidget> ServerListEntry : ServerListEntries)
	{
		if (ServerListEntry != SelectedServerListEntryEntry && ServerListEntry.IsValid())
		{
			ServerListEntry->Reset();
		}
	}
	JoinButton->SetIsEnabled(true);
}

bool UMainMenuWidget::StartSessionSearch()
{
	if (!GameInstance.IsValid() || !IsValid(ServerList))
	{
		return false;
	}
	
	StartJoinLobbyLoading();
	ClearServerList();
	
	SessionSearchDelegateHandle = GameInstance->OnSearchResultsReadyDelegate.AddUObject(this, &ThisClass::OnSessionSearchResultsReady);
	GameInstance->FindSessions();
	
	return true;
}

void UMainMenuWidget::ClearServerList()
{
	if (!IsValid(ServerList) || !IsValid(JoinButton))
	{
		return;
	}
	ServerListEntries.Empty();
	ServerList->ClearChildren();
	JoinButton->SetIsEnabled(false);
}

void UMainMenuWidget::StartJoinLobbyLoading() const
{
	LoadingThrobber->SetVisibility(ESlateVisibility::HitTestInvisible);
	JoinButton->SetIsEnabled(false);
	RefreshButton->SetIsEnabled(false);
}

void UMainMenuWidget::EndJoinLobbyLoading() const
{
	LoadingThrobber->SetVisibility(ESlateVisibility::Hidden);
	RefreshButton->SetIsEnabled(true);
}
