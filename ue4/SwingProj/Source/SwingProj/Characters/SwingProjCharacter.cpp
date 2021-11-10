// Copyright Epic Games, Inc. All Rights Reserved.

#include "SwingProjCharacter.h"

#include "DrawDebugHelpers.h"
#include "Actors/Interactive/RopeSwingAttachmentActor.h"
#include "Camera/CameraComponent.h"
#include "Chaos/ChaosDebugDraw.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/MovementComponents/SPBaseCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

//////////////////////////////////////////////////////////////////////////
// ASwingProjCharacter

ASwingProjCharacter::ASwingProjCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USPBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	BaseCharacterMovementComponent = StaticCast<USPBaseCharacterMovementComponent*>(GetCharacterMovement());
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

USPBaseCharacterMovementComponent* ASwingProjCharacter::GetBaseCharacterMovementComponent() const
{
	return BaseCharacterMovementComponent;
}

void ASwingProjCharacter::Jump()
{
	Super::Jump();
	if (bIsSwinging)
	{
		DettachFromRope();
	}
}

void ASwingProjCharacter::RegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActors.AddUnique(InteractiveActor);
}

void ASwingProjCharacter::UnRegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActors.RemoveSingleSwap(InteractiveActor);
}

TArray<AInteractiveActor*> ASwingProjCharacter::GetCurrentAvailableInteractiveActors() const
{
	return AvailableInteractiveActors;
}

ARopeSwingAttachmentActor* ASwingProjCharacter::GetCurrentRopeSwingAttachActor() const
{
	return CurrentRopeSwingAttachActor;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASwingProjCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("AttachToRope", IE_Pressed, this, &ASwingProjCharacter::AttachToRope);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &ASwingProjCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASwingProjCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASwingProjCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASwingProjCharacter::LookUpAtRate);
}

void ASwingProjCharacter::UpdateRopeSwing(float DeltaTime)
{
	if (!bIsSwinging || !IsValid(CurrentRopeSwingAttachActor))
	{
		return;
	}
	
	CurrentRopeVector = CurrentRopeSwingAttachActor->GetActorLocation() - GetActorLocation();
	CurrentRopeVectorNormal = CurrentRopeVector.GetSafeNormal();
	if (FMath::IsNearlyEqual(CurrentRopeVector.Size(), CurrentRopeLength, 0.1f) || CurrentRopeVector.Size() > CurrentRopeLength)
	{
		bIsRopeStretched = true;
	}
	else
	{
		bIsRopeStretched = false;
	}
	
	FVector RopeAccel = FVector::DotProduct(-CurrentRopeVectorNormal, GetVelocity().GetSafeNormal()) * CurrentRopeVector * FMath::Abs(GetCharacterMovement()->GetGravityZ()) * RopeForceRatio;
	DrawDebugRopeSwing();
	
	if (bIsRopeStretched)
	{
		GetCharacterMovement()->AddForce(RopeAccel);		
	}
	
	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + RopeAccel / 10.f, FColor::Purple, false, -1.f, 0, 2);
}

void ASwingProjCharacter::DrawDebugRopeSwing()
{
	if (!bIsRopeStretched)
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + CurrentRopeVector, FColor::Green, false, -1, 0, 2);
	}
	else
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + CurrentRopeVector, FColor::Red, false, -1, 0, 2);
	}
	
}

void ASwingProjCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASwingProjCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASwingProjCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateRopeSwing(DeltaTime);
}

void ASwingProjCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ASwingProjCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ASwingProjCharacter::AttachToRope()
{
	if (bIsSwinging)
	{
		return;
	}
	CurrentRopeSwingAttachActor = nullptr;

	for (uint8 i = 0; i < GetCurrentAvailableInteractiveActors().Num(); ++i)
	{
		AInteractiveActor* CurrentActor = GetCurrentAvailableInteractiveActors()[i];
		if (CurrentActor->IsA<ARopeSwingAttachmentActor>())
		{
			CurrentRopeSwingAttachActor = StaticCast<ARopeSwingAttachmentActor*>(CurrentActor);
			break;
		}
	}

	if (!IsValid(CurrentRopeSwingAttachActor))
	{
		return;
	}
	
	CurrentRopeVector = CurrentRopeSwingAttachActor->GetActorLocation() - GetActorLocation();
	CurrentRopeLength = CurrentRopeVector.Size();
	bIsSwinging = true;
}

void ASwingProjCharacter::DettachFromRope()
{
	if (!GetBaseCharacterMovementComponent()->IsFalling() || !bIsSwinging)
	{
		return;
	}
	bIsSwinging = false;
	
	if (IsValid(CurrentRopeSwingAttachActor))
	{
		CurrentRopeSwingAttachActor = nullptr;
	}
	CurrentRopeVectorNormal = FVector::ZeroVector;
	CurrentRopeLength = 0.f;
	bIsRopeStretched = false;
}
