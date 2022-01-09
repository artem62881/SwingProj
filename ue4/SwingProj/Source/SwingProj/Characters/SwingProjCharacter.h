// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactive/InteractiveActor.h"
#include "GameFramework/Character.h"
#include "SwingProjCharacter.generated.h"

class USPBaseCharacterMovementComponent;
class ARopeSwingAttachmentActor;
class UCableComponent;

UCLASS(config=Game)
class ASwingProjCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASwingProjCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void OnConstruction(const FTransform& Transform) override;
	
	USPBaseCharacterMovementComponent* GetBaseCharacterMovementComponent() const;
	
	virtual void Jump() override;

	void AttachToRope();

	UFUNCTION(BlueprintCallable)
	bool IsSwinging() const { return bIsSwinging; };
	
	UFUNCTION(BlueprintCallable)
	FRotator GetCurrentRopeRotation() const;
	
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rope, meta = (AllowPrivateAccess = "true"))
	UCableComponent* Rope;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rope, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* HookMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Rope, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* ThrowMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RopeImpulseRatio = 1.5f;
	
	virtual void Tick(float DeltaTime) override;
	
	void MoveForward(float Value);
	void MoveRight(float Value);


	void OnRopeAttached();
	
	void DettachFromRope();

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
private:
	TArray<AInteractiveActor*> AvailableInteractiveActors;

	FVector CurrentRopeVector = FVector::ZeroVector;
	float CurrentRopeLength = 0.f;
	bool bIsRopeStretched = false;
	bool bIsSwinging = false;

	void EquipRope();
	void ThrowRope();
	
	void UpdateRopeSwing(float DeltaTime);
	ARopeSwingAttachmentActor* CurrentRopeSwingAttachActor = nullptr;
};

