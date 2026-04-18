#include "Stations/PassStation.h"
#include "GastroFPSCharacter.h"

APassStation::APassStation()
{
}

bool APassStation::CanInteract_Implementation(APawn* Interactor) const
{
	if (ReadyQueue.Num() == 0) return false;
	AGastroFPSCharacter* Char = Cast<AGastroFPSCharacter>(Interactor);
	if (!Char) return false;
	if (Char->IsCarryingPizza()) return false;
	return true;
}

void APassStation::Interact_Implementation(APawn* Interactor)
{
	if (ReadyQueue.Num() == 0) return;
	AGastroFPSCharacter* Char = Cast<AGastroFPSCharacter>(Interactor);
	if (!Char || Char->IsCarryingPizza()) return;

	FOrderData Order = ReadyQueue[0];
	ReadyQueue.RemoveAt(0);
	Char->PickUpPizza(Order);
}

FText APassStation::GetPromptText_Implementation(APawn* Interactor) const
{
	AGastroFPSCharacter* Char = Cast<AGastroFPSCharacter>(Interactor);
	if (!Char || Char->IsCarryingPizza()) return FText::GetEmpty();
	if (ReadyQueue.Num() > 0)
	{
		return FText::Format(
			FText::FromString(TEXT("[E] Odbierz pizzę ({0} w kolejce)")),
			FText::AsNumber(ReadyQueue.Num())
		);
	}
	return FText::GetEmpty();
}

void APassStation::OnFoodReady(const FOrderData& Order)
{
	ReadyQueue.Add(Order);
}
