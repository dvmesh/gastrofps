// Stolik. Ma slot na jednego klienta. Interakcja zależy od stanu:
// - pusty: nic się nie dzieje
// - klient siedzi (NotTaken): "take order" → peek, serwer zapamiętuje
// - klient siedzi (waiting for food) + gracz niesie pizzę: "deliver order"

#pragma once

#include "CoreMinimal.h"
#include "Stations/Station.h"
#include "Core/GastroTypes.h"
#include "TableStation.generated.h"

class ACustomer;

UCLASS()
class GASTROFPS_API ATableStation : public AStation
{
	GENERATED_BODY()

public:
	ATableStation();

	virtual bool CanInteract_Implementation(APawn* Interactor) const override;
	virtual void Interact_Implementation(APawn* Interactor) override;
	virtual FText GetPromptText_Implementation(APawn* Interactor) const override;

	// Slot na klienta — 1 na stolik w MVP
	UPROPERTY()
	TWeakObjectPtr<ACustomer> Occupant;

	// Punkt w świecie gdzie klient ma usiąść (offset od stolika)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* SeatPoint;
};
