// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "UObject/NoExportTypes.h"
#include "EBoneConnectionPoint.h"
#include "FabrikJoint.h"

#include "FabrikBone.generated.h"

/**
UENUM(BlueprintType)
enum class EBoneConnectionPoint : uint8
{
	BCP_Start 	UMETA(DisplayName = "Start"),
	BCP_End 	UMETA(DisplayName = "End")
};
*/

UCLASS()
class OPENMOTION_API UFabrikBone : public UObject
{
	GENERATED_BODY()

public:
	UFabrikBone(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		EBoneConnectionPoint BoneConnectionPoint;

	//# TODO
//######## FabrikJoint3D mJoint = new FabrikJoint3D();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		UFabrikJoint* Joint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FVector StartLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FVector EndLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		float Length;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FColor Color;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		float LineWidth;

	/**
	* Return the live (i.e. live calculated) length of this bone from its current start and end locations.
	*
	* @return	The 'live' calculated distance between the start and end locations of this bone.
	*/

	UFabrikBone* Init(FVector InStartLocation, FVector InEndLocation);
	UFabrikBone* Init(FVector InStartLocation, FVector InEndLocation, FName InName);
	UFabrikBone* Init(FVector InStartLocation, FVector InDirectionUV, float InLength);
	UFabrikBone* Init(FVector InStartLocation, FVector InDirectionUV, float InLength, FName InName);
	UFabrikBone* Init(FVector InStartLocation, FVector InDirectionUV, float InLength, FColor InColor);
	UFabrikBone* Init(UFabrikBone* Source); // Clone

	float LiveLength();
	TArray<float> GetStartLocationAsArray();
	TArray<float> GetEndLocationAsArray();

	// TODO: public JointType getJointType()	{ return mJoint.getJointType(); }
	// public void setHingeJointClockwiseConstraintDegs(float angleDegs) {	mJoint.setHingeJointClockwiseConstraintDegs(angleDegs);	}
	// public float getHingeJointClockwiseConstraintDegs()	{ return mJoint.getHingeClockwiseConstraintDegs(); }
	// public void setHingeJointAnticlockwiseConstraintDegs(float angleDegs) { mJoint.setHingeJointAnticlockwiseConstraintDegs(angleDegs);
	// public float getHingeJointAnticlockwiseConstraintDegs() { return mJoint.getHingeAnticlockwiseConstraintDegs(); }
	// setBallJointConstraintDegs
	// 

	//void SetLineWidth(float InLineWidth);
	FVector GetDirectionUV()
	{
		//public static Vec3f getDirectionUV(Vec3f v1, Vec3f v2) { return v2.minus(v1).normalise(); }
		FVector Res = (EndLocation - StartLocation);// Vec3f.getDirectionUV(mStartLocation, mEndLocation);
		Res.Normalize();
		return Res;
	}

	FString GetDebugMsg();
};
