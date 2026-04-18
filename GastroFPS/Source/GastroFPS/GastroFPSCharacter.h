// Bazowy first-person character dla GastroFPS.
// Do movementu/lookingu używa Enhanced Input (istniejące IA_Move/IA_Look/IA_Jump z templatu).
// Interakcja (E) używa legacy key binding żeby nie wymagać kolejnego uasset.

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

	// =================== Interakcja ===================
	// Zasięg trace'u dla interakcji
	UPROPERTY(EditAnywhere, Category = "GastroFPS|Interact")
	float InteractTraceCm = 250.f;

	// Czas życia bubble'a zamówienia po peekowaniu
	UPROPERTY(EditAnywhere, Category = "GastroFPS|Order")
	float PeekedOrderTTLSec = 5.f;

	UFUNCTION(BlueprintCallable, Category = "GastroFPS|Interact")
	AStation* GetCurrentInteractTarget() const { return CurrentInteractTarget; }

	// =================== Peek (zapamiętane zamówienie) ===================
	bool HasPeekedOrder() const { return bHasPeekedOrder && PeekedOrderRemainingSec > 0.f; }
	void PeekOrderFrom(ACustomer* Customer);
	FOrderData ConsumePeekedOrder();

	UFUNCTION(BlueprintPure, Category = "GastroFPS|Order")
	float GetPeekedOrderRemainingSec() const { return PeekedOrderRemainingSec; }

	UFUNCTION(BlueprintPure, Category = "GastroFPS|Order")
	const FOrderData& GetPeekedOrderForHUD() const { return PeekedOrder; }

	// =================== Carrying pizza ===================
	bool IsCarryingPizza() const { return bCarryingPizza; }
	void PickUpPizza(const FOrderData& Order);
	void ClearCarriedPizza();
	bool DoesCarriedOrderMatch(const FOrderData& CustomerOrder) const;

	UFUNCTION(BlueprintPure, Category = "GastroFPS|Order")
	const FOrderData& GetCarriedOrderForHUD() const { return CarriedOrder; }

protected:
	// Enhanced Input callbacks (unchanged)
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

	// Interact (legacy key binding na E)
	void OnInteractPressed();

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

private:
	// Aktualnie namierzona stacja (cached co tick dla HUD)
	UPROPERTY()
	AStation* CurrentInteractTarget = nullptr;

	// Peek state
	UPROPERTY()
	FOrderData PeekedOrder;
	bool bHasPeekedOrder = false;
	float PeekedOrderRemainingSec = 0.f;

	// Carry state
	UPROPERTY()
	FOrderData CarriedOrder;
	bool bCarryingPizza = false;

	// Helpers
	AStation* TraceForStation() const;
};
