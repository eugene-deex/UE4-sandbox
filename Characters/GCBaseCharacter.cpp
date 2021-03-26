// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"


#include "../Components/GCBaseCharacterMovementComponent.h"

#include "Sound/SoundCue.h"


AGCBaseCharacter::AGCBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGCBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	GCBaseCharacterMovementComponent = StaticCast<UGCBaseCharacterMovementComponent*>(GetCharacterMovement());

	// load Jump Sound
	static ConstructorHelpers::FObjectFinder<USoundCue> JumpSoundCueObject(TEXT("SoundCue'/Game/GameCode/Audio/Jump_Cue.Jump_Cue'"));

	if (JumpSoundCueObject.Succeeded())
	{
		Jump_Cue = JumpSoundCueObject.Object;
		JumpAudioComponent = CreateDefaultSubobject<UAudioComponent>(
			TEXT("JumpAudioComponent")
			);
		
		JumpAudioComponent->bAutoActivate = false;
		JumpAudioComponent->SetSound(StaticCast<USoundBase*>(Jump_Cue));
		JumpAudioComponent->SetupAttachment(RootComponent);
	}
}

bool AGCBaseCharacter::IsProning() const
{
	return GCBaseCharacterMovementComponent->IsProning();
}
void AGCBaseCharacter::SetIsProning(bool Value) const
{
	GCBaseCharacterMovementComponent->SetIsProning(Value);
}

bool AGCBaseCharacter::IsCrouching() const
{
	return GCBaseCharacterMovementComponent->IsCrouching();
}

void AGCBaseCharacter::Ragdoll()
{
	if (bIsDead)
	{
		// TODO: restore character's position is not finished yet
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("AGCBaseCharacter::Ragdoll  - alive"));

		// if on ground (check with linetrace down):
		GetBaseCharacterMovementComponent()->SetMovementMode(EMovementMode::MOVE_Walking);
		// Mesh()->GetAnimInstance()->montagePlay()  <-- get up animation (AnimMontage'/Game/AdvancedLocomotionV4/CharacterAssets/MannequinSkeleton/AnimationExamples/Actions/ALS_CLF_GetUp_Front_Montage_Default'
		
		// if not on ground:
		//GetBaseCharacterMovementComponent()->SetMovementMode(EMovementMode::MOVE_Falling);
		// set CharacterMovement->Velocity = get last ragdoll velocity

		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetMesh()->SetAllBodiesSimulatePhysics(false);
		bIsDead = false;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("AGCBaseCharacter::Ragdoll  - dead"));
		GetBaseCharacterMovementComponent()->SetMovementMode(EMovementMode::MOVE_None);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetAllBodiesBelowSimulatePhysics(FName("pelvis"), true, true);
		bIsDead = true;
	}
}

void AGCBaseCharacter::Jump()
{
	if (GetBaseCharacterMovementComponent()->IsOutOfStamina())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("no stamina to jump"));
		return;
	}

	if (!GCBaseCharacterMovementComponent->IsFalling())
	{
		JumpAudioComponent->Play();
		//JumpAudioComponent->SetFloatParameter(FName("pitch"), 5500.f);
	}
	
	Super::Jump();
}

void AGCBaseCharacter::Prone2Crouch()
{
	// player is proning now
	// GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::White, TEXT("PRONE TO CROUCH"));

	// change prone state to stand state

	// default capsule, R=34, 90*2
	UCapsuleComponent* DefaultCapsuleComponent = GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent();
	// current (prone) capsule radius, half-height: 34, 30
	float CurrentRadius, CurrentHalfHeight;
	GetCapsuleComponent()->GetScaledCapsuleSize(CurrentRadius, CurrentHalfHeight);

	float CrouchedRadius = DefaultCapsuleComponent->GetUnscaledCapsuleRadius();
	float CrouchedHalfHeight = GCBaseCharacterMovementComponent->CrouchedHalfHeight;// DefaultCapsuleComponent->GetUnscaledCapsuleHalfHeight();

	// Test first
	// find new location for full size pawn shape
	const FVector PawnLocation = GetCharacterMovement()->UpdatedComponent->GetComponentLocation();
	const float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
	float MinFloorDistHalf = UCharacterMovementComponent::MIN_FLOOR_DIST / 2.f;

	float newZ = PawnLocation.Z - CurrentHalfHeight
		+ (CrouchedHalfHeight * GetActorScale3D().Z) + SweepInflation + MinFloorDistHalf;

	const FVector NewLoc = FVector(
		PawnLocation.X,
		PawnLocation.Y,
		newZ);

	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, this);
	FCollisionResponseParams ResponseParam;
	ECollisionChannel Channel = GetCharacterMovement()->UpdatedComponent->GetCollisionObjectType();

	bool bEncroached = GetWorld()->OverlapBlockingTestByChannel(
		NewLoc, // position to check overlap
		FQuat::Identity, // no rotation
		Channel, // Collision channel: ECC_PAWN
		FCollisionShape::MakeCapsule(CrouchedRadius, CrouchedHalfHeight),
		CapsuleParams, // tag "CrouchTrace"
		ResponseParam);

	if (bEncroached) // if there is no space to crouch up - stop
	{
		UE_LOG(LogTemp, Warning, TEXT("GCBaseCharacter::UnProne - no space to unprone"))
			return;
	}

	// test is ok, let's stand up
	GCBaseCharacterMovementComponent->SetIsProning(false);
	bIsCrouched = true;

	//float HeightAdjust = DefaultHalfHeight - CurrentHalfHeight; // 90-30=60
	float HeightAdjust = GCBaseCharacterMovementComponent->CrouchedHalfHeight - CurrentHalfHeight; // 60-30=30
	float ScaledHeightAdjust = HeightAdjust * GetCapsuleComponent()->GetShapeScale();

	// Restore crouch-size capsule
	PlayerHeightAdjust(HeightAdjust, ScaledHeightAdjust); // move mesh & eyes
	GetCapsuleComponent()->SetCapsuleSize(CrouchedRadius, CrouchedHalfHeight, true);
	OnEndProne(HeightAdjust, ScaledHeightAdjust);
}

void AGCBaseCharacter::Prone()
{
	// Player is uncrouched and staying now
	bIsCrouched = false;
	SetIsProning(true);

	// change capsule size
	GetCapsuleComponent()->SetCapsuleSize(
		GCBaseCharacterMovementComponent->ProneCapsuleRadius,
		GCBaseCharacterMovementComponent->ProneCapsuleHalfHeight,
		true);

	// adjust mesh location (it has Z=-60 now)
	ACharacter* DefaultCharacter = GetClass()->GetDefaultObject<ACharacter>();

	const float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - GCBaseCharacterMovementComponent->ProneCapsuleHalfHeight; // 60
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * GetCapsuleComponent()->GetShapeScale(); // 60 * 1

	PlayerHeightAdjust(HalfHeightAdjust, ScaledHalfHeightAdjust); 
	OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void AGCBaseCharacter::OnStartCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
	Super::OnStartCrouch(HeightAdjust, ScaledHeightAdjust);
}

void AGCBaseCharacter::PlayerHeightAdjust(float HeightAdjust, float ScaledHeightAdjust)
{
	RecalculateBaseEyeHeight();
	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	USkeletalMeshComponent* MeshDefault = DefaultChar->GetMesh();

	if (GetMesh() && MeshDefault)
	{
		FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = MeshDefault->GetRelativeLocation().Z + HeightAdjust;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->GetBaseTranslationOffset().Z + HeightAdjust;
	}

	K2_OnStartCrouch(HeightAdjust, ScaledHeightAdjust);
}

void AGCBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	StaminaValue = StaminaMax;
}

bool AGCBaseCharacter::CanJumpInternal_Implementation() const
{
	// forbid to jump if not enough space even to stand up

	// FULL CAPSULE - take CDO (Class Default Object) of type ACharacter & and get full-size capsule from it
	ACharacter* DefaultCharacter = GetClass()->GetDefaultObject<ACharacter>();
	const FCollisionShape StandingCapsuleShape = DefaultCharacter->GetCapsuleComponent()->GetCollisionShape();

	// SMALL CAPSULE - get size of current small capsule (in sitting position)
	float PawnRadius, PawnHalfHeight;
	GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight); // radius - 34; half-height - 60; height - 120
	
	// COUNT NEW LOCATION - New location of full size pawn shape
	const FVector PawnLocation = GetCharacterMovement()->UpdatedComponent->GetComponentLocation(); // 369, -90, 82
	const float SweepInflation = KINDA_SMALL_NUMBER * 10.f; // 0.00099999
	float MinFloorDistHalf = UCharacterMovementComponent::MIN_FLOOR_DIST / 2.f; // 1.9f / 2 = 0.95
	
	float newZ = PawnLocation.Z - PawnHalfHeight + StandingCapsuleShape.Capsule.HalfHeight;
	newZ += SweepInflation + MinFloorDistHalf;

	const FVector NewLoc = FVector(
			PawnLocation.X, 
			PawnLocation.Y, 
			newZ); 

	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, this); 
	FCollisionResponseParams ResponseParam;
	ECollisionChannel Channel = GetCharacterMovement()->UpdatedComponent->GetCollisionObjectType(); 

	bool bEncroached = GetWorld()->OverlapBlockingTestByChannel(
		NewLoc, // position to check overlap
		FQuat::Identity, // no rotation
		Channel, // Collision channel: ECC_PAWN
		StandingCapsuleShape, 
		CapsuleParams, // tag "CrouchTrace"
		ResponseParam);

	// if player is sitting & there is enough space to stand up - you can jump
	if (bIsCrouched && !bEncroached) 
	{
		return true;
	}
	 
	return Super::CanJumpInternal_Implementation();
}

void AGCBaseCharacter::OnJumped_Implementation()
{
	if (bIsCrouched) 
	{
		UnCrouch();
	}
}

void AGCBaseCharacter::StartSprint()
{
	bIsSprintRequested = true;
}

void AGCBaseCharacter::StopSprint()
{
	bIsSprintRequested = false;
}

void AGCBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
		
	GEngine->AddOnScreenDebugMessage(14, 2.f,
		FColor::Yellow,
		FString::Printf(TEXT("Sprint: %s | Crouch: %s | Prone: %s"), 
			GCBaseCharacterMovementComponent->IsSprinting()? TEXT("YES") : TEXT("-"),
			GCBaseCharacterMovementComponent->IsCrouching() ? TEXT("YES") : TEXT("-"),
			GCBaseCharacterMovementComponent->IsProning()? TEXT("YES") : TEXT("-")
		)
	);

	TryChangeSprintState();

	// Stamina
	if (GCBaseCharacterMovementComponent->IsSprinting())
	{
		StaminaValue -= StaminaSprintConsumptionVelocity * DeltaTime;
	}
	else {
		StaminaValue += StaminaRestoreVelocity * DeltaTime;
	}

	StaminaValue = FMath::Clamp(StaminaValue, 0.f, StaminaMax); // limit value between 0 & max

	if (FMath::IsNearlyZero(StaminaValue))
	{
		GCBaseCharacterMovementComponent->SetIsOutOfStamina(true);
	}
	else if(StaminaValue == StaminaMax)
	{
		GCBaseCharacterMovementComponent->SetIsOutOfStamina(false);
	}
}

void AGCBaseCharacter::OnSprintStart_Implementation()
{
	//UE_LOG(LogTemp, Log, TEXT("OnSprintStart_Implementation"));
}

void AGCBaseCharacter::OnSprintStop_Implementation()
{
	//UE_LOG(LogTemp, Warning, TEXT("OnSprintStop_Implementation"));
}

void AGCBaseCharacter::TryChangeSprintState()
{
	if (bIsSprintRequested && !GCBaseCharacterMovementComponent->IsSprinting() && CanSprint())
	{
		UE_LOG(LogTemp, Log, TEXT("Start sprint"));
		GCBaseCharacterMovementComponent->StartSprint();
		OnSprintStart();

		if (bIsCrouched)
		{
			UnCrouch();
		}
	}

	if (!bIsSprintRequested && GCBaseCharacterMovementComponent->IsSprinting())
	{
		GCBaseCharacterMovementComponent->StopSprint();
		OnSprintStop();
	}
}

bool AGCBaseCharacter::CanSprint()
{
	if (GCBaseCharacterMovementComponent->IsProning())
	{
		return false;
	}

	return true;
}

void AGCBaseCharacter::OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	UE_LOG(LogTemp, Log, TEXT("onStartProne"))
	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());

	USkeletalMeshComponent* MeshDefault = DefaultChar->GetMesh();
	FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();

	MeshRelativeLocation.X -= 50.f;
	BaseTranslationOffset.X = MeshRelativeLocation.X;
}

void AGCBaseCharacter::OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	UE_LOG(LogTemp, Log, TEXT("onEndProne"))
	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());

	USkeletalMeshComponent* MeshDefault = DefaultChar->GetMesh();
	FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();

	MeshRelativeLocation.X += 50.f;
	BaseTranslationOffset.X = MeshRelativeLocation.X;
}