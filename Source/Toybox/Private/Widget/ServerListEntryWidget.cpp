// Fill out your copyright notice in the Description page of Project Settings.



#include "Widget/ServerListEntryWidget.h"

#include "Components/TextBlock.h"
#include "Engine/Font.h"


void UServerListEntryWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!ensure(IsValid(SelectButton))) return;
	
	SelectButton->OnClicked.AddDynamic(this, &ThisClass::OnSelectButtonClicked);
	NormalBackgroundColor = SelectButton->GetBackgroundColor();
}

void UServerListEntryWidget::NativeDestruct()
{
	if (IsValid(SelectButton)) SelectButton->OnClicked.Clear();
	
	Super::NativeDestruct();
}

bool UServerListEntryWidget::IsSelectedEntry() const
{
	return bIsSelected;
}

int32 UServerListEntryWidget::GetCachedIndex() const
{
	return CachedIndex;
}

void UServerListEntryWidget::SetServerListEntry(const int32 ServerIndex, const FText& HostServerName)
{
	if (!IsValid(Index) || !IsValid(ServerName))
	{
		return;
	}
	Index->SetText(FText::AsNumber(ServerIndex));
	Index->SetFont(NumberFont);
	ServerName->SetText(HostServerName);
	CachedIndex = ServerIndex;
}

void UServerListEntryWidget::Reset()
{
	if (!IsValid(SelectButton))
	{
		return;
	}
	bIsSelected = false;
	SelectButton->SetBackgroundColor(NormalBackgroundColor);
}

void UServerListEntryWidget::OnSelectButtonClicked()
{
	if(!IsValid(SelectButton) || !OnServerSelected.IsBound())
	{
		return;
	}
	SelectButton->SetBackgroundColor(FLinearColor::Blue);
	bIsSelected = true;
	OnServerSelected.Execute(this);
}
