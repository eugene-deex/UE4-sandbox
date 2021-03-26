// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

void UGCBaseCharacterMovementComponent::UnCrouch(bool bClientSimulation)
{
	if (bIsProning)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGCBaseCharacterMovementComponent::UnCrouch skipped is proning"))
		return;
	}

	Super::UnCrouch();
}

float UGCBaseCharacterMovementComponent::GetMaxSpeed() const
{
	float Result = Super::GetMaxSpeed();

	if (bIsProning)
	{
		return ProneMaxSpeed;
	}

	if (bIsOutOfStamina)
	{
		return MaxSpeedOutOfStamina;
	}

	if (bIsSprinting)
	{
		Result = SprintSpeed;
	}

	return Result;
}

void UGCBaseCharacterMovementComponent::StartSprint()
{
	if (bIsOutOfStamina)
	{
		return;
	}

	bIsSprinting = true;
	bForceMaxAccel = 1;
}

void UGCBaseCharacterMovementComponent::StopSprint()
{
	bIsSprinting = false;
	bForceMaxAccel = 0;
}

bool UGCBaseCharacterMovementComponent::CanCrouchInCurrentState() const
{
	if (!IsMovingOnGround())
	{
		return false;
	}

	return Super::CanCrouchInCurrentState();
}

void UGCBaseCharacterMovementComponent::SetIsOutOfStamina(bool Value)
{
	bIsOutOfStamina = Value;

	if (bIsOutOfStamina)
	{
		StopSprint();
	}
}

void UGCBaseCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (MOVE_Swimming == MovementMode)
	{
		// set swimming capsule size
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(SwimmingCapsuleRadius, SwimmingCapsuleHalfHeight);
	}
	else if(MOVE_Swimming == PreviousMovementMode)
	{
		UCapsuleComponent* DefaultCapsule = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent();

		// restore normal capsule size
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCapsule->GetUnscaledCapsuleRadius(), DefaultCapsule->GetUnscaledCapsuleHalfHeight(), true);
	}
}
