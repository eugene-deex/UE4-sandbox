// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Controller.h"
#include "GCPlayerController.generated.h"

UENUM()
enum class EBodyPoseState : uint8
{
	STAY,
	CROUCH,
	PRONE
};

UCLASS()
class GAMECODE_API AGCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetPawn(APawn* InPawn) override; // set ptr to CachedBaseCharacter

protected:
	virtual void SetupInputComponent() override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);	
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);
	void Jump();
	void Ragdoll();

	void SwimForward(float Value);
	void SwimRight(float Value);
	void SwimUp(float Value);

	void StartSprint();
	void StopSprint();

	UFUNCTION()
	void OnProneTimer();
	void SetProneTimer();
	void ChangeProneCrouchState();

	TSoftObjectPtr<class AGCBaseCharacter> CachedBaseCharacter;

	FTimerHandle ProneTimerHnd; // prone timer
	float ProneDelay = 0.5f;

	EBodyPoseState CurrentState = EBodyPoseState::STAY;
};
