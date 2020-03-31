// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "EBoneConnectionPoint.h"
#include "EJointType.h"

#include "FabrikJoint.generated.h"


/**
 * 
 */
UCLASS()
class OPENMOTION_API UFabrikJoint : public UObject
{
	GENERATED_BODY()
	
public:
	static float MIN_CONSTRAINT_ANGLE_DEGS;// = 0.0f;
	static float MAX_CONSTRAINT_ANGLE_DEGS;// = 180.0f;
	 
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		float RotorConstraintDegs;// = MAX_CONSTRAINT_ANGLE_DEGS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		float HingeClockwiseConstraintDegs;// = MAX_CONSTRAINT_ANGLE_DEGS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		float HingeAnticlockwiseConstraintDegs;// = MAX_CONSTRAINT_ANGLE_DEGS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FVector RotationAxisUV;// = new Vec3f();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FVector ReferenceAxisUV;// = new Vec3f();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		EJointType JointType;// = JointType.BALL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		EBoneConnectionPoint BoneConnectionPoint;

	UFabrikJoint(const FObjectInitializer& ObjectInitializer);
	
	UFabrikJoint* Init(UFabrikJoint* Source);

	UFabrikJoint* Clone(UFabrikJoint* Source);

	void SetAsBallJoint(float InConstraintAngleDegs);
	void SetHinge(EJointType InJointType, FVector InRotationAxis, float InClockwiseConstraintDegs, float InAnticlockwiseConstraintDegs, FVector InReferenceAxis);
	void SetAsGlobalHinge(FVector InGlobalRotationAxis, float InCwConstraintDegs, float InAcwConstraintDegs, FVector InGlobalReferenceAxis);
	void SetAsLocalHinge(FVector InLocalRotationAxis, float InCwConstraintDegs, float InAcwConstraintDegs, FVector InLocalReferenceAxis);
	
	static void ValidateConstraintAngleDegs(float InAngleDegs);
	static void ValidateAxis(FVector InAxis);
};
