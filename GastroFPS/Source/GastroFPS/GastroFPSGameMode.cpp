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
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Engine/World.h"

AGastroFPSGameMode::AGastroFPSGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	GameStateClass = AGastroFPSGameState::StaticClass();
	HUDClass = AGastroHUD::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> PawnBPFinder(
		TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	if (PawnBPFinder.Succeeded())
	{
		DefaultPawnClass = PawnBPFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PCBPFinder(
		TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonPlayerController"));
	if (PCBPFinder.Succeeded())
	{
		PlayerControllerClass = PCBPFinder.Class;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Game/LevelPrototyping/Meshes/SM_Cube"));
	if (CubeFinder.Succeeded())
	{
		CubeMesh = CubeFinder.Object;
	}

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

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GrayFinder(
		TEXT("/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray"));
	if (GrayFinder.Succeeded()) GrayMaterial = GrayFinder.Object;
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

	// Najpierw trace down PRZED destroyem (żeby znaleźć istniejący floor)
	float FloorZ = FindFloorZ(Origin);

	if (bCleanLevelAtStart)
	{
		CleanLevel();
	}

	SpawnWorld(Origin, FloorZ);
	bWorldSpawned = true;

	if (AGastroFPSGameState* GS = GetWorld()->GetGameState<AGastroFPSGameState>())
	{
		GS->StartMatch(MatchDurationSec);
	}
}

float AGastroFPSGameMode::FindFloorZ(const FVector& Origin) const
{
	const FVector Start = Origin + FVector(0.f, 0.f, 50.f);
	const FVector End = Origin - FVector(0.f, 0.f, 1000.f);
	FHitResult Hit;
	FCollisionQueryParams Params;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return Hit.Location.Z;
	}
	// Fallback: zakładamy że PlayerStart ma ~96cm capsule i floor jest pod nim
	return Origin.Z - 96.f;
}

void AGastroFPSGameMode::CleanLevel()
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> ToDestroy;
	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		ToDestroy.Add(*It);
	}
	for (AActor* A : ToDestroy)
	{
		if (A) A->Destroy();
	}
	UE_LOG(LogTemp, Display, TEXT("[GastroFPS] CleanLevel: destroyed %d static mesh actors"), ToDestroy.Num());
}

void AGastroFPSGameMode::SpawnWorld(const FVector& Origin, float FloorZ)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// ============ Podłoga (duży płaski sześcian pod origin) ============
	{
		AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(),
			FVector(Origin.X, Origin.Y, FloorZ - 10.f),
			FRotator::ZeroRotator,
			Params);
		if (Floor && Floor->GetStaticMeshComponent())
		{
			Floor->SetMobility(EComponentMobility::Movable); // by móc ustawić scale po spawn
			if (CubeMesh) Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
			Floor->GetStaticMeshComponent()->SetRelativeScale3D(FVector(40.f, 40.f, 0.2f));
			if (GrayMaterial) Floor->GetStaticMeshComponent()->SetMaterial(0, GrayMaterial);
			Floor->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
			Floor->SetMobility(EComponentMobility::Static);
		}
	}

	// ============ Stoliki (3 w rządku, Y spread) ============
	const float TableScale_X = 1.2f, TableScale_Y = 1.2f, TableScale_Z = 0.8f;
	const float TableHalfZ = 50.f * TableScale_Z; // cube=100cm, half=50, * scale
	const float TableForward = 500.f;
	for (int32 i = 0; i < NumTables; ++i)
	{
		const float YOffset = (i - (NumTables - 1) * 0.5f) * 350.f;
		FVector TableLoc = Origin + FVector(TableForward, YOffset, 0.f);
		TableLoc.Z = FloorZ + TableHalfZ;
		if (ATableStation* Table = World->SpawnActor<ATableStation>(
			ATableStation::StaticClass(), TableLoc, FRotator::ZeroRotator, Params))
		{
			if (CubeMesh) Table->Mesh->SetStaticMesh(CubeMesh);
			Table->Mesh->SetRelativeScale3D(FVector(TableScale_X, TableScale_Y, TableScale_Z));
			if (YellowMaterial) Table->Mesh->SetMaterial(0, YellowMaterial);
			if (CubeMesh) Table->CashVisual->SetStaticMesh(CubeMesh);
			if (CubeMesh) Table->DirtyVisual->SetStaticMesh(CubeMesh);
			if (GreenMaterial) Table->CashVisual->SetMaterial(0, GreenMaterial);
			if (RedMaterial) Table->DirtyVisual->SetMaterial(0, RedMaterial);
			Table->TableNumber = i + 1;
			Tables.Add(Table);
		}
	}

	// ============ POS (z prawej strony kuchni) ============
	const float POSForward = 900.f;
	const float POSScale_X = 0.6f, POSScale_Y = 1.0f, POSScale_Z = 1.0f;
	const float POSHalfZ = 50.f * POSScale_Z;
	FVector POSLoc = Origin + FVector(POSForward, 300.f, 0.f);
	POSLoc.Z = FloorZ + POSHalfZ;
	if (APOSStation* POS = World->SpawnActor<APOSStation>(APOSStation::StaticClass(), POSLoc, FRotator::ZeroRotator, Params))
	{
		if (CubeMesh) POS->Mesh->SetStaticMesh(CubeMesh);
		POS->Mesh->SetRelativeScale3D(FVector(POSScale_X, POSScale_Y, POSScale_Z));
		if (BlueMaterial) POS->Mesh->SetMaterial(0, BlueMaterial);
	}

	// ============ Pass (obok POS) ============
	const float PassForward = 1100.f;
	const float PassScale_X = 1.0f, PassScale_Y = 1.0f, PassScale_Z = 0.5f;
	const float PassHalfZ = 50.f * PassScale_Z;
	FVector PassLoc = Origin + FVector(PassForward, 300.f, 0.f);
	PassLoc.Z = FloorZ + PassHalfZ;
	if (APassStation* Pass = World->SpawnActor<APassStation>(APassStation::StaticClass(), PassLoc, FRotator::ZeroRotator, Params))
	{
		if (CubeMesh) Pass->Mesh->SetStaticMesh(CubeMesh);
		Pass->Mesh->SetRelativeScale3D(FVector(PassScale_X, PassScale_Y, PassScale_Z));
		if (GreenMaterial) Pass->Mesh->SetMaterial(0, GreenMaterial);
	}

	// ============ Customer spawn point (przy "drzwiach" — ujemny X) ============
	CustomerSpawnPoint = Origin + FVector(-500.f, -300.f, 0.f);
	CustomerSpawnPoint.Z = FloorZ + 90.f; // Customer center około 90cm nad floor (body scale 1.8 * 50cm half)
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
	// Zbierz wszystkie wolne stoliki i wybierz losowy
	TArray<ATableStation*> Free;
	for (const TWeakObjectPtr<ATableStation>& W : Tables)
	{
		if (W.IsValid() && W->State == ETableState::Free && !W->Occupant.IsValid())
		{
			Free.Add(W.Get());
		}
	}
	if (Free.Num() == 0) return nullptr;
	const int32 Idx = FMath::RandRange(0, Free.Num() - 1);
	return Free[Idx];
}

void AGastroFPSGameMode::TrySpawnCustomer()
{
	ATableStation* Table = FindFreeTable();
	if (!Table) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Mały random offset żeby kilku spawnujących się w tym samym miejscu nie nakładało się
	const FVector Offset = FVector(
		FMath::RandRange(-30.f, 30.f),
		FMath::RandRange(-30.f, 30.f),
		0.f);

	ACustomer* Cust = GetWorld()->SpawnActor<ACustomer>(
		ACustomer::StaticClass(), CustomerSpawnPoint + Offset, FRotator::ZeroRotator, Params);
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
