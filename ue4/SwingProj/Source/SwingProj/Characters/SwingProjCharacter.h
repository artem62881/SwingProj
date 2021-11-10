// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactive/InteractiveActor.h"
#include "GameFramework/Character.h"
#include "SwingProjCharacter.generated.h"

class USPBaseCharacterMovementComponent;
class ARopeSwingAttachmentActor;
UCLASS(config=Game)
class ASwingProjCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASwingProjCharacter(const FObjectInitializer& ObjectInitializer);

	USPBaseCharacterMovementComponent* GetBaseCharacterMovementComponent() const;
	
	virtual void Jump() override;
	
	void RegisterInteractiveActor(AInteractiveActor* InteractiveActor);
	void UnRegisterInteractiveActor(AInteractiveActor* InteractiveActor);
	
	TArray<AInteractiveActor*> GetCurrentAvailableInteractiveActors() const;
	ARopeSwingAttachmentActor* GetCurrentRopeSwingAttachActor() const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	USPBaseCharacterMovementComponent* BaseCharacterMovementComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RopeForceRatio = 1.f;
	
	virtual void Tick(float DeltaTime) override;
	
	void MoveForward(float Value);
	void MoveRight(float Value);

	void AttachToRope();
	void DettachFromRope();

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
private:
	TArray<AInteractiveActor*> AvailableInteractiveActors;

	FVector CurrentRopeVector = FVector::ZeroVector;
	FVector CurrentRopeVectorNormal = FVector::ZeroVector;
	float CurrentRopeLength = 0.f;
	bool bIsRopeStretched = false;
	bool bIsSwinging = false;
	
	void UpdateRopeSwing(float DeltaTime);
	void DrawDebugRopeSwing();
	ARopeSwingAttachmentActor* CurrentRopeSwingAttachActor = nullptr;
};

