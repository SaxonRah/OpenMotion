// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "EJointType.h"
#include "EBoneConstraintType.h"
#include "FabrikChain.generated.h"


class UFabrikJoint;
class UFabrikBone;
class UFabrikStructure;

/**
 * 
 */
UCLASS()
class OPENMOTION_API UFabrikChain : public UObject
{
	GENERATED_BODY()
	
public:

	UFabrikChain(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		TArray<UFabrikBone*> Chain;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		float SolveDistanceThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
	 int MaxIterationAttempts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
	 float MinIterationChange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
	 float ChainLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
	 int NumBones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FVector FixedBaseLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
	 bool FixedBaseMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
	EBoneConstraintType BaseboneConstraintType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FVector BaseboneConstraintUV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FVector BaseboneRelativeConstraintUV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
	FVector BaseboneRelativeReferenceConstraintUV;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FVector LastTargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		float ConstraintLineWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FVector LastBaseLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		float CurrentSolveDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		int ConnectedChainNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		int ConnectedBoneNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FVector EmbeddedTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
	bool UseEmbeddedTarget;

	FVector GetBaseLocation();

	FVector CreateMaxVector()
	{
		return FVector(FLT_MAX, FLT_MAX, FLT_MAX);
	}

	float FloatMax()
	{
		return FLT_MAX, FLT_MAX, FLT_MAX;
	}


	void Init(UFabrikChain* InSource);
	UFabrikChain* Init(FName InName);
	void AddBone(UFabrikBone* InBone);

	void AddConsecutiveBone(FVector InDirectionUV, float InLength);
	void AddConsecutiveBone(FVector InDirectionUV, float InLength, FColor InColour);
	
	void AddConsecutiveFreelyRotatingHingedBone(FVector InDirectionUV, float InLength, EJointType InJointType, FVector InHingeRotationAxis);
	void AddConsecutiveFreelyRotatingHingedBoneC(FVector InDirectionUV, float InLength, EJointType InJointType, FVector InHingeRotationAxis, FColor InColour);

	void AddConsecutiveHingedBoneC(FVector InDirectionUV,
		float InLength,
		EJointType InJointType,
		FVector InHingeRotationAxis,
		float InClockwiseDegs,
		float InAnticlockwiseDegs,
		FVector InHingeReferenceAxis,
		FColor InColour);

	void AddConsecutiveHingedBone(FVector InDirectionUV,
		float InLength,
		EJointType InJointType,
		FVector InHingeRotationAxis,
		float InClockwiseDegs,
		float InAnticlockwiseDegs,
		FVector InHingeConstraintReferenceAxis);

	void AddConsecutiveRotorConstrainedBoneC(FVector InBoneDirectionUV, float InBoneLength, float InConstraintAngleDegs, FColor InColor);
	void AddConsecutiveRotorConstrainedBone(FVector InBoneDirectionUV, float InBoneLength, float InConstraintAngleDegs);
	UFabrikBone* GetBone(int InBoneNumber);

	FVector GetEffectorLocation();
	float GetLiveChainLength();
	void RemoveBone(int InBoneNumber);
	void SetRotorBaseboneConstraint(EBoneConstraintType InRotorType, FVector InConstraintAxis, float InAngleDegs);

	void SetHingeBaseboneConstraint(EBoneConstraintType InHingeType, FVector InHingeRotationAxis, float InCwConstraintDegs, float InAcwConstraintDegs, FVector InHingeReferenceAxis);
	void SetFreelyRotatingGlobalHingedBasebone(FVector InHingeRotationAxis);
	void SetFreelyRotatingLocalHingedBasebone(FVector InHingeRotationAxis);
	void SetLocalHingedBasebone(FVector InHingeRotationAxis, float InCwDegs, float InAcwDegs, FVector InHingeReferenceAxis);
	void SetGlobalHingedBasebone(FVector InHingeRotationAxis, float InCwDegs, float InAcwDegs, FVector InHingeReferenceAxis);
	//void SetBaseboneConstraintUV(FVector InConstraintUV);
	// public void setBaseLocation(Vec3f baseLocation) { mFixedBaseLocation = baseLocation; }

	//###########
	void ConnectToStructure(UFabrikStructure* InStructure, int InChainNumber, int InBoneNumber);
	void SetColour(FColor InColour);
	float SolveForEmbeddedTarget();
	float SolveForTarget(FVector InNewTarget);

	// public String toString()
	float SolveIK(FVector InTarget);
	void UpdateChainLength();
	//void UpdateEmbeddedTarget(FVector InNewEmbeddedTarget);
	TArray<UFabrikBone*> CloneIkChain();
};
