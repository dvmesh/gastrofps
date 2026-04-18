#include "GastroFPSGameMode.h"
#include "Core/GastroFPSGameState.h"
#include "Core/GastroTypes.h"
#include "Stations/TableStation.h"
#include "Stations/POSStation.h"
#include "Stations/PassStation.h"
#include "Customers/Customer.h"
#include "UI/GastroHUD.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

AGastroFPSGameMode::AGastroFPSGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	GameStateClass = AGastroFPSGameState::StaticClass();
	HUDClass = AGastroHUD::StaticClass();

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Game/LevelPrototyping/Meshes/SM_Cube"));
	if (CubeFinder.Succeeded())
	{
		CubeMesh = CubeFinder.Object;
	}

	// Mapowanie na istniejące materiały z LevelPrototyping/Materials
	// Red = DefaultColorway (klienci), Blue = TopDark (POS), Green = Gray_02 (pass), Yellow = Gray_Round (stolik)
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> RedFinder(
		TEXT("/Game/LevelPrototyping/Materials/MI_DefaultColorway"));
	if (RedFinder.Succeeded()) RedMaterial = RedFinder.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BlueFinder(
		TEXT("/Game/LevelPrototyping/Materials/MI_PrototypeGrid_TopDark"));
	if (BlueFinder.Succeeded()) BlueMaterial = BlueFinder.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GreenFinder(
		TEXT("/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray_02"));
	if (GreenFinder.Succeeded()) GreenMaterial = GreenFinder.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> YellowFinder(
		TEXT("/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray_Round"));
	if (YellowFinder.Succeeded()) YellowMaterial = YellowFinder.Object;
}

void AGastroFPSGameMode::StartPlay()
{
	Super::StartPlay();

	FVector Origin = FVector::ZeroVector;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		Origin = It->GetActorLocation();
		break;
	}

	SpawnWorld(Origin);
	bWorldSpawned = true;

	if (AGastroFPSGameState* GS = GetWorld()->GetGameState<AGastroFPSGameState>())
	{
		GS->StartMatch(MatchDurationSec);
	}
}

void AGastroFPSGameMode::SpawnWorld(const FVector& Origin)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Stolik
	FVector TableLoc = Origin + FVector(400.f, -200.f, 0.f);
	if (ATableStation* Table = World->SpawnActor<ATableStation>(ATableStation::StaticClass(), TableLoc, FRotator::ZeroRotator, Params))
	{
		if (CubeMesh) Table->Mesh->SetStaticMesh(CubeMesh);
		Table->Mesh->SetRelativeScale3D(FVector(1.2f, 1.2f, 0.8f));
		if (YellowMaterial) Table->Mesh->SetMaterial(0, YellowMaterial);
		Tables.Add(Table);
	}

	// POS
	FVector POSLoc = Origin + FVector(400.f, 200.f, 0.f);
	if (APOSStation* POS = World->SpawnActor<APOSStation>(APOSStation::StaticClass(), POSLoc, FRotator::ZeroRotator, Params))
	{
		if (CubeMesh) POS->Mesh->SetStaticMesh(CubeMesh);
		POS->Mesh->SetRelativeScale3D(FVector(0.6f, 1.0f, 1.0f));
		if (BlueMaterial) POS->Mesh->SetMaterial(0, BlueMaterial);
	}

	// Pass
	FVector PassLoc = Origin + FVector(600.f, 200.f, 0.f);
	if (APassStation* Pass = World->SpawnActor<APassStation>(APassStation::StaticClass(), PassLoc, FRotator::ZeroRotator, Params))
	{
		if (CubeMesh) Pass->Mesh->SetStaticMesh(CubeMesh);
		Pass->Mesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.5f));
		if (GreenMaterial) Pass->Mesh->SetMaterial(0, GreenMaterial);
	}

	CustomerSpawnPoint = Origin + FVector(-400.f, -400.f, 0.f);
}

void AGastroFPSGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bWorldSpawned) return;

	AGastroFPSGameState* GS = GetWorld()->GetGameState<AGastroFPSGameState>();
	if (!GS || !GS->bMatchActive) return;

	NextCustomerSpawnTimer -= DeltaSeconds;
	if (NextCustomerSpawnTimer <= 0.f)
	{
		TrySpawnCustomer();
		NextCustomerSpawnTimer = CustomerSpawnIntervalSec;
	}
}

ATableStation* AGastroFPSGameMode::FindFreeTable() const
{
	for (const TWeakObjectPtr<ATableStation>& W : Tables)
	{
		if (W.IsValid() && !W->Occupant.IsValid())
		{
			return W.Get();
		}
	}
	return nullptr;
}

void AGastroFPSGameMode::TrySpawnCustomer()
{
	ATableStation* Table = FindFreeTable();
	if (!Table) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACustomer* Cust = GetWorld()->SpawnActor<ACustomer>(ACustomer::StaticClass(), CustomerSpawnPoint, FRotator::ZeroRotator, Params);
	if (!Cust) return;

	if (CubeMesh)
	{
		Cust->BodyMesh->SetStaticMesh(CubeMesh);
		Cust->HeadMesh->SetStaticMesh(CubeMesh);
	}
	if (RedMaterial)
	{
		Cust->BodyMesh->SetMaterial(0, RedMaterial);
		Cust->HeadMesh->SetMaterial(0, RedMaterial);
	}

	FOrderItem Item;
	Item.Pizza = (EPizzaType)(FMath::RandRange(0, (int32)EPizzaType::MAX - 1));
	Item.Size = EPizzaSize::Medium;
	Cust->CurrentOrder.Items = { Item };
	Cust->CurrentOrder.OrderId = NextOrderId++;
	Cust->CurrentOrder.CustomerRef = Cust;
	Cust->CurrentOrder.State = EOrderState::NotTaken;

	Cust->AssignToTable(Table);
}
