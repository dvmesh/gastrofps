// POS terminal. Gracz musi najpierw "peeknąć" zamówienie ze stolika, potem tutaj je potwierdzić.
// W MVP: E na POS → jeśli gracz ma peeked order i nic nie niesie → submit do kuchni.
// Uruchamia timer cook (3s), po timerze pizza pojawia się na najbliższym Pass.

#pragma once

#include "CoreMinimal.h"
#include "Stations/Station.h"
#include "POSStation.generated.h"

UCLASS()
class GASTROFPS_API APOSStation : public AStation
{
	GENERATED_BODY()

public:
	APOSStation();

	virtual bool CanInteract_Implementation(APawn* Interactor) const override;
	virtual void Interact_Implementation(APawn* Interactor) override;
	virtual FText GetPromptText_Implementation(APawn* Interactor) const override;

	// Czas gotowania (sekundy). MVP: 3s niezależnie od pizzy.
	UPROPERTY(EditAnywhere, Category = "GastroFPS|POS")
	float CookTimeSeconds = 3.0f;
};
