// Fill out your copyright notice in the Description page of Project Settings.

#include "FabrikJoint.h"
#include "FabrikUtil.h"

#include "OpenMotion.h"

float UFabrikJoint::MIN_CONSTRAINT_ANGLE_DEGS = 0.0f;
float UFabrikJoint::MAX_CONSTRAINT_ANGLE_DEGS = 180.0f;

UFabrikJoint::UFabrikJoint(const FObjectInitializer& ObjectInitializer)
{
	RotorConstraintDegs = MAX_CONSTRAINT_ANGLE_DEGS;

	HingeClockwiseConstraintDegs = MAX_CONSTRAINT_ANGLE_DEGS;

	HingeAnticlockwiseConstraintDegs = MAX_CONSTRAINT_ANGLE_DEGS;

	//RotationAxisUV;

	//ReferenceAxisUV;// = new Vec3f();
	JointType = EJointType::JT_Ball;
}

UFabrikJoint* UFabrikJoint::Init(UFabrikJoint* Source)
{ 
	RotorConstraintDegs = Source->RotorConstraintDegs;
	HingeClockwiseConstraintDegs = Source->HingeClockwiseConstraintDegs;
	HingeAnticlockwiseConstraintDegs = Source->HingeAnticlockwiseConstraintDegs;
	RotationAxisUV = Source->RotationAxisUV;
	ReferenceAxisUV = Source->ReferenceAxisUV;
	JointType = Source->JointType;
	BoneConnectionPoint = Source->BoneConnectionPoint;
	return this;
}

UFabrikJoint* UFabrikJoint::Clone(UFabrikJoint* Source)
{
	UFabrikJoint* NewJoint = NewObject<UFabrikJoint>();
	NewJoint->Init(Source);
	return NewJoint;
}

void UFabrikJoint::SetAsBallJoint(float InConstraintAngleDegs)
{
	// Throw a RuntimeException if the rotor constraint angle is outside the range 0 to 180 degrees
	UFabrikJoint::ValidateConstraintAngleDegs(InConstraintAngleDegs);

	// Set the rotor constraint angle and the joint type to be BALL.
	RotorConstraintDegs = InConstraintAngleDegs;
	JointType = EJointType::JT_Ball;

}

void UFabrikJoint::SetHinge(EJointType InJointType, FVector InRotationAxis, float InClockwiseConstraintDegs, float InAnticlockwiseConstraintDegs, FVector InReferenceAxis)
{
	// Ensure the reference axis falls within the plane of the rotation axis (i.e. they are perpendicular, so their dot product is zero)		
	float dotP = FVector::DotProduct(InRotationAxis, InReferenceAxis);
	
	if (! UFabrikUtil::ApproximatelyEquals(dotP, 0.0f, 0.01f)) // FMath::Abs(dotP) < 0.01f) //  !Utils.approximatelyEquals(Vec3f.dotProduct(rotationAxis, referenceAxis), 0.0f, 0.01f))
	{
		//float angleDegs = FVector::an Vec3f.getAngleBetweenDegs(rotationAxis, referenceAxis);
		UE_LOG(OpenMotionLog, Fatal, TEXT("The reference axis must be in the plane of the hinge rotation axis - angle between them is currently: TODO"));  //throw new IllegalArgumentException("The reference axis must be in the plane of the hinge rotation axis - angle between them is currently: " + angleDegs);
	}

	// Validate the constraint angles to be within the valid range and the axis isn't zero
	UFabrikJoint::ValidateConstraintAngleDegs(InClockwiseConstraintDegs);
	UFabrikJoint::ValidateConstraintAngleDegs(InAnticlockwiseConstraintDegs);
	UFabrikJoint::ValidateAxis(InRotationAxis);
	UFabrikJoint::ValidateAxis(InReferenceAxis);


	// Set params

	HingeClockwiseConstraintDegs = InClockwiseConstraintDegs;

	HingeAnticlockwiseConstraintDegs = InAnticlockwiseConstraintDegs;

	JointType = InJointType;

	RotationAxisUV = InRotationAxis; // set(rotationAxis.normalised());
	RotationAxisUV.Normalize();

	ReferenceAxisUV = InReferenceAxis;// .normalised());
	ReferenceAxisUV.Normalize();
}

void UFabrikJoint::SetAsGlobalHinge(FVector InGlobalRotationAxis, float InCwConstraintDegs, float InAcwConstraintDegs, FVector InGlobalReferenceAxis)
{

	SetHinge(EJointType::JT_GlobalHinge, InGlobalRotationAxis, InCwConstraintDegs, InAcwConstraintDegs, InGlobalReferenceAxis);
}

void UFabrikJoint::SetAsLocalHinge(FVector InLocalRotationAxis, float InCwConstraintDegs, float InAcwConstraintDegs, FVector InLocalReferenceAxis)
{
	SetHinge(EJointType::JT_LocalHinge, InLocalRotationAxis, InCwConstraintDegs, InAcwConstraintDegs, InLocalReferenceAxis);
}


// public float getHingeClockwiseConstraintDegs()
// public float getHingeAnticlockwiseConstraintDegs()
// public void setBallJointConstraintDegs(float angleDegs)
// public float getBallJointConstraintDegs()
// public void setHingeJointClockwiseConstraintDegs(float angleDegs)
// public void setHingeJointAnticlockwiseConstraintDegs(float angleDegs)
// public void setHingeRotationAxis(Vec3f axis) => mRotationAxisUV
// public Vec3f getHingeReferenceAxis() => mReferenceAxisUV
// public Vec3f getHingeRotationAxis() // mRotationAxisUV
// public String toString()






void UFabrikJoint::ValidateConstraintAngleDegs(float InAngleDegs)
{

	if (InAngleDegs < UFabrikJoint::MIN_CONSTRAINT_ANGLE_DEGS || InAngleDegs > UFabrikJoint::MAX_CONSTRAINT_ANGLE_DEGS)
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("Constraint angles must be within the range"));
		//throw new IllegalArgumentException("Constraint angles must be within the range " + MIN_CONSTRAINT_ANGLE_DEGS + " to " + MAX_CONSTRAINT_ANGLE_DEGS + " inclusive.");
	}

}



void UFabrikJoint::ValidateAxis(FVector InAxis)
{

	if (!(InAxis.Size() > 0.0f))
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("Provided axis is illegal - it has a magnitude of zero."));
	}

}