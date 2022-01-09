// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_ThrowRope.h"

#include "Characters/SwingProjCharacter.h"

void UAnimNotify_ThrowRope::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	ASwingProjCharacter* CachedOwner = Cast<ASwingProjCharacter>(MeshComp->GetOwner());
	if (!IsValid(CachedOwner))
	{
		return;
	}

	CachedOwner->AttachToRope();
}
