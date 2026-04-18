// Bazowa klasa dla wszystkich interaktywnych stanowisk (stolik, POS, pass, prep, oven)
// Gracz celuje kamerą → trace z pierwszej osoby → jeśli trafi stację i jest w zasięgu,
// w HUD pojawia się prompt "[E] Interakt". Naciśnięcie E → CanInteract() → Interact()

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Station.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS(Abstract)
class GASTROFPS_API AStation : public AActor
{
	GENERATED_BODY()

public:
	AStation();

	// Zwraca true jeśli dany gracz może w tej chwili wejść w interakcję
	// (np. POS: tylko Server role; Pass: tylko z gotowym daniem w ręce).
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GastroFPS|Station")
	bool CanInteract(APawn* Interactor) const;
	virtual bool CanInteract_Implementation(APawn* Interactor) const;

	// Wykonaj interakcję (E). Server-side logic.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GastroFPS|Station")
	void Interact(APawn* Interactor);
	virtual void Interact_Implementation(APawn* Interactor);

	// Tekst promptu pokazywany w HUD ("Weź zamówienie", "Otwórz POS", "Pick up pizza")
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GastroFPS|Station")
	FText GetPromptText(APawn* Interactor) const;
	virtual FText GetPromptText_Implementation(APawn* Interactor) const;

	// Zasięg interakcji w cm
	UPROPERTY(EditAnywhere, Category = "GastroFPS|Station")
	float InteractReachCm = 200.f;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Mesh;
};
