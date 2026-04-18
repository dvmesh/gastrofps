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

	AIControllerClass = nullptr;
	AutoPossessAI = EAutoPossessAI::Disabled;
}

void ACustomer::AssignToTable(ATableStation* Table)
{
	if (!Table) return;
	AssignedTable = Table;
	Table->Occupant = this;
	Table->State = ETableState::Occupied;
	CurrentOrder.TableNumber = Table->TableNumber;
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
			// Walkout bez obsługi — stolik wraca do Free (nic nie jedli, nie ma mess)
			if (AssignedTable.IsValid())
			{
				AssignedTable->State = ETableState::Free;
				AssignedTable->Occupant.Reset();
			}
			AGastroFPSGameState* GS = GetWorld()->GetGameState<AGastroFPSGameState>();
			if (GS) GS->IncrementOrdersFailed();
			SetState(ECustomerState::Leaving);
		}
		break;
	}
	case ECustomerState::OrderTaken_WaitingForFood:
	{
		if (StateTimer > MaxWaitForFoodSec)
		{
			// Walkout po zbyt długim oczekiwaniu na jedzenie
			CurrentOrder.State = EOrderState::Failed;
			if (AssignedTable.IsValid())
			{
				AssignedTable->State = ETableState::Free;
				AssignedTable->Occupant.Reset();
			}
			AGastroFPSGameState* GS = GetWorld()->GetGameState<AGastroFPSGameState>();
			if (GS) GS->IncrementOrdersFailed();
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
			// Zostaw kasę na stole — player musi ją zebrać
			const int32 Amount = ComputePayoutAmount();
			if (AssignedTable.IsValid())
			{
				AssignedTable->MarkPaidAndLeft(Amount);
			}
			AGastroFPSGameState* GS = GetWorld()->GetGameState<AGastroFPSGameState>();
			if (GS) GS->IncrementOrdersCompleted();
			SetState(ECustomerState::Leaving);
		}
		break;
	}
	case ECustomerState::Leaving:
	{
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
	// Błędna dostawa: nie zmieniamy stanu — klient dalej czeka. Gracz re-cook.
	// Time penalty (niższy tip) pojawia się naturalnie, bo service time rośnie.
}

int32 ACustomer::ComputePayoutAmount() const
{
	const int32 Base = CurrentOrder.GetBaseValue();
	float ServiceTime = CurrentOrder.TimeDelivered - CurrentOrder.TimeSubmitted;
	float TipMul = 0.f;
	if (ServiceTime < 30.f) TipMul = 0.3f;
	else if (ServiceTime < 60.f) TipMul = 0.15f;
	return Base + FMath::RoundToInt(Base * TipMul);
}
