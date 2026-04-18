#include "Customers/Customer.h"
#include "Components/StaticMeshComponent.h"
#include "Stations/TableStation.h"
#include "Core/GastroFPSGameState.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"

ACustomer::ACustomer()
{
	PrimaryActorTick.bCanEverTick = true;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	RootComponent = BodyMesh;
	BodyMesh->SetCollisionProfileName(TEXT("Pawn"));
	BodyMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.8f));

	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Head"));
	HeadMesh->SetupAttachment(BodyMesh);
	HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, 110.f));
	HeadMesh->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.4f));
	HeadMesh->SetCollisionProfileName(TEXT("NoCollision"));

	// Placeholder mesh — ustawi GameMode przy spawnie
	AIControllerClass = nullptr;
	AutoPossessAI = EAutoPossessAI::Disabled;
}

void ACustomer::AssignToTable(ATableStation* Table)
{
	if (!Table) return;
	AssignedTable = Table;
	Table->Occupant = this;
	TargetLocation = Table->SeatPoint->GetComponentLocation();
	SetState(ECustomerState::WalkingToTable);
}

void ACustomer::SetState(ECustomerState NewState)
{
	State = NewState;
	StateTimer = 0.f;
}

void ACustomer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	StateTimer += DeltaSeconds;

	switch (State)
	{
	case ECustomerState::WalkingToTable:
	{
		UpdateWalking(DeltaSeconds);
		break;
	}
	case ECustomerState::Seated_WaitingForServer:
	{
		if (StateTimer > MaxWaitForServerSec)
		{
			// Walkout
			if (AssignedTable.IsValid()) AssignedTable->Occupant.Reset();
			SetState(ECustomerState::Leaving);
		}
		break;
	}
	case ECustomerState::OrderTaken_WaitingForFood:
	{
		if (StateTimer > MaxWaitForFoodSec)
		{
			// Walkout z kary (zamówienie failed)
			CurrentOrder.State = EOrderState::Failed;
			if (AssignedTable.IsValid()) AssignedTable->Occupant.Reset();
			SetState(ECustomerState::Leaving);
		}
		break;
	}
	case ECustomerState::Eating:
	{
		if (StateTimer > EatingDurationSec)
		{
			SetState(ECustomerState::WaitingToPay);
		}
		break;
	}
	case ECustomerState::WaitingToPay:
	{
		if (StateTimer > PayingDurationSec)
		{
			AwardPayout(true);
			if (AssignedTable.IsValid()) AssignedTable->Occupant.Reset();
			SetState(ECustomerState::Leaving);
		}
		break;
	}
	case ECustomerState::Leaving:
	{
		// Odejdź i zniknij
		FVector Fwd = GetActorForwardVector() * -1.f;
		AddActorWorldOffset(Fwd * WalkSpeedCmPerSec * DeltaSeconds);
		if (StateTimer > 3.f)
		{
			SetState(ECustomerState::WalkedOut);
			Destroy();
		}
		break;
	}
	default: break;
	}
}

void ACustomer::UpdateWalking(float DeltaSeconds)
{
	const FVector Loc = GetActorLocation();
	const FVector Delta = TargetLocation - Loc;
	const float Dist = Delta.Size();
	if (Dist < 20.f)
	{
		SetActorLocation(TargetLocation);
		CurrentOrder.TimeSeated = GetWorld()->GetTimeSeconds();
		SetState(ECustomerState::Seated_WaitingForServer);
		return;
	}
	const FVector Dir = Delta / Dist;
	const float Step = FMath::Min(WalkSpeedCmPerSec * DeltaSeconds, Dist);
	AddActorWorldOffset(Dir * Step);
	SetActorRotation(Dir.Rotation());
}

void ACustomer::OnOrderPeeked()
{
	if (State == ECustomerState::Seated_WaitingForServer)
	{
		CurrentOrder.State = EOrderState::Peeked;
		// Zostajemy w tym samym stanie — klient nadal czeka na jedzenie.
		// Przejście do OrderTaken_WaitingForFood dopiero po Submit.
	}
}

void ACustomer::OnOrderSubmitted()
{
	if (State == ECustomerState::Seated_WaitingForServer)
	{
		CurrentOrder.State = EOrderState::SubmittedToKitchen;
		CurrentOrder.TimeSubmitted = GetWorld()->GetTimeSeconds();
		SetState(ECustomerState::OrderTaken_WaitingForFood);
	}
}

void ACustomer::OnFoodDelivered(bool bCorrectOrder)
{
	if (State != ECustomerState::OrderTaken_WaitingForFood) return;
	if (bCorrectOrder)
	{
		CurrentOrder.State = EOrderState::Delivered;
		CurrentOrder.TimeDelivered = GetWorld()->GetTimeSeconds();
		SetState(ECustomerState::Eating);
	}
	else
	{
		// Zła pizza — kara, klient wychodzi
		AwardPayout(false);
		if (AssignedTable.IsValid()) AssignedTable->Occupant.Reset();
		SetState(ECustomerState::Leaving);
	}
}

void ACustomer::AwardPayout(bool bCorrect)
{
	AGastroFPSGameState* GS = GetWorld()->GetGameState<AGastroFPSGameState>();
	if (!GS) return;
	if (bCorrect)
	{
		const int32 Base = CurrentOrder.GetBaseValue();
		// Proste liczenie tipu: <30s po submit = 30% tip, <60s = 15%, inaczej 0
		float ServiceTime = CurrentOrder.TimeDelivered - CurrentOrder.TimeSubmitted;
		float TipMul = 0.f;
		if (ServiceTime < 30.f) TipMul = 0.3f;
		else if (ServiceTime < 60.f) TipMul = 0.15f;
		int32 Total = Base + FMath::RoundToInt(Base * TipMul);
		GS->AddMoney(Total);
		GS->IncrementOrdersCompleted();
	}
	else
	{
		GS->AddMoney(-30);
		GS->IncrementOrdersFailed();
	}
}
