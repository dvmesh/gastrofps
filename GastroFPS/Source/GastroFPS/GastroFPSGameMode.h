// Proceduralnie stawia whitebox pizzerii w StartPlay, spawnuje klientów co X sekund.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GastroFPSGameMode.generated.h"

class ATableStation;
class APOSStation;
class APassStation;
class ACustomer;
class UStaticMesh;
class UMaterialInterface;

UCLASS()
class AGastroFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGastroFPSGameMode();

	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "GastroFPS|Match")
	float MatchDurationSec = 15 * 60.f;

	UPROPERTY(EditAnywhere, Category = "GastroFPS|Match")
	float CustomerSpawnIntervalSec = 20.f;

	UPROPERTY()
	UStaticMesh* CubeMesh = nullptr;

	UPROPERTY()
	UMaterialInterface* RedMaterial = nullptr;

	UPROPERTY()
	UMaterialInterface* BlueMaterial = nullptr;

	UPROPERTY()
	UMaterialInterface* GreenMaterial = nullptr;

	UPROPERTY()
	UMaterialInterface* YellowMaterial = nullptr;

private:
	void SpawnWorld(const FVector& Origin);
	void TrySpawnCustomer();
	ATableStation* FindFreeTable() const;

	TArray<TWeakObjectPtr<ATableStation>> Tables;
	FVector CustomerSpawnPoint = FVector::ZeroVector;

	float NextCustomerSpawnTimer = 0.f;
	bool bWorldSpawned = false;
	int32 NextOrderId = 1;
};
