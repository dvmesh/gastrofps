#include "Stations/TableStation.h"
#include "Customers/Customer.h"
#include "GastroFPSCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ATableStation::ATableStation()
{
	SeatPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SeatPoint"));
	SeatPoint->SetupAttachment(Root);
	SeatPoint->SetRelativeLocation(FVector(-120.f, 0.f, 0.f)); // 120cm przed stolikiem

	// Cash visual — mały zielony sześcian na blacie, widoczny tylko w Dirty_WithCash
	CashVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CashVisual"));
	CashVisual->SetupAttachment(Root);
	CashVisual->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	CashVisual->SetRelativeScale3D(FVector(0.25f, 0.4f, 0.08f));
	CashVisual->SetCollisionProfileName(TEXT("NoCollision"));
	CashVisual->SetVisibility(false);

	// Dirty visual — czerwony "brudny talerz" placeholder, widoczny w Dirty_* stanach
	DirtyVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DirtyVisual"));
	DirtyVisual->SetupAttachment(Root);
	DirtyVisual->SetRelativeLocation(FVector(15.f, 0.f, 65.f));
	DirtyVisual->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.05f));
	DirtyVisual->SetCollisionProfileName(TEXT("NoCollision"));
	DirtyVisual->SetVisibility(false);
}

bool ATableStation::CanInteract_Implementation(APawn* Interactor) const
{
	AGastroFPSCharacter* Char = Cast<AGastroFPSCharacter>(Interactor);
	if (!Char) return false;

	switch (State)
	{
	case ETableState::Occupied:
	{
		if (!Occupant.IsValid()) return false;
		const ECustomerState CS = Occupant->State;
		if (CS == ECustomerState::Seated_WaitingForServer) return true; // peek
		if (CS == ECustomerState::OrderTaken_WaitingForFood && Char->IsCarryingPizza()) return true; // deliver
		return false;
	}
	case ETableState::Dirty_WithCash:  return true;  // pick up cash
	case ETableState::Dirty_NoCash:    return true;  // clean
	case ETableState::Free:
	default:                            return false;
	}
}

void ATableStation::Interact_Implementation(APawn* Interactor)
{
	AGastroFPSCharacter* Char = Cast<AGastroFPSCharacter>(Interactor);
	if (!Char) return;

	switch (State)
	{
	case ETableState::Occupied:
	{
		if (!Occupant.IsValid()) return;
		const ECustomerState CS = Occupant->State;
		if (CS == ECustomerState::Seated_WaitingForServer)
		{
			// Peek zamówienia — dodajemy do pamięci gracza (bez limitu czasu)
			Char->AddPeekedOrder(Occupant->CurrentOrder);
			Occupant->OnOrderPeeked();
		}
		else if (CS == ECustomerState::OrderTaken_WaitingForFood && Char->IsCarryingPizza())
		{
			const bool bCorrect = Char->DoesCarriedOrderMatch(Occupant->CurrentOrder);
			if (bCorrect)
			{
				Occupant->OnFoodDelivered(true);
			}
			else
			{
				// BŁĘDNA DOSTAWA: marnujemy próbę, klient nadal czeka (daj szansę naprawy).
				// Tip się zjedzie bo service time rośnie. Ale brak hard-fail.
				Occupant->OnFoodDelivered(false);
			}
			Char->ClearCarriedPizza();
		}
		break;
	}
	case ETableState::Dirty_WithCash:
	{
		const int32 Pocketed = CollectCash();
		Char->PocketCash(Pocketed);
		// Stan przechodzi do Dirty_NoCash — trzeba jeszcze posprzątać
		break;
	}
	case ETableState::Dirty_NoCash:
	{
		CleanUp();
		break;
	}
	default: break;
	}
}

FText ATableStation::GetPromptText_Implementation(APawn* Interactor) const
{
	AGastroFPSCharacter* Char = Cast<AGastroFPSCharacter>(Interactor);
	if (!Char) return FText::GetEmpty();

	switch (State)
	{
	case ETableState::Occupied:
	{
		if (!Occupant.IsValid()) return FText::GetEmpty();
		const ECustomerState CS = Occupant->State;
		if (CS == ECustomerState::Seated_WaitingForServer)
		{
			return FText::Format(FText::FromString(TEXT("[E] Weź zamówienie (T{0})")),
				FText::AsNumber(TableNumber));
		}
		if (CS == ECustomerState::OrderTaken_WaitingForFood && Char->IsCarryingPizza())
		{
			return FText::Format(FText::FromString(TEXT("[E] Podaj pizzę (T{0})")),
				FText::AsNumber(TableNumber));
		}
		return FText::GetEmpty();
	}
	case ETableState::Dirty_WithCash:
		return FText::Format(FText::FromString(TEXT("[E] Zbierz kasę: ${0}")),
			FText::AsNumber(CashAmount));
	case ETableState::Dirty_NoCash:
		return FText::FromString(TEXT("[E] Posprzątaj stolik"));
	default:
		return FText::GetEmpty();
	}
}

void ATableStation::MarkPaidAndLeft(int32 Amount)
{
	CashAmount = Amount;
	State = ETableState::Dirty_WithCash;
	Occupant.Reset();
	if (CashVisual) CashVisual->SetVisibility(true);
	if (DirtyVisual) DirtyVisual->SetVisibility(true);
}

int32 ATableStation::CollectCash()
{
	int32 Out = CashAmount;
	CashAmount = 0;
	State = ETableState::Dirty_NoCash;
	if (CashVisual) CashVisual->SetVisibility(false);
	return Out;
}

void ATableStation::CleanUp()
{
	State = ETableState::Free;
	if (DirtyVisual) DirtyVisual->SetVisibility(false);
}
