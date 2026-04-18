// Bazowy first-person character dla GastroFPS.
// Używa Enhanced Input dla move/look/jump i legacy key binding dla E (interact).
// Peek zamówień: gracz pamięta DOWOLNĄ ilość zamówień (jak kelner z blokiem) — brak TTL.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Core/GastroTypes.h"
#include "GastroFPSCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class ACustomer;
class AStation;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(abstract)
class AGastroFPSCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

protected:
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;

public:
	AGastroFPSCharacter();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "GastroFPS|Interact")
	float InteractTraceCm = 250.f;

	UFUNCTION(BlueprintCallable, Category = "GastroFPS|Interact")
	AStation* GetCurrentInteractTarget() const { return CurrentInteractTarget; }

	// =================== Peek queue (kelner z pamięcią) ===================
	bool HasPeekedOrders() const { return PeekedOrders.Num() > 0; }
	int32 NumPeekedOrders() const { return PeekedOrders.Num(); }

	// Dodaje zamówienie do queue jeżeli jeszcze tam nie ma (po OrderId/CustomerRef)
	void AddPeekedOrder(const FOrderData& Order);

	// Pobiera najstarsze (FIFO) zamówienie z queue — używa POS
	FOrderData ConsumeOldestPeekedOrder();

	const TArray<FOrderData>& GetPeekedOrdersForHUD() const { return PeekedOrders; }

	// =================== Carrying pizza ===================
	bool IsCarryingPizza() const { return bCarryingPizza; }
	void PickUpPizza(const FOrderData& Order);
	void ClearCarriedPizza();
	bool DoesCarriedOrderMatch(const FOrderData& CustomerOrder) const;

	UFUNCTION(BlueprintPure, Category = "GastroFPS|Order")
	const FOrderData& GetCarriedOrderForHUD() const { return CarriedOrder; }

	// =================== Cash pickup ===================
	void PocketCash(int32 Amount);

protected:
	void MoveInput(const FInputActionValue& Value);
	void LookInput(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	void OnInteractPressed();

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

private:
	UPROPERTY()
	AStation* CurrentInteractTarget = nullptr;

	// Queue peeków — bez limitu czasu (gracz pamięta sam jak kelner z blokiem)
	UPROPERTY()
	TArray<FOrderData> PeekedOrders;

	UPROPERTY()
	FOrderData CarriedOrder;
	bool bCarryingPizza = false;

	AStation* TraceForStation() const;
};
