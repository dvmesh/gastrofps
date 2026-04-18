#include "Stations/Station.h"
#include "Components/StaticMeshComponent.h"

AStation::AStation()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

bool AStation::CanInteract_Implementation(APawn* Interactor) const
{
	return Interactor != nullptr;
}

void AStation::Interact_Implementation(APawn* Interactor)
{
	// Baza — nic nie robi. Subclasses override.
}

FText AStation::GetPromptText_Implementation(APawn* Interactor) const
{
	return FText::FromString(TEXT("Interakt"));
}
