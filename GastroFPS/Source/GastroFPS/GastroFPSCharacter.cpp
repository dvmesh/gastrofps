// Copyright Epic Games, Inc. All Rights Reserved.

#include "GastroFPSCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GastroFPS.h"
#include "Stations/Station.h"
#include "Customers/Customer.h"
#include "Core/GastroFPSGameState.h"
#include "Engine/World.h"

AGastroFPSCharacter::AGastroFPSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void AGastroFPSCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CurrentInteractTarget = TraceForStation();
}

void AGastroFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AGastroFPSCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AGastroFPSCharacter::DoJumpEnd);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGastroFPSCharacter::MoveInput);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGastroFPSCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AGastroFPSCharacter::LookInput);
	}
	else
	{
		UE_LOG(LogGastroFPS, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
	}

	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &AGastroFPSCharacter::OnInteractPressed);
}

void AGastroFPSCharacter::MoveInput(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AGastroFPSCharacter::LookInput(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void AGastroFPSCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AGastroFPSCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AGastroFPSCharacter::DoJumpStart() { Jump(); }
void AGastroFPSCharacter::DoJumpEnd() { StopJumping(); }

void AGastroFPSCharacter::OnInteractPressed()
{
	AStation* Target = TraceForStation();
	if (!Target) return;
	if (!Target->CanInteract(this)) return;
	Target->Interact(this);
}

AStation* AGastroFPSCharacter::TraceForStation() const
{
	if (!FirstPersonCameraComponent) return nullptr;

	const FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	const FVector Fwd = FirstPersonCameraComponent->GetForwardVector();
	const FVector End = Start + Fwd * InteractTraceCm;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	if (!bHit) return nullptr;

	AActor* HitActor = Hit.GetActor();
	return Cast<AStation>(HitActor);
}

void AGastroFPSCharacter::AddPeekedOrder(const FOrderData& Order)
{
	// Dedupe: jeżeli to zamówienie już w queue (po OrderId), nie dodawaj
	for (const FOrderData& Existing : PeekedOrders)
	{
		if (Existing.OrderId == Order.OrderId && Order.OrderId != -1)
		{
			return;
		}
	}
	PeekedOrders.Add(Order);
}

FOrderData AGastroFPSCharacter::ConsumeOldestPeekedOrder()
{
	if (PeekedOrders.Num() == 0) return FOrderData();
	FOrderData Out = PeekedOrders[0];
	PeekedOrders.RemoveAt(0);
	return Out;
}

void AGastroFPSCharacter::PickUpPizza(const FOrderData& Order)
{
	CarriedOrder = Order;
	bCarryingPizza = true;
}

void AGastroFPSCharacter::ClearCarriedPizza()
{
	CarriedOrder = FOrderData();
	bCarryingPizza = false;
}

bool AGastroFPSCharacter::DoesCarriedOrderMatch(const FOrderData& CustomerOrder) const
{
	if (!bCarryingPizza) return false;
	if (CarriedOrder.OrderId != -1 && CarriedOrder.OrderId == CustomerOrder.OrderId) return true;
	if (CarriedOrder.CustomerRef.IsValid() && CarriedOrder.CustomerRef == CustomerOrder.CustomerRef)
	{
		return true;
	}
	return false;
}

void AGastroFPSCharacter::PocketCash(int32 Amount)
{
	AGastroFPSGameState* GS = GetWorld()->GetGameState<AGastroFPSGameState>();
	if (GS) GS->AddMoney(Amount);
}
