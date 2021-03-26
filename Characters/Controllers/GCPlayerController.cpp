// Fill out your copyright notice in the Description page of Project Settings.


#include "GCPlayerController.h"
#include "../GCBaseCharacter.h"

void AGCPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	CachedBaseCharacter = Cast<AGCBaseCharacter>(InPawn); // dynamic cast - called once per game
}

void AGCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	// InputComponent - created in AActor when called Super::SetupInputComponent()

	InputComponent->BindAxis("MoveForward", this, &AGCPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AGCPlayerController::MoveRight);
	InputComponent->BindAxis("Turn", this, &AGCPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AGCPlayerController::LookUp);
	
	InputComponent->BindAxis("TurnAtRate", this, &AGCPlayerController::TurnAtRate);
	InputComponent->BindAxis("LookUpAtRate", this, &AGCPlayerController::LookUpAtRate);

	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AGCPlayerController::Jump);

	InputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &AGCPlayerController::StartSprint);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &AGCPlayerController::StopSprint);

	InputComponent->BindAction("ProneCrouch", EInputEvent::IE_Pressed, this, &AGCPlayerController::SetProneTimer);
	InputComponent->BindAction("ProneCrouch", EInputEvent::IE_Released, this, &AGCPlayerController::ChangeProneCrouchState);
	
	InputComponent->BindAxis("SwimForward", this, &AGCPlayerController::SwimForward);
	InputComponent->BindAxis("SwimRight", this, &AGCPlayerController::SwimRight);
	InputComponent->BindAxis("SwimUp", this, &AGCPlayerController::SwimUp);

	InputComponent->BindAction("Ragdoll", EInputEvent::IE_Pressed, this, &AGCPlayerController::Ragdoll);
}

void AGCPlayerController::Ragdoll()
{
	CachedBaseCharacter->Ragdoll();
}

void AGCPlayerController::SetProneTimer()
{
	GetWorld()->GetTimerManager().SetTimer(ProneTimerHnd, this, &AGCPlayerController::OnProneTimer, ProneDelay, false);

	if (CachedBaseCharacter->IsProning())
		CurrentState = EBodyPoseState::PRONE;
	else if (CachedBaseCharacter->IsCrouching())
		CurrentState = EBodyPoseState::CROUCH;
	else
		CurrentState = EBodyPoseState::STAY;


	// SHORT press:

	// stay -> crouch
	if(EBodyPoseState::STAY == CurrentState)
	{
		CachedBaseCharacter->Crouch();
	}

	// prone -> crouch (only if impossible to stay)
	if (EBodyPoseState::PRONE == CurrentState)
	{
		CachedBaseCharacter->Prone2Crouch();
	}
	
	if (EBodyPoseState::CROUCH == CurrentState)
	{
		CachedBaseCharacter->UnCrouch();
	}
}

void AGCPlayerController::OnProneTimer()
{

	if (CachedBaseCharacter->IsProning())
		CurrentState = EBodyPoseState::PRONE;
	else if (CachedBaseCharacter->IsCrouching())
		CurrentState = EBodyPoseState::CROUCH;
	else
		CurrentState = EBodyPoseState::STAY;

	// LONG PRESS 

	// crouch to prone (without stay first)
	if (EBodyPoseState::CROUCH == CurrentState)
	{

		CachedBaseCharacter->UnCrouch();
		CachedBaseCharacter->Prone();
	}
}

void AGCPlayerController::ChangeProneCrouchState()
{
	UE_LOG(LogTemp, Warning, TEXT("AGCPlayerController::ClearProneTimer - CLEAR"))
		GetWorld()->GetTimerManager().ClearTimer(ProneTimerHnd);
}

void AGCPlayerController::MoveForward(float Value)
{
	if (CachedBaseCharacter.IsValid())   // TSoftObjectPtr::IsValid
	{
		CachedBaseCharacter->MoveForward(Value);
	}
}

void AGCPlayerController::MoveRight(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->MoveRight(Value);
	}
}

void AGCPlayerController::Turn(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Turn(Value);
	}
}

void AGCPlayerController::LookUp(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->LookUp(Value);
	}
}

void AGCPlayerController::TurnAtRate(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->TurnAtRate(Value);
	}
}

void AGCPlayerController::LookUpAtRate(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->LookUpAtRate(Value);
	}
}

void AGCPlayerController::Jump()
{
	if (CachedBaseCharacter.IsValid())
	{
		if (CachedBaseCharacter->IsProning())
		{
			CachedBaseCharacter->Prone2Crouch();
			return;
		}

		CachedBaseCharacter->Jump();
	}
}

void AGCPlayerController::SwimForward(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimForward(Value);
	}
}

void AGCPlayerController::SwimRight(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimRight(Value);
	}
}

void AGCPlayerController::SwimUp(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimUp(Value);
	}
}

void AGCPlayerController::StartSprint()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartSprint();
	}
}

void AGCPlayerController::StopSprint()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopSprint();
	}
}

