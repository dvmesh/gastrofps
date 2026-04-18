// Proceduralnie stawia whitebox pizzerii w StartPlay:
// - destroy wszystkie AStaticMeshActor z Lvl_FirstPerson (czyści shooter targety, ściany, floor templatu)
// - spawn własnej podłogi i stacji na wyliczonym poziomie (trace down z PlayerStart)
// - spawnuje 3 stoliki (T1-T3), 1 POS, 1 Pass
// - loop spawn customerów do wolnych stolików co X sekund

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
	float CustomerSpawnIntervalSec = 15.f;

	UPROPERTY(EditAnywhere, Category = "GastroFPS|Match")
	int32 NumTables = 3;

	UPROPERTY(EditAnywhere, Category = "GastroFPS|Match")
	bool bCleanLevelAtStart = true;

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

	UPROPERTY()
	UMaterialInterface* GrayMaterial = nullptr;

private:
	void CleanLevel();
	void SpawnWorld(const FVector& Origin, float FloorZ);
	void TrySpawnCustomer();
	ATableStation* FindFreeTable() const;
	float FindFloorZ(const FVector& Origin) const;

	TArray<TWeakObjectPtr<ATableStation>> Tables;
	FVector CustomerSpawnPoint = FVector::ZeroVector;

	float NextCustomerSpawnTimer = 0.f;
	bool bWorldSpawned = false;
	int32 NextOrderId = 1;
};
