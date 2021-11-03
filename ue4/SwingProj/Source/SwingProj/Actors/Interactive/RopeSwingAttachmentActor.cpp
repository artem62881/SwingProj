// Fill out your copyright notice in the Description page of Project Settings.


#include "RopeSwingAttachmentActor.h"
#include "Components/SphereComponent.h"

ARopeSwingAttachmentActor::ARopeSwingAttachmentActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttacmentMesh"));
	MeshComponent -> SetupAttachment(RootComponent);
	
	OverlapVolume = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapVolume"));
	OverlapVolume->SetupAttachment(RootComponent);
	OverlapVolume->SetCollisionProfileName(FName("PawnInteractionVolume"));
	OverlapVolume->SetGenerateOverlapEvents(true);
	
}
