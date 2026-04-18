// Pass window — miejsce gdzie pojawia się gotowe jedzenie. Gracz E żeby wziąć.
// W MVP trzyma kolejkę FOrderData — bierze się najstarsze.

#pragma once

#include "CoreMinimal.h"
#include "Stations/Station.h"
#include "Core/GastroTypes.h"
#include "PassStation.generated.h"

UCLASS()
class GASTROFPS_API APassStation : public AStation
{
	GENERATED_BODY()

public:
	APassStation();

	virtual bool CanInteract_Implementation(APawn* Interactor) const override;
	virtual void Interact_Implementation(APawn* Interactor) override;
	virtual FText GetPromptText_Implementation(APawn* Interactor) const override;

	// Wywoływane przez POSStation po zakończeniu timera cook
	void OnFoodReady(const FOrderData& Order);

	// Kolejka gotowych zamówień oczekujących na odbiór
	UPROPERTY()
	TArray<FOrderData> ReadyQueue;
};
