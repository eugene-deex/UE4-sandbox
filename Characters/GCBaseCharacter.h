// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"

#include "GCBaseCharacter.generated.h"

class UGCBaseCharacterMovementComponent;
	
UCLASS(Abstract, NotBlueprintable)
class GAMECODE_API AGCBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGCBaseCharacter(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE UGCBaseCharacterMovementComponent* GetBaseCharacterMovementComponent() const { return GCBaseCharacterMovementComponent; }

	UFUNCTION(BlueprintCallable)
	bool IsProning() const;

	void SetIsProning(bool Value) const;

	UFUNCTION(BlueprintCallable)
	bool IsCrouching() const;

	UFUNCTION(BlueprintCallable)
	float GetStaminaValue() const { return StaminaValue; }

	FORCEINLINE float GetBodyOffset() const { return BodyOffset; }
	FORCEINLINE float GetIKLeftOffset() const { return IKLeftOffset; }
	FORCEINLINE float GetIKRightOffset() const { return IKRightOffset; }
	
	virtual void MoveForward(float Value) {};
	virtual void MoveRight(float Value) {};
	virtual void LookUp(float Value) {};
	virtual void Turn(float Value) {};
	
	virtual void SwimForward(float Value) {};
	virtual void SwimRight(float Value) {};
	virtual void SwimUp(float Value) {};

	virtual void Jump() override;
	
	void Prone();
	void Prone2Crouch();

	// adjust springArm height funcs
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	virtual void OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	void PlayerHeightAdjust(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

	virtual void LookUpAtRate(float Value) {};
	virtual void TurnAtRate(float Value) {}; 
	
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;

	virtual void StartSprint();
	virtual void StopSprint();
	
	void Ragdoll();
	
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "IK Settings")
	FName IKLeftSocketName; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "IK Settings")
	FName IKRightSocketName; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Audio)
	class USoundCue* Jump_Cue;
	
protected:

	//UFUNCTION(BlueprintImplementableEvent, Category = "Character | Movement")   -- BP only 
	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement") // BP & C++ implementation
	void OnSprintStart();
	virtual void OnSprintStart_Implementation();
	
	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement")
	void OnSprintStop();
	virtual void OnSprintStop_Implementation();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Controls")
	float BaseTurnRate = 45.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Controls")
	float BaseLookUpRate = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Movement")
	float SprintSpeed = 800.f;

	UGCBaseCharacterMovementComponent* GCBaseCharacterMovementComponent;

	virtual bool CanSprint();

	float IKLeftOffset = 0.f;
	float IKRightOffset = 0.f;
	float BodyOffset = 0.f;

	// STAMINA
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Character | Stamina", meta = (ClampMin = 0.f, UIMin = 0.f))
	float StaminaMax = 70.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Character | Stamina", meta = (ClampMin = 0.f, UIMin = 0.f))
	float StaminaRestoreVelocity = 30.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Character | Stamina", meta = (ClampMin = 0.f, UIMin = 0.f))
	float StaminaSprintConsumptionVelocity = 30.f; // how fast stamina will be out

private:

	float StaminaValue; // current stamina value
	bool bIsSprintRequested = false;
	void TryChangeSprintState();
	UAudioComponent* JumpAudioComponent;
	bool bIsDead = false;
};
