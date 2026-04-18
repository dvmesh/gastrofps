// Klient NPC — AI pawn. Prosty state machine dla MVP, bez navmesh (idzie prosto do SeatPoint).
// Dla M1 single-player wystarczy; w M2+ podmiana na AIController+NavMesh.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Core/GastroTypes.h"
#include "Customer.generated.h"

class UStaticMeshComponent;
class ATableStation;

UCLASS()
class GASTROFPS_API ACustomer : public APawn
{
	GENERATED_BODY()

public:
	ACustomer();

	virtual void Tick(float DeltaSeconds) override;

	// Config
	UPROPERTY(EditAnywhere, Category = "GastroFPS|Customer")
	float WalkSpeedCmPerSec = 200.f;

	UPROPERTY(EditAnywhere, Category = "GastroFPS|Customer")
	float MaxWaitForServerSec = 45.f;

	UPROPERTY(EditAnywhere, Category = "GastroFPS|Customer")
	float MaxWaitForFoodSec = 90.f;

	UPROPERTY(EditAnywhere, Category = "GastroFPS|Customer")
	float EatingDurationSec = 4.f;

	UPROPERTY(EditAnywhere, Category = "GastroFPS|Customer")
	float PayingDurationSec = 1.f;

	// Runtime
	UPROPERTY(BlueprintReadOnly, Category = "GastroFPS|Customer")
	ECustomerState State = ECustomerState::Spawning;

	UPROPERTY(BlueprintReadOnly, Category = "GastroFPS|Customer")
	FOrderData CurrentOrder;

	UPROPERTY()
	TWeakObjectPtr<ATableStation> AssignedTable;

	// Wywoływane gdy stolik ma slot
	void AssignToTable(ATableStation* Table);

	// Callbacks od stacji
	void OnOrderPeeked();
	void OnOrderSubmitted();
	void OnFoodDelivered(bool bCorrectOrder);

	// Wypłata (base + tip) — wywoływana przy MarkPaidAndLeft na stoliku
	int32 ComputePayoutAmount() const;

	// Składowe
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* BodyMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* HeadMesh;

private:
	float StateTimer = 0.f;
	FVector TargetLocation = FVector::ZeroVector;

	void SetState(ECustomerState NewState);
	void UpdateWalking(float DeltaSeconds);
};
