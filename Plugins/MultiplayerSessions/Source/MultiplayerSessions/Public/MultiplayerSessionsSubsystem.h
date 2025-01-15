// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MultiplayerSessionsSubsystem.generated.h"

//Custom delegates for callbacks
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCreateSession, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnFindSessions, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnJoinSession, EOnJoinSessionCompleteResult::Type Result);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDestroySession, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStartSession, bool bWasSuccessful);

/**
 *
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:

	IOnlineSessionPtr OnlineSessionInterface;
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	//Delegate for OnlineSubsystem callbacks
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;

	//Handles for delegates for OnlineSubsystem callbacks
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	void SetSessionSettings(int32 NumPublicConnections, const FString MatchType);
	void SetSessionSearch(int32 MaxSearchResults);

protected:

	//Internal OnlineSubsystem delegate callbacks
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

public:

	UMultiplayerSessionsSubsystem();

	//Blueprint-exposed session functions
	UFUNCTION(BlueprintCallable, Category = MultiplayerSessions)
	void CreateSession(ULocalPlayer* LocalPlayer, int32 NumPublicConnections, const FString MatchType);
	UFUNCTION(BlueprintCallable, Category = MultiplayerSessions)
	void StartSession(FName SessionName);
	UFUNCTION(BlueprintCallable, Category = MultiplayerSessions)
	void FindSessions(ULocalPlayer* LocalPlayer, int32 MaxSearchResults);
	UFUNCTION(BlueprintCallable, Category = MultiplayerSessions)
	void JoinSessionByIndex(ULocalPlayer* LocalPlayer, int32 SessionIndex);
	UFUNCTION(BlueprintCallable, Category = MultiplayerSessions)
	void DestroySession(FName SessionName);

	//Native
	void JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& Session);

	//Session
	bool HasSessionInterface();
	TSharedPtr<FOnlineSessionSettings> GetSessionSettings();
	TSharedPtr<FOnlineSessionSearch> GetSessionSearch();
	FNamedOnlineSession* GetSession(FName SessionName = NAME_GameSession);
	bool IsInSession(FName SessionName = NAME_GameSession);

	UFUNCTION(BlueprintCallable, Category = MultiplayerSessions)
	bool GetConnectionString(const FName& SessionName, FString& ConnectionString);

	UFUNCTION(BlueprintCallable, Category = MultiplayerSessions)
	void ServerTravel(APlayerController* PlayerController, FString MapURL, bool bAbsolute = false, bool bListenServer = false);
	UFUNCTION(BlueprintCallable, Category = MultiplayerSessions)
	void ClientTravel(FName SessionName, APlayerController* PlayerController, ETravelType TravelType, bool bSeamless = false);

	//Publicly exposed callbacks
	FOnCreateSession OnCreateSession;
	FOnStartSession OnStartSession;
	FOnDestroySession OnDestroySession;
	FOnFindSessions OnFindSessions;
	FOnJoinSession OnJoinSession;

	//Const FNames
	static const FName NAME_MatchType;
};