// Typy wspólne dla GastroFPS — enum rang, stany zamówień, pozycje menu

#pragma once

#include "CoreMinimal.h"
#include "GastroTypes.generated.h"

UENUM(BlueprintType)
enum class EPizzaType : uint8
{
	Margherita,
	Pepperoni,
	QuattroFormaggi,
	Funghi,
	Diavola,
	Capricciosa,
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EPizzaSize : uint8
{
	Small,
	Medium,
	Large
};

UENUM(BlueprintType)
enum class EOrderState : uint8
{
	// Piktogram jeszcze nie pokazany (klient usiadł ale server nie podszedł)
	NotTaken,
	// Server zobaczył zamówienie (timer bubble aktywny)
	Peeked,
	// Zamówienie wbite do POS, czeka na wykonanie
	SubmittedToKitchen,
	// Jedzenie gotowe na pass
	Ready,
	// Jedzenie podane klientowi, klient je
	Delivered,
	// Zamówienie zakończone (klient zapłacił i wyszedł)
	Completed,
	// Klient wyszedł w złości (walkout)
	Failed
};

UENUM(BlueprintType)
enum class ECustomerState : uint8
{
	Spawning,
	WalkingToTable,
	Seated_WaitingForServer,
	OrderTaken_WaitingForFood,
	Eating,
	WaitingToPay,
	Leaving,
	WalkedOut
};

USTRUCT(BlueprintType)
struct FOrderItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPizzaType Pizza = EPizzaType::Margherita;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPizzaSize Size = EPizzaSize::Medium;
};

USTRUCT(BlueprintType)
struct FOrderData
{
	GENERATED_BODY()

	// Unikalne ID zamówienia w ramach meczu
	UPROPERTY(BlueprintReadOnly)
	int32 OrderId = -1;

	// Referencja do stolika/klienta
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> CustomerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOrderItem> Items;

	UPROPERTY(BlueprintReadOnly)
	EOrderState State = EOrderState::NotTaken;

	// Znacznik czasu kiedy klient usiadł (do liczenia tipu)
	UPROPERTY(BlueprintReadOnly)
	float TimeSeated = 0.f;

	// Kiedy zamówienie trafiło do kuchni (POS submit)
	UPROPERTY(BlueprintReadOnly)
	float TimeSubmitted = -1.f;

	// Kiedy dostarczono
	UPROPERTY(BlueprintReadOnly)
	float TimeDelivered = -1.f;

	// Bazowa wartość zamówienia w $
	int32 GetBaseValue() const
	{
		return Items.Num() * 15;
	}
};
