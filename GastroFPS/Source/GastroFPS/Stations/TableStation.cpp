#include "Stations/TableStation.h"
#include "Customers/Customer.h"
#include "GastroFPSCharacter.h"

ATableStation::ATableStation()
{
	SeatPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SeatPoint"));
	SeatPoint->SetupAttachment(Root);
	SeatPoint->SetRelativeLocation(FVector(-80.f, 0.f, 0.f)); // 80cm przed stolikiem
}

bool ATableStation::CanInteract_Implementation(APawn* Interactor) const
{
	if (!Occupant.IsValid()) return false;
	AGastroFPSCharacter* Char = Cast<AGastroFPSCharacter>(Interactor);
	if (!Char) return false;

	const ECustomerState CS = Occupant->State;
	if (CS == ECustomerState::Seated_WaitingForServer)
	{
		return true; // take order
	}
	if (CS == ECustomerState::OrderTaken_WaitingForFood && Char->IsCarryingPizza())
	{
		return true; // deliver
	}
	return false;
}

void ATableStation::Interact_Implementation(APawn* Interactor)
{
	if (!Occupant.IsValid()) return;
	AGastroFPSCharacter* Char = Cast<AGastroFPSCharacter>(Interactor);
	if (!Char) return;

	const ECustomerState CS = Occupant->State;
	if (CS == ECustomerState::Seated_WaitingForServer)
	{
		// Peek zamówienia — gracz "widzi" co klient chce
		Char->PeekOrderFrom(Occupant.Get());
		Occupant->OnOrderPeeked();
	}
	else if (CS == ECustomerState::OrderTaken_WaitingForFood && Char->IsCarryingPizza())
	{
		// Dostarczenie — jeżeli pizza się zgadza z zamówieniem, sukces
		const bool bCorrect = Char->DoesCarriedOrderMatch(Occupant->CurrentOrder);
		Occupant->OnFoodDelivered(bCorrect);
		Char->ClearCarriedPizza();
	}
}

FText ATableStation::GetPromptText_Implementation(APawn* Interactor) const
{
	if (!Occupant.IsValid()) return FText::GetEmpty();
	AGastroFPSCharacter* Char = Cast<AGastroFPSCharacter>(Interactor);
	if (!Char) return FText::GetEmpty();

	if (Occupant->State == ECustomerState::Seated_WaitingForServer)
	{
		return FText::FromString(TEXT("[E] Weź zamówienie"));
	}
	if (Occupant->State == ECustomerState::OrderTaken_WaitingForFood && Char->IsCarryingPizza())
	{
		return FText::FromString(TEXT("[E] Podaj pizzę"));
	}
	return FText::GetEmpty();
}
