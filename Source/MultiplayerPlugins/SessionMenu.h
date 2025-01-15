// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "SessionMenu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPLUGINS_API USessionMenu : public UUserWidget
{
	GENERATED_BODY()

private:

	FDelegateHandle OnCreateSessionDelegateHandle;
	FDelegateHandle OnStartSessionDelegateHandle;
	FDelegateHandle OnFindSessionsDelegateHandle;
	FDelegateHandle OnJoinSessionDelegateHandle;
	FDelegateHandle OnDestroySessionDelegateHandle;

	void SubscribeToSessionDelegates();
	void UnsubscribeFromSessionDelegates();

	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ClientButton;
	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;
	UPROPERTY(meta = (BindWidget))
	UButton* HideButton;
	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;

	UFUNCTION()
	void OnHostClicked();
	UFUNCTION()
	void OnClientClicked();
	UFUNCTION()
	void OnCloseClicked();
	UFUNCTION()
	void OnHideClicked();
	UFUNCTION()
	void OnQuitClicked();

protected:

	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	virtual void NativeDestruct() override;

	//Callbacks
	void OnCreateSession(bool bWasSuccessful);
	void OnStartSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySession(bool bWasSuccessful);

public:

	virtual bool Initialize() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = MultiplayerSessionWidget)
	void Show(bool InIsFocusable);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = MultiplayerSessionWidget)
	void Hide();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = MultiplayerSessionWidget)
	void Close();
	
};