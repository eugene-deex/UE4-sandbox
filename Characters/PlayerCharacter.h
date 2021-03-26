// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCBaseCharacter.h"
#include "Components/TimelineComponent.h"

#include "PlayerCharacter.generated.h"

class UGCBaseCharacterMovementComponent;

UCLASS(Blueprintable)
class GAMECODE_API APlayerCharacter : public AGCBaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	UGCBaseCharacterMovementComponent* GetBaseCharacterMovementComponent() const;

	// test
	bool Trace(const FVector& Start, const FVector& End, FHitResult& Result);

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void MoveForward(float Value) override;
	virtual void MoveRight(float Value) override;
	virtual void Turn(float Value) override;
	virtual void LookUp(float Value) override;
	virtual void TurnAtRate(float Value) override;
	virtual void LookUpAtRate(float Value) override;
	
	virtual void SwimForward(float Value) override;
	virtual void SwimRight(float Value) override;
	virtual void SwimUp(float Value) override;

	virtual void OnSprintStart_Implementation() override;
	virtual void OnSprintStop_Implementation() override;

	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Camera")
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Camera")
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = "Sprint Settings")
	UCurveFloat* SpringArmCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint Settings")
	float SprintArmLength = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint Settings")
	float DefaultArmLength = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = "IK Settings", meta = (ClampMin=0.f, UIMin=0.f))
	float IKInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = "IK Settings", meta = (ClampMin=0.f, UIMin=0.f))
	float IKExtendMaxDist = 50.f;

private:

	FTimeline Timeline;

	UFUNCTION()
	void ShiftSpringArm(float Value);

	float ComputeIKOffset(const FName& SocketName); // count leg offset for TwoBoneIK 

	float IKTraceDist = 0.f; // linetrace length
	float IKActorScale = 0.f; // Actor's possible scale value.

	void DrawDebug(const FColor& color, const FVector loc);

	float IKCrouchedOffset = 0.f;
	float IKStayedOffset = 2.5f;
};
