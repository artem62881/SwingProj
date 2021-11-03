// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveActor.h"
#include "Characters/SwingProjCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"


AInteractiveActor::AInteractiveActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	/*AttachmentPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AttachmentPoint"));
	RootComponent = AttachmentPoint;

	VisualMesh = CreateDefaultSubobject<UMeshComponent>(TEXT("Mesh"));
	VisualMesh->SetupAttachment(RootComponent);*/
	
	//OverlapVolume->SetupAttachment(RootComponent);
}

void AInteractiveActor::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(OverlapVolume))
	{
		OverlapVolume->OnComponentBeginOverlap.AddDynamic(this, &AInteractiveActor::OnInteractionVolumeOverlapBegin);
		OverlapVolume->OnComponentEndOverlap.AddDynamic(this, &AInteractiveActor::OnInteractionVolumeOverlapEnd);
	}
}

void AInteractiveActor::OnInteractionVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsOverlappingCharacterCapsule(OtherActor, OtherComp))
	{
		return;
	}
	ASwingProjCharacter* CachedCharacter = StaticCast<ASwingProjCharacter*>(OtherActor);
	CachedCharacter->RegisterInteractiveActor(this);
}

void AInteractiveActor::OnInteractionVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsOverlappingCharacterCapsule(OtherActor, OtherComp))
	{
		return;
	}
	ASwingProjCharacter* CachedCharacter = StaticCast<ASwingProjCharacter*>(OtherActor);
	CachedCharacter->UnRegisterInteractiveActor(this);
}

bool AInteractiveActor::IsOverlappingCharacterCapsule(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	ASwingProjCharacter* CachedCharacter = Cast<ASwingProjCharacter>(OtherActor);
	if (!IsValid(CachedCharacter))
	{
		return false;
	}

	if (Cast<UCapsuleComponent>(OtherComp) != CachedCharacter->GetCapsuleComponent())
	{
		return false;
	}
	
	return true;
}


