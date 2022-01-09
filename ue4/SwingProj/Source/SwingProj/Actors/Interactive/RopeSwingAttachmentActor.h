// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveActor.h"
#include "RopeSwingAttachmentActor.generated.h"

/**
 * 
 */
class UStaticMeshComponent;

UCLASS()
class SWINGPROJ_API ARopeSwingAttachmentActor : public AInteractiveActor
{
	GENERATED_BODY()
	
public:
	ARopeSwingAttachmentActor();

	UStaticMeshComponent* GetMesh() const { return MeshComponent; };
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float OverlapVolumeRadius = 200.f;
};
