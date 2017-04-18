// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Pawn.h"
#include "SpiderSensor/sensorData.h"
#include "QuadcopterPawn.generated.h"

UCLASS(config=Game)
class AQuadcopterPawn : public APawn
{
	GENERATED_BODY()

	/** StaticMesh component that will be the visuals for our flying pawn */
	//UPROPERTY(Category = Mesh, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	//class AActor* tSound;
	//class APawn* LeoSound;

	/** StaticMesh component that will be the visuals for our flying pawn */
	UPROPERTY(Category = Mesh, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent *Mesh;

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = Camera, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	/** Camera component that will be our viewpoint */
	UPROPERTY(Category = Camera, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UPROPERTY(Category = Camera, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* SphereComponent;
public:
	AQuadcopterPawn(const FObjectInitializer& ObjectInitializer);

	// Begin AActor overrides
	UFUNCTION(BlueprintCallable, Category = "Example Nodes")
	virtual void Tick(float DeltaSeconds) override;

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	// End AActor overrides

	virtual void BeginDestroy();
protected:
	
	// Begin APawn overrides
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override; // Allows binding actions/axes to functions
	// End APawn overrides
	
	/** Bound to the thrust axis */
	void ThrustInput(float Val);
	
	/** Bound to the vertical axis */
	void MoveUpInput(float Val);

	//void PlaySound();

	/** Bound to the horizontal axis */
	void MoveRightInput(float Val);

	/** Bound to the horizontal axis */
	void RiseInput(float Val);

	/** Turn */
	void TurnInput(float Val);



private:

	/** How quickly up speed changes */
	UPROPERTY(Category = Plane, EditAnywhere)
	float UpAcceleration;

	/** How quickly up speed changes */
	float UpSpeed;

	/** How quickly forward speed changes */
	UPROPERTY(Category=Plane, EditAnywhere)
	float Acceleration;
	

	/** How quickly pawn can steer */
	UPROPERTY(Category=Plane, EditAnywhere)
	float TurnSpeed;

	/** Max forward speed */
	UPROPERTY(Category = Pitch, EditAnywhere)
	float MaxSpeed;

	/** Min forward speed */
	UPROPERTY(Category=Yaw, EditAnywhere)
	float MinSpeed;

	/** Current forward speed */
	float CurrentForwardSpeed;

	// The Quadcopter's Current Rotation
	FRotator CurRotation;

	// Move Speed
	FVector CurVelocity;

	// Spider Sensor
	hid_device *handle;
	sensorData *sensor;
	float normalized_data[64];

public:
	/** Returns Mesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetPlaneMesh() const { return Mesh; }
	/** Returns SpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }
};
