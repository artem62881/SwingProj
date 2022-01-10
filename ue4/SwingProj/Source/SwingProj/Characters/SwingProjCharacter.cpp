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
#include "CableComponent.h"

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

	HookMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hook"));
	HookMesh->SetupAttachment(GetMesh());
	HookMesh->SetMobility(EComponentMobility::Movable);
	
	Rope = CreateDefaultSubobject<UCableComponent>(TEXT("Rope"));
	Rope->SetupAttachment(GetMesh());
	HookMesh->SetMobility(EComponentMobility::Movable);
}

void ASwingProjCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	HookMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName(TEXT("HandGrabSocket")));
	
	Rope->CableLength = 100.f;
	Rope->CableWidth = 3.f;
	Rope->bEnableStiffness = true;
	Rope->SetWorldLocation(GetMesh()->GetSocketLocation(FName(TEXT("BeltSocket"))));
	Rope->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName(TEXT("BeltSocket")));
	//Rope->SetWorldLocation(GetMesh()->GetSocketLocation(FName(TEXT("HandGrabSocket"))));
	Rope->SetAttachEndTo(this, FName(TEXT("HookMesh")));
}

USPBaseCharacterMovementComponent* ASwingProjCharacter::GetBaseCharacterMovementComponent() const
{
	return BaseCharacterMovementComponent;
}

void ASwingProjCharacter::Jump()
{
	Super::Jump();
	if (bIsSwinging && GetCharacterMovement()->IsFalling())
	{
		DettachFromRope();
	}
}

FRotator ASwingProjCharacter::GetCurrentRopeRotation() const
{
	return CurrentRopeVector.ToOrientationRotator() - GetActorRotation();
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
	PlayerInputComponent->BindAction("AttachToRope", IE_Pressed, this, &ASwingProjCharacter::ThrowRope);
	
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

void ASwingProjCharacter::EquipRope()
{
	if (IsValid(Rope))
	{
		HookMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName(TEXT("HandGrabSocket")));
		HookMesh->SetWorldLocation(GetMesh()->GetSocketLocation(FName(TEXT("HandGrabSocket"))));
		Rope->CableLength = 100.f;
		Rope->SetWorldLocation(GetMesh()->GetSocketLocation(FName(TEXT("BeltSocket"))));
		Rope->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName(TEXT("BeltSocket")));
	}
}

void ASwingProjCharacter::ThrowRope()
{
	if (bIsSwinging || IsValid(CurrentRopeSwingAttachActor))
	{
		DettachFromRope();
		return;
	}
	
	CurrentRopeSwingAttachActor = nullptr;
	float MinCosine = 0.35f;
	for (uint8 i = 0; i < GetCurrentAvailableInteractiveActors().Num(); ++i)
	{
		AInteractiveActor* CurrentActor = GetCurrentAvailableInteractiveActors()[i];
		float CurrentCosine = FVector::DotProduct(FollowCamera->GetForwardVector(), (CurrentActor->GetActorLocation() - GetActorLocation()).GetSafeNormal());
		if (CurrentActor->IsA<ARopeSwingAttachmentActor>() && CurrentCosine > MinCosine)
		{
			MinCosine = CurrentCosine; 
			CurrentRopeSwingAttachActor = StaticCast<ARopeSwingAttachmentActor*>(CurrentActor);
		}
	}

	if (IsValid(CurrentRopeSwingAttachActor))
	{
		CurrentRopeVector = CurrentRopeSwingAttachActor->GetActorLocation() - GetActorLocation();
		PlayAnimMontage(ThrowMontage);
	}
}

void ASwingProjCharacter::UpdateRopeSwing(float DeltaTime)
{
	if (!bIsSwinging || !IsValid(CurrentRopeSwingAttachActor))
	{
		return;
	}

	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + CurrentRopeVector, FColor::Green, false, -1, 0, 2);
	
	CurrentRopeVector = CurrentRopeSwingAttachActor->GetActorLocation() - GetActorLocation();
	FVector CurrentRopeVectorNormal = CurrentRopeVector.GetSafeNormal();
	
	if ((CurrentRopeVector.Size() > CurrentRopeLength || FMath::IsNearlyEqual(CurrentRopeVector.Size(), CurrentRopeLength, 10.f)) && bIsRopeStretched == false)
	{
		bIsRopeStretched = true;
	}
		
	if (bIsRopeStretched)
	{
		FVector RopeImpulse = CurrentRopeVectorNormal * GetVelocity().Size() * GetCharacterMovement()->Mass * RopeImpulseRatio * FVector::DotProduct(CurrentRopeVectorNormal, -GetVelocity().GetSafeNormal());
		if (FVector::DotProduct(CurrentRopeVectorNormal, FVector::UpVector) < 0.f || FVector::DotProduct(CurrentRopeVectorNormal, GetVelocity().GetSafeNormal()) > 0.f)
		{
			bIsRopeStretched = false;
			return;
		}
		
		GetCharacterMovement()->AddImpulse(RopeImpulse);
		//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + RopeImpulse.GetSafeNormal() * 150.f, FColor::Red, false, -1, 0, 4);
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
	//TODO
	OnRopeAttached();
}

void ASwingProjCharacter::OnRopeAttached()
{
	CurrentRopeLength = CurrentRopeVector.Size();
	bIsSwinging = true;
	bIsRopeStretched = false;
	
	if (IsValid(Rope) && IsValid(CurrentRopeSwingAttachActor))
	{
		HookMesh->AttachToComponent(CurrentRopeSwingAttachActor->GetMesh(), FAttachmentTransformRules::KeepWorldTransform);
		HookMesh->SetWorldLocation(CurrentRopeSwingAttachActor->GetActorLocation());
		Rope->SetWorldLocation(GetMesh()->GetSocketLocation(FName(TEXT("HandGrabSocket"))));
		Rope->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName(TEXT("HandGrabSocket")));
		Rope->CableLength = CurrentRopeLength * 0.7f;
	}
}

void ASwingProjCharacter::DettachFromRope()
{
	EquipRope();
	bIsSwinging = false;
	if (IsValid(CurrentRopeSwingAttachActor))
	{
		CurrentRopeSwingAttachActor = nullptr;
	}
	CurrentRopeVector = FVector::ZeroVector;
	CurrentRopeLength = 0.f;
	bIsRopeStretched = false;
}
