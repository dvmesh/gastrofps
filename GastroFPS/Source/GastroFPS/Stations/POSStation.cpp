#include "Stations/POSStation.h"
#include "Stations/PassStation.h"
#include "GastroFPSCharacter.h"
#include "Customers/Customer.h"
#include "EngineUtils.h"
#include "TimerManager.h"

APOSStation::APOSStation()
{
}

bool APOSStation::CanInteract_Implementation(APawn* Interactor) const
{
	AGastroFPSCharacter* Char = Cast<AGastroFPSCharacter>(Interactor);
	if (!Char) return false;
	if (Char->IsCarryingPizza()) return false;
	return Char->HasPeekedOrder();
}

void APOSStation::Interact_Implementation(APawn* Interactor)
{
	AGastroFPSCharacter* Char = Cast<AGastroFPSCharacter>(Interactor);
	if (!Char || !Char->HasPeekedOrder()) return;

	FOrderData Order = Char->ConsumePeekedOrder();

	// Oznacz zamówienie jako submitted (synchronizacja z klientem)
	if (Order.CustomerRef.IsValid())
	{
		if (ACustomer* Cust = Cast<ACustomer>(Order.CustomerRef.Get()))
		{
			Cust->OnOrderSubmitted();
		}
	}

	// Znajdź najbliższy pass (w MVP jest tylko jeden)
	APassStation* TargetPass = nullptr;
	float BestDist = FLT_MAX;
	for (TActorIterator<APassStation> It(GetWorld()); It; ++It)
	{
		float D = FVector::DistSquared(It->GetActorLocation(), GetActorLocation());
		if (D < BestDist) { BestDist = D; TargetPass = *It; }
	}

	// Timer cook
	FTimerHandle Th;
	TWeakObjectPtr<APassStation> PassWeak = TargetPass;
	FOrderData OrderCopy = Order;
	GetWorld()->GetTimerManager().SetTimer(Th, [PassWeak, OrderCopy]()
	{
		if (PassWeak.IsValid())
		{
			PassWeak->OnFoodReady(OrderCopy);
		}
	}, CookTimeSeconds, false);
}

FText APOSStation::GetPromptText_Implementation(APawn* Interactor) const
{
	AGastroFPSCharacter* Char = Cast<AGastroFPSCharacter>(Interactor);
	if (!Char) return FText::GetEmpty();
	if (Char->HasPeekedOrder() && !Char->IsCarryingPizza())
	{
		return FText::FromString(TEXT("[E] Wbij zamówienie"));
	}
	return FText::GetEmpty();
}
