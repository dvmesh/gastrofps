// Stan meczu — pieniądze, counters zamówień, pozostały czas. W M1 single-player.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GastroFPSGameState.generated.h"

UCLASS()
class GASTROFPS_API AGastroFPSGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AGastroFPSGameState();

	UPROPERTY(BlueprintReadOnly, Category = "GastroFPS|Match")
	int32 Money = 0;

	UPROPERTY(BlueprintReadOnly, Category = "GastroFPS|Match")
	int32 OrdersCompleted = 0;

	UPROPERTY(BlueprintReadOnly, Category = "GastroFPS|Match")
	int32 OrdersFailed = 0;

	// Pozostały czas w sekundach
	UPROPERTY(BlueprintReadOnly, Category = "GastroFPS|Match")
	float TimeLeftSec = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "GastroFPS|Match")
	bool bMatchActive = false;

	void AddMoney(int32 Delta);
	void IncrementOrdersCompleted();
	void IncrementOrdersFailed();

	void StartMatch(float DurationSec);
	void EndMatch();

	virtual void Tick(float DeltaSeconds) override;
};
