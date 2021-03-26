// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h" // move camera back slightly when sprint

#include "Components/CapsuleComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

class UGCBaseCharacterMovementComponent;

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->SetupAttachment(RootComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->bUsePawnControlRotation = false;
	CameraComponent->SetupAttachment(SpringArmComponent);

	GetCharacterMovement()->bOrientRotationToMovement = 1;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;

	IKActorScale = GetActorScale3D().Z;

	float CapsuleRadius, CapsuleHalfSize;
	GetCapsuleComponent()->GetScaledCapsuleSize(CapsuleRadius, CapsuleHalfSize);
	
	IKTraceDist = CapsuleHalfSize;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(SpringArmCurve))
	{
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindUFunction(this, "ShiftSpringArm");
		Timeline.AddInterpFloat(SpringArmCurve, TimelineCallback);
	}

}

void APlayerCharacter::ShiftSpringArm(float Value)
{
	SpringArmComponent->TargetArmLength = FMath::Lerp(SprintArmLength, DefaultArmLength, Value);
}

void APlayerCharacter::MoveForward(float Value)
{

	if ((GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling()) 
			&& !FMath::IsNearlyZero(Value, 1e-6f)) // more than 0.000006f  (10^-6)
	{
		// take current rotation angle of the controller
		FRotator Rotator(0.f, GetControlRotation().Yaw, 0.f);  // 0, 90,  0		
		
		// rotator -> rotation matrix * forward vector = new rotated vector
		FVector ForwardVector = Rotator.RotateVector(FVector::ForwardVector); // 1,0,0 -> 0,1,0

		AddMovementInput(ForwardVector, Value);
		//AddMovementInput(GetActorForwardVector(), Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if ((GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
		&& !FMath::IsNearlyZero(Value, 1e-6f)) // more than 0.000006f  (10^-6)
	{
		FRotator Rotator(0.f, GetControlRotation().Yaw, 0.f);
		
		FVector RightVector = Rotator.RotateVector(FVector::RightVector);

		AddMovementInput(RightVector, Value);
		//AddMovementInput(GetActorRightVector(), Value);
	}
}

void APlayerCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void APlayerCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void APlayerCharacter::TurnAtRate(float Value)
{
	AddControllerYawInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::LookUpAtRate(float Value)
{
	AddControllerPitchInput(Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void APlayerCharacter::OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void APlayerCharacter::SwimForward(float Value)
{
	// we want to swim in mouse direction, even up and down
	// Movement is more than 0.000006f (10^-6)
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value, 1e-6f))
	{
		// take current rotation angle of the controller
		FRotator PitchYawRotator(GetControlRotation().Pitch, GetControlRotation().Yaw, 0.f);

		// rotator -> rotation matrix * forward vector = new rotated vector
		FVector ForwardVector = PitchYawRotator.RotateVector(FVector::ForwardVector); 
		AddMovementInput(ForwardVector, Value);
	}
}

void APlayerCharacter::SwimRight(float Value)
{
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value, 1e-6f))
	{
		FRotator Rotator(0.f, GetControlRotation().Yaw, 0.f);
		FVector RightVector = Rotator.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
}

void APlayerCharacter::SwimUp(float Value)
{
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value, 1e-6f))
	{
		AddMovementInput(FVector::UpVector, Value);
	}
}

void APlayerCharacter::OnSprintStart_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("APlayerCharacter OnSprintStart_Implementation"));
	Timeline.Play();
}

void APlayerCharacter::OnSprintStop_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("APlayerCharacter OnSprintStop_Implementation"));
	Timeline.Reverse();
}


UGCBaseCharacterMovementComponent* APlayerCharacter::GetBaseCharacterMovementComponent() const
{
	return GCBaseCharacterMovementComponent;
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Timeline.TickTimeline(DeltaSeconds); // move camera on sprint

	// Count IK offsets for legs
	float LeftOffset = ComputeIKOffset(IKLeftSocketName);
	float RightOffset = ComputeIKOffset(IKRightSocketName); 
	float BOffsetTmp = 0.f;

	// right leg is lower than left one
	if (RightOffset < LeftOffset)
	{
		BOffsetTmp = RightOffset;
		LeftOffset -= RightOffset;
		RightOffset = 0.f;
	}
	else 
	{
		BOffsetTmp = LeftOffset;
		RightOffset -= LeftOffset;
		LeftOffset = 0.f;
	}

	IKLeftOffset = FMath::FInterpTo(IKLeftOffset, LeftOffset, DeltaSeconds, IKInterpSpeed);
	IKRightOffset = FMath::FInterpTo(IKRightOffset, RightOffset, DeltaSeconds, IKInterpSpeed);
	BodyOffset = FMath::FInterpTo(BodyOffset, BOffsetTmp, DeltaSeconds, IKInterpSpeed);
}

void APlayerCharacter::DrawDebug(const FColor &color, const FVector loc)
{
	DrawDebugCrosshairs(GetWorld(), loc, FRotator(0, 0, 0), 100.f, color, false, 1.f, 0);
}

float APlayerCharacter::ComputeIKOffset(const FName& SocketName)
{
	FHitResult Hit;
	float Result = 0.f;
	float ActorOffset = (IsCrouching()) ? IKCrouchedOffset : IKStayedOffset;

	FVector SocketLoc = GetMesh()->GetSocketLocation(SocketName); FMatrix x;
	FVector TraceStart(SocketLoc.X, SocketLoc.Y, GetActorLocation().Z);
	FVector TraceEndUp(SocketLoc.X, SocketLoc.Y, GetActorLocation().Z - ActorOffset - IKTraceDist);
	FVector TraceEndDown = TraceEndUp - IKExtendMaxDist * FVector::UpVector; 

	if(Trace(TraceStart, TraceEndUp, Hit)) // trace up from foot to center of the body
	{
		Result = (Hit.Location.Z - SocketLoc.Z) / IKActorScale;
	} 
	else if(Trace(TraceEndUp, TraceEndDown, Hit)) // trace down from leg to the ground
	{
		Result = (Hit.Location.Z - SocketLoc.Z) / IKActorScale;
	}

	return Result;
}

bool APlayerCharacter::Trace(const FVector& Start, const FVector& End, FHitResult& Result)
{
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);

	return UKismetSystemLibrary::LineTraceSingle(GetWorld(), Start, End, TraceType, true,
		TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, Result, true);
}