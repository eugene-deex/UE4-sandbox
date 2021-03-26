// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacterAnimInstance.h"
#include "../GCBaseCharacter.h"
#include "../../Components/GCBaseCharacterMovementComponent.h"

void UGCBaseCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	checkf(TryGetPawnOwner()->IsA<AGCBaseCharacter>(), TEXT("UGCBaseCharacterAnimInstance::NativeBeginPlay pawn owner should be AGCBaseCharacter"));

	CachedBaseCharacter = StaticCast<AGCBaseCharacter*>(TryGetPawnOwner());
}

// does not call in ABP preview window
void UGCBaseCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!CachedBaseCharacter.IsValid())
	{
		return;
	}

	UGCBaseCharacterMovementComponent* CharacterMovement = CachedBaseCharacter->GetBaseCharacterMovementComponent();

	Speed = CharacterMovement->Velocity.Size();
	bIsFalling = CharacterMovement->IsFalling();
	bIsCrouching = CharacterMovement->IsCrouching();
	bIsSprinting = CharacterMovement->IsSprinting();
	bIsOutOfStamina = CharacterMovement->IsOutOfStamina();
	bIsProning = CharacterMovement->IsProning();
	bIsSwimming = CharacterMovement->IsSwimming();
	
	// disable IK offsets if player is in air
	if (bIsFalling || Speed)
	{ 
		BodyOffset = RightEffectorLocation = LeftEffectorLocation = FVector::ZeroVector;
	}
	else
	{
		LeftEffectorLocation = FVector(CachedBaseCharacter->GetIKLeftOffset(), 0.f, 0.f);
		RightEffectorLocation = FVector(CachedBaseCharacter->GetIKRightOffset(), 0.f, 0.f);
		BodyOffset = FVector(0.f, 0.f, CachedBaseCharacter->GetBodyOffset());
	}

	GEngine->AddOnScreenDebugMessage(0, 2.f, FColor::White, FString::Printf(TEXT("Anim LEFT: %.1f ; RIGHT: %.1f ; BODY %.1f"),
		LeftEffectorLocation.X, RightEffectorLocation.X, BodyOffset.X
	));

	
}
