// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionMenu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"

bool USessionMenu::Initialize()
{
	if (!Super::Initialize()) return false;

	UGameInstance* GI = GetGameInstance();
	if (GI)
		MultiplayerSessionsSubsystem = GI->GetSubsystem<UMultiplayerSessionsSubsystem>();

	if (HostButton)
		HostButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnHostClicked);
	if (ClientButton)
		ClientButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClientClicked);
	if (HideButton)
		HideButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnHideClicked);
	if (CloseButton)
		CloseButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnCloseClicked);
	if (QuitButton)
		QuitButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnQuitClicked);

	return true;
}

void USessionMenu::Show_Implementation(bool InIsFocusable)
{
	if (!IsInViewport())
		AddToViewport();

	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(InIsFocusable);

	SubscribeToSessionDelegates();

	APlayerController* PC = GetOwningPlayer<APlayerController>();
	if (PC)
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputModeData);
		PC->SetShowMouseCursor(true);
	}
}

void USessionMenu::Hide_Implementation()
{
	if (IsVisible())
		SetVisibility(ESlateVisibility::Hidden);

	UnsubscribeFromSessionDelegates();

	APlayerController* PC = GetOwningPlayer<APlayerController>();
	if (PC)
	{
		FInputModeGameOnly InputModeData;
		PC->SetInputMode(InputModeData);
		PC->SetShowMouseCursor(false);
	}
}

void USessionMenu::Close_Implementation()
{
	if (IsVisible())
		SetVisibility(ESlateVisibility::Hidden);

	RemoveFromParent();

	APlayerController* PC = GetOwningPlayer<APlayerController>();
	if (PC)
	{
		FInputModeGameOnly InputModeData;
		PC->SetInputMode(InputModeData);
		PC->SetShowMouseCursor(false);
	}
}

void USessionMenu::NativeDestruct()
{
	if (IsVisible())
		SetVisibility(ESlateVisibility::Hidden);

	RemoveFromParent();

	UnsubscribeFromSessionDelegates();

	Super::NativeDestruct();
}

void USessionMenu::SubscribeToSessionDelegates()
{
	if (MultiplayerSessionsSubsystem)
	{
		OnCreateSessionDelegateHandle = MultiplayerSessionsSubsystem->OnCreateSession.AddUObject(this, &ThisClass::OnCreateSession);
		OnStartSessionDelegateHandle = MultiplayerSessionsSubsystem->OnStartSession.AddUObject(this, &ThisClass::OnStartSession);
		OnFindSessionsDelegateHandle = MultiplayerSessionsSubsystem->OnFindSessions.AddUObject(this, &ThisClass::OnFindSessions);
		OnJoinSessionDelegateHandle = MultiplayerSessionsSubsystem->OnJoinSession.AddUObject(this, &ThisClass::OnJoinSession);
		OnDestroySessionDelegateHandle = MultiplayerSessionsSubsystem->OnDestroySession.AddUObject(this, &ThisClass::OnDestroySession);
	}
}

void USessionMenu::UnsubscribeFromSessionDelegates()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->OnCreateSession.Remove(OnCreateSessionDelegateHandle);
		MultiplayerSessionsSubsystem->OnStartSession.Remove(OnStartSessionDelegateHandle);
		MultiplayerSessionsSubsystem->OnFindSessions.Remove(OnFindSessionsDelegateHandle);
		MultiplayerSessionsSubsystem->OnJoinSession.Remove(OnJoinSessionDelegateHandle);
		MultiplayerSessionsSubsystem->OnDestroySession.Remove(OnDestroySessionDelegateHandle);
	}
}

void USessionMenu::OnCreateSession(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCreateSession_Implementation %d"), bWasSuccessful);

	if (!bWasSuccessful)
	{
		//TODO Provide feedback to the player, enable input, etc...
		return;
	}

	if (MultiplayerSessionsSubsystem)
		MultiplayerSessionsSubsystem->ServerTravel(GetOwningPlayer(), FString("/Game/Maps/Lobby"), false, true);
}

void USessionMenu::OnStartSession(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("OnStartSession %d"), bWasSuccessful);

	if (!bWasSuccessful)
	{
		//TODO Provide feedback to the player, enable input, etc...
		return;
	}
}

void USessionMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		//TODO Provide feedback to the player, enable input, etc...
		return;
	}

	if (MultiplayerSessionsSubsystem)
	{
		if (SessionResults.Num() > 0)
		{
			for (auto Result : SessionResults)
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.f, bWasSuccessful ? FColor::Green : FColor::Red, FString::Printf(TEXT("Session found")));

				FString MatchType;
				if (Result.Session.SessionSettings.Get<FString>(UMultiplayerSessionsSubsystem::NAME_MatchType, MatchType))
				{
					if (GEngine)
						GEngine->AddOnScreenDebugMessage(-1, 15.f, bWasSuccessful ? FColor::Green : FColor::Red, FString::Printf(TEXT("Session found: %s %i"), *(Result.GetSessionIdStr() + " " + Result.Session.OwningUserName), bWasSuccessful));

					//NOTE Coding this to auto-join at the moment.
					MultiplayerSessionsSubsystem->JoinSession(GetOwningLocalPlayer(), Result);
				}
			}
		}
		else
		{
			//TODO No results. React accordingly.
		}
	}
}

void USessionMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	bool bWasSuccessful = false;
	FString Message;
	switch (Result)
	{
		case EOnJoinSessionCompleteResult::Success:
		{
			bWasSuccessful = true;
			Message = FString("Success");
			break;
		}
		default:
		{
			Message = FString("Failure");
			break;
		}
	}

	if (!bWasSuccessful)
	{
		//TODO Provide feedback to the player, enable input, etc...
		return;
	}

	if (MultiplayerSessionsSubsystem)
		MultiplayerSessionsSubsystem->ClientTravel(NAME_GameSession, GetOwningPlayer(), ETravelType::TRAVEL_Absolute, false);
}

void USessionMenu::OnDestroySession(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("OnDestroySession %d"), bWasSuccessful);
}

void USessionMenu::OnHostClicked()
{
	if (MultiplayerSessionsSubsystem)
		MultiplayerSessionsSubsystem->CreateSession(GetOwningLocalPlayer(), 4, FString("DeathMatch"));
}

void USessionMenu::OnClientClicked()
{
	if (MultiplayerSessionsSubsystem)
		MultiplayerSessionsSubsystem->FindSessions(GetOwningLocalPlayer(), 1000);
}

void USessionMenu::OnCloseClicked()
{
	Close();
}

void USessionMenu::OnHideClicked()
{
	Hide();
}

void USessionMenu::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}