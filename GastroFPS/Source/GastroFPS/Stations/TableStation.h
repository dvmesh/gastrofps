// Stolik. Stany: Free / Occupied / Dirty_WithCash / Dirty_NoCash.
// Gracz pamiętą numer stolika i co klient zamówił — peek nie ma timera, kelner jak IRL.

#pragma once

#include "CoreMinimal.h"
#include "Stations/Station.h"
#include "Core/GastroTypes.h"
#include "TableStation.generated.h"

class ACustomer;
class UStaticMeshComponent;

UCLASS()
class GASTROFPS_API ATableStation : public AStation
{
	GENERATED_BODY()

public:
	ATableStation();

	virtual bool CanInteract_Implementation(APawn* Interactor) const override;
	virtual void Interact_Implementation(APawn* Interactor) override;
	virtual FText GetPromptText_Implementation(APawn* Interactor) const override;

	// Numer stolika nadawany przez GameMode (1, 2, 3...)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GastroFPS|Table")
	int32 TableNumber = 1;

	// Klient siedzący (null jeśli Free/Dirty)
	UPROPERTY()
	TWeakObjectPtr<ACustomer> Occupant;

	// Stan stolika
	UPROPERTY(BlueprintReadOnly, Category = "GastroFPS|Table")
	ETableState State = ETableState::Free;

	// Kasa do pobrania (tylko w Dirty_WithCash)
	UPROPERTY(BlueprintReadOnly, Category = "GastroFPS|Table")
	int32 CashAmount = 0;

	// Punkt gdzie klient ma usiąść (offset od stolika)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* SeatPoint;

	// Wizualne markery
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* CashVisual;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* DirtyVisual;

	// API używane przez Customer
	void MarkPaidAndLeft(int32 Amount);

	// API używane przez player (zbiera kasę)
	int32 CollectCash();

	// API player — kliknięcie E żeby posprzątać (tylko Dirty_NoCash)
	void CleanUp();
};
