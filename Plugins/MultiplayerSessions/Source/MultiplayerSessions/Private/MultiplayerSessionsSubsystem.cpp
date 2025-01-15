// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"

//Default settings
const FName UMultiplayerSessionsSubsystem::NAME_MatchType = FName("MatchType");

const FString DEFAULT_MatchType = FString("DeathMatch");
const int32 DEFAULT_NumPublicConnections = 4;
const int32 DEFAULT_MaxSearchResults = 1000;

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
{
	CreateSessionCompleteDelegate.BindUObject(this, &ThisClass::OnCreateSessionComplete);
	StartSessionCompleteDelegate.BindUObject(this, &ThisClass::OnStartSessionComplete);
	DestroySessionCompleteDelegate.BindUObject(this, &ThisClass::OnDestroySessionComplete);
	FindSessionsCompleteDelegate.BindUObject(this, &ThisClass::OnFindSessionsComplete);
	JoinSessionCompleteDelegate.BindUObject(this, &ThisClass::OnJoinSessionComplete);
}

void UMultiplayerSessionsSubsystem::CreateSession(ULocalPlayer* LocalPlayer, int32 NumPublicConnections, const FString MatchType)
{
	if (!HasSessionInterface() || !LocalPlayer)
	{
		OnCreateSession.Broadcast(false);
		return;
	}

	FNamedOnlineSession* ExistingSession = GetSession();
	if (ExistingSession != nullptr)
	{
		//TODO Consider sending a message stating "A session already exists. Disconnecting first..." or something similar.
		//S2L20 Create Session Around 2:50 mark he does not use return. S2L28 discusses the topic as well.
		//Consider storing numpubliconnections and matchtype if the code should disconnect and automatically try to start the new session.

		DestroySession(ExistingSession->SessionName);
		return;
	}

	SetSessionSettings(NumPublicConnections, MatchType);

	CreateSessionCompleteDelegateHandle = OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	if (!OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings))
		OnCreateSessionComplete(NAME_GameSession, false);
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (HasSessionInterface())
		OnlineSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

	OnCreateSession.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::StartSession(FName SessionName)
{
	if (!HasSessionInterface())
	{
		OnStartSession.Broadcast(false);
		return;
	}

	StartSessionCompleteDelegateHandle = OnlineSessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);
	if (!OnlineSessionInterface->StartSession(SessionName))
		OnStartSessionComplete(SessionName, false);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (HasSessionInterface())
		OnlineSessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);

	OnStartSession.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::FindSessions(ULocalPlayer* LocalPlayer, int32 MaxSearchResults)
{
	if (!HasSessionInterface())
	{
		OnFindSessions.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	FindSessionsCompleteDelegateHandle = OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	SetSessionSearch(MaxSearchResults);

	if (!OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
		OnFindSessionsComplete(false);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (HasSessionInterface())
		OnlineSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

	OnFindSessions.Broadcast(SessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::JoinSessionByIndex(ULocalPlayer* LocalPlayer, int32 SessionIndex)
{
	if (!SessionSearch.IsValid()) return;
	if (SessionSearch->SearchResults.Num() <= SessionIndex) return;

	JoinSession(LocalPlayer, SessionSearch->SearchResults[SessionIndex]);
}

void UMultiplayerSessionsSubsystem::JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& Session)
{
	if (!HasSessionInterface())
	{
		OnJoinSessionComplete(NAME_GameSession, EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
	if (!OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Session))
		OnJoinSessionComplete(NAME_GameSession, EOnJoinSessionCompleteResult::UnknownError);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (HasSessionInterface())
		OnlineSessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

	OnJoinSession.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::DestroySession(FName SessionName)
{
	if (!HasSessionInterface())
	{
		OnDestroySession.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = OnlineSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!OnlineSessionInterface->DestroySession(SessionName))
		OnDestroySessionComplete(SessionName, false);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (HasSessionInterface())
		OnlineSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

	OnDestroySession.Broadcast(bWasSuccessful);
}

FNamedOnlineSession* UMultiplayerSessionsSubsystem::GetSession(FName SessionName)
{
	return HasSessionInterface() ? OnlineSessionInterface->GetNamedSession(SessionName) : nullptr;
}

bool UMultiplayerSessionsSubsystem::GetConnectionString(const FName& SessionName, FString& ConnectionString)
{
	return HasSessionInterface() ? OnlineSessionInterface->GetResolvedConnectString(SessionName, ConnectionString) : false;
}

void UMultiplayerSessionsSubsystem::ServerTravel(APlayerController* PlayerController, FString MapURL, bool bAbsolute, bool bListenServer)
{
	if (bListenServer)
		MapURL = FString::Printf(TEXT("%s?listen"), *MapURL);

	if (PlayerController)
	{
		if (!PlayerController->GetWorld()->ServerTravel(MapURL, bAbsolute))
		{
			//TODO Create a broadcast delegate to notify travel failure.
		}
	}
}

void UMultiplayerSessionsSubsystem::ClientTravel(FName SessionName, APlayerController* PlayerController, ETravelType TravelType, bool bSeamless)
{
	FString ConnectionString;
	if (PlayerController && GetConnectionString(SessionName, ConnectionString))
	{
		PlayerController->ClientTravel(ConnectionString, TravelType, bSeamless);
	}
}

void UMultiplayerSessionsSubsystem::SetSessionSettings(int32 NumPublicConnections, const FString MatchType)
{
	//Should default standards be enforced?
	//NumPublicConnections = FMath::Clamp(NumPublicConnections, 1, 1000);
	//if (!MatchType.IsEmpty())
	//	MatchType = DEFAULT_MatchType;

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

	SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = (OnlineSubsystem == nullptr) ? true : (OnlineSubsystem->GetSubsystemName().IsEqual("NULL")) ? true : false;
	SessionSettings->NumPublicConnections = NumPublicConnections;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bUseLobbiesIfAvailable = true; //NOTE Fix for UE 5
	SessionSettings->Set(NAME_MatchType, MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->BuildUniqueId = 1;
}

TSharedPtr<FOnlineSessionSettings> UMultiplayerSessionsSubsystem::GetSessionSettings()
{
	if (!SessionSettings.IsValid())
		SetSessionSettings(DEFAULT_NumPublicConnections, DEFAULT_MatchType);

	return SessionSettings;
}

bool UMultiplayerSessionsSubsystem::HasSessionInterface()
{
	if (!OnlineSessionInterface)
	{
		IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
		if (OnlineSubsystem)
			OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
	}

	return OnlineSessionInterface.IsValid();
}

void UMultiplayerSessionsSubsystem::SetSessionSearch(int32 MaxSearchResults)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = MaxSearchResults;
	SessionSearch->bIsLanQuery = (OnlineSubsystem == nullptr) ? true : (OnlineSubsystem->GetSubsystemName().IsEqual("NULL")) ? true : false;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
}

TSharedPtr<FOnlineSessionSearch> UMultiplayerSessionsSubsystem::GetSessionSearch()
{
	if (!SessionSearch.IsValid())
		SetSessionSearch(DEFAULT_MaxSearchResults);

	return SessionSearch;
}

bool UMultiplayerSessionsSubsystem::IsInSession(FName SessionName)
{
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (LocalPlayer == nullptr) return false;

	return HasSessionInterface() ? OnlineSessionInterface->IsPlayerInSession(SessionName, *LocalPlayer->GetPreferredUniqueNetId()) : false;
}