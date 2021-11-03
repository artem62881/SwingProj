// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveActor.h"
#include "RopeSwingAttachmentActor.generated.h"

/**
 * 
 */
UCLASS()
class SWINGPROJ_API ARopeSwingAttachmentActor : public AInteractiveActor
{
	GENERATED_BODY()
	
public:
	ARopeSwingAttachmentActor();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float OverlapVolumeRadius = 200.f;
};
