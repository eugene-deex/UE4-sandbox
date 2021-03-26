// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"

#include "GCBaseCharacterMovementComponent.generated.h"

class ACharacter;


UCLASS()
class GAMECODE_API UGCBaseCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	FORCEINLINE bool IsSprinting() { return bIsSprinting; }

	void StartSprint();
	void StopSprint();

	virtual float GetMaxSpeed() const override;
	virtual bool CanCrouchInCurrentState() const;
	virtual void UnCrouch(bool bClientSimulation) override;

	virtual void OnMovementModeChanged(EMovementMode, uint8) override;

	void SetIsOutOfStamina(bool Value);
	
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = "Character movement: sprint", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SprintSpeed = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = "Character movement: Speed out of stamina", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxSpeedOutOfStamina = 100.f;

	UFUNCTION() 
	bool IsOutOfStamina() { return bIsOutOfStamina; }
	
	FORCEINLINE bool IsProning() { return bIsProning; }

	FORCEINLINE void SetIsProning(bool Value) { bIsProning = Value; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Prone settings")
	float ProneCapsuleRadius = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Prone settings")
	float ProneCapsuleHalfHeight = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Prone settings")
	float ProneMaxSpeed = 100.f;

	UPROPERTY(Category = "Character Movement: Swimming", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SwimmingCapsuleRadius = 60.f;

	UPROPERTY(Category = "Character Movement: Swimming", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SwimmingCapsuleHalfHeight = 60.f;

private:
	bool bIsSprinting;
	bool bIsProning = false;
	bool bIsOutOfStamina = false;
};
