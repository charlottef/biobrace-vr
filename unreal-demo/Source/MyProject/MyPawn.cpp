// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Quadcopter.h"
#include "QuadcopterPawn.h"


AQuadcopterPawn::AQuadcopterPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> Mesh;
		FConstructorStatics()
			: Mesh(TEXT("/Game/QuadCopter/Low_quadCopter.Low_quadCopter"))
		{
		}

	};

	static FConstructorStatics ConstructorStatics;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SceneComponent1"));
	SphereComponent->InitSphereRadius(100.0f);
	SphereComponent->SetSimulatePhysics(true);
	SphereComponent->SetEnableGravity(false);
	SphereComponent->SetLinearDamping(.7f);
	SphereComponent->SetAngularDamping(.99f);
	SphereComponent->SetAllPhysicsRotation(FRotator::ZeroRotator);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RootComponent = SphereComponent;

	// Create static mesh component
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh0"));
	Mesh->SetStaticMesh(ConstructorStatics.Mesh.Get());
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->AttachTo(RootComponent);
	
	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->AttachTo(RootComponent);
	SpringArm->TargetArmLength = 500.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(0.f,0.f,60.f);
	SpringArm->RelativeRotation = FRotator(-20.f, 0.f, 0.f);
	SpringArm->bEnableCameraLag = false;
	SpringArm->CameraLagSpeed = 15.f;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller
	//Camera->AddLocalTransform(FTransform(FVector(-1000.f, 0.f, 0.f)));

	// Set handling parameters
	Acceleration = 1000.f;
	TurnSpeed = 10.f;
	MaxSpeed = 5000.f;
	MinSpeed = -5000.f;
	CurrentForwardSpeed = 0.f;
	UpSpeed = 0.f;
	UpAcceleration = 300.0f;

	// Spider Sensor Init
	handle = hid_open(0x4d8, 0x3f, NULL); // create handle to open device

	sensor = new sensorData();
	for (int i = 0; i < 64; i++) {
		normalized_data[i] = 0;
	}
}

void AQuadcopterPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// [0] - [2] accel & [3] - [5] gyro
	unsigned char buf[65];
	int sensor_data[64];

	sensor->getData(buf, sensor_data, normalized_data, handle); // get normalized data
	TurnSpeed -= normalized_data[5] * 40.0f;

	//if (handle)
	//{
	//	sensor->detectGesture(normalized_data); // detect the gesture
	//}

	// Clamp Rotations
	CurRotation.Pitch = FMath::ClampAngle(CurRotation.Pitch, -45.f, 45.f);
	CurRotation.Roll = FMath::ClampAngle(CurRotation.Roll, -45.f, 45.f);
	CurRotation.Yaw = TurnSpeed;
	//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, FString::Printf(TEXT("Accel: (%f, %f, %f) Gyro: (%f, %f, %f)"), normalized_data[0], normalized_data[1], normalized_data[2], normalized_data[3], normalized_data[4], normalized_data[5]));
	Mesh->SetRelativeRotation(CurRotation);

	//SprinArm rotation
	SpringArm->SetRelativeRotation(FRotator(0.f,CurRotation.Yaw , 0.f));

	// Clamp Velocity
	MoveUpInput((normalized_data[0]*-1.f) * 5);
	MoveRightInput(normalized_data[1] * 5);

	((normalized_data[2]-0.24) < -0.45) ? (normalized_data[2] = 0.24) : (normalized_data[2] -=0.24);
 	RiseInput((normalized_data[2] )* 3);

	CurVelocity *= 0.98;

	// Rise Upwards according to Mesh Rotation
	FRotator YCurRot = FRotator(0,90+CurRotation.Yaw, 0);
	FVector LocalMove = YCurRot.Quaternion() * CurVelocity * 10000.f;
	SphereComponent->AddForce(LocalMove);
	SphereComponent->SetAllPhysicsRotation(FRotator::ZeroRotator);
	// Rotate plane
	//AddRelativeRotation(DeltaRotation);

}

void AQuadcopterPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Lolwat"));
	// Set velocity to zero upon collision
	CurVelocity = FVector::ZeroVector;
}

void AQuadcopterPawn::BeginDestroy()
{
	Super::BeginDestroy();
	hid_close(handle);
}


void AQuadcopterPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	// Bind our control axis' to callback functions
	InputComponent->BindAxis("Thrust", this, &AQuadcopterPawn::ThrustInput);
	InputComponent->BindAxis("MoveUp", this, &AQuadcopterPawn::MoveUpInput);
	InputComponent->BindAxis("MoveRight", this, &AQuadcopterPawn::MoveRightInput);
	InputComponent->BindAxis("Rise", this, &AQuadcopterPawn::RiseInput);
	InputComponent->BindAxis("Turn", this, &AQuadcopterPawn::TurnInput);
}

void AQuadcopterPawn::ThrustInput(float Val)
{
	// Is there no input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);
	// If input is not held down, reduce speed
	float CurrentAcc = (bHasInput == false && CurrentForwardSpeed != 0) ? 
		((CurrentForwardSpeed < 0) ? (0.5f * Acceleration) : (- 0.5f * Acceleration)) : (Val * Acceleration);
	
	// Calculate new speed
	float NewForwardSpeed = CurrentForwardSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
	// Clamp between MinSpeed and MaxSpeed
	CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
	
}

/*void AQuadcopterPawn::PlaySound()
{
	LeoSound->PawnMakeNoise(1.0, FVector::ZeroVector, true, tSound);
}*/

void AQuadcopterPawn::MoveUpInput(float Val)
{
	CurRotation.Pitch = FMath::FInterpTo(CurRotation.Pitch, FMath::Sign(Val) * 30, GetWorld()->GetDeltaSeconds(), 1.f);
	CurVelocity.Y +=  Val * 300 * GetWorld()->GetDeltaSeconds();
	//PlaySound();
}

void AQuadcopterPawn::MoveRightInput(float Val)
{
	CurRotation.Roll = FMath::FInterpTo(CurRotation.Roll, FMath::Sign(Val) * 45, GetWorld()->GetDeltaSeconds(), 1.f);
	CurVelocity.X += Val * 300 * GetWorld()->GetDeltaSeconds();
}


void AQuadcopterPawn::RiseInput(float Val)
{
	CurVelocity.Z += Val * 300 * GetWorld()->GetDeltaSeconds();
}

void AQuadcopterPawn::TurnInput(float Val)
{
	TurnSpeed += (Val * 1.f);
}

