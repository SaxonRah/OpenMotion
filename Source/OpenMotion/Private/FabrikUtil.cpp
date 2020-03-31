// Fill out your copyright notice in the Description page of Project Settings.

#include "FabrikUtil.h"
#include "FabrikMat3f.h"

UFabrikUtil::UFabrikUtil(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer) {}

const FColor UFabrikUtil::RED(255 * 1.0f, 255 * 0.0f, 255 * 0.0f); // 1.0f, 0.0f, 0.0f, 1.0f
const FColor UFabrikUtil::GREEN(255 * 0.0f, 255 * 1.0f, 255 * 0.0f);
const FColor UFabrikUtil::BLUE(255 * 0.0f, 255 * 0.0f, 255 * 1.0f);
const FColor UFabrikUtil::MID_RED(255 * 0.6f, 255 * 0.0f, 255 * 0.0f);
const FColor UFabrikUtil::MID_GREEN(255 * 0.0f, 255 * 0.6f, 255 * 0.0f);
const FColor UFabrikUtil::MID_BLUE(255 * 0.0f, 255 * 0.0f, 255 * 0.6f);
const FColor UFabrikUtil::BLACK(255 * 0.0f, 255 * 0.0f, 255 * 0.0f);
const FColor UFabrikUtil::GREY(255 * 0.5f, 255 * 0.5f, 255 * 0.5f);
const FColor UFabrikUtil::WHITE(255 * 1.0f, 255 * 1.0f, 255 * 1.0f);
const FColor UFabrikUtil::YELLOW(255 * 1.0f, 255 * 1.0f, 255 * 0.0f);
const FColor UFabrikUtil::CYAN(255 * 0.0f, 255 * 1.0f, 255 * 1.0f);
const FColor UFabrikUtil::MAGENTA(255 * 1.0f, 255 * 0.0f, 255 * 1.0f);

const FVector UFabrikUtil::X_AXIS(1, 0, 0);
const FVector UFabrikUtil::Y_AXIS(0, 1, 0);
const FVector UFabrikUtil::Z_AXIS(0, 0, 1);


FVector UFabrikUtil::RotateAboutAxisRads(FVector InSource, float InAngleRads, FVector InRotationAxis)
{
	
	//Mat3f rotationMatrix = new Mat3f();
	UFabrikMat3f* RotationMatrix = NewObject<UFabrikMat3f>();

	float sinTheta = (float)FMath::Sin(InAngleRads);
	float cosTheta = (float)FMath::Cos(InAngleRads);
	float oneMinusCosTheta = 1.0f - cosTheta;

	// It's quicker to pre-calc these and reuse than calculate x * y, then y * x later (same thing).
	float xyOne = InRotationAxis.X * InRotationAxis.Y * oneMinusCosTheta;
	float xzOne = InRotationAxis.X * InRotationAxis.Z * oneMinusCosTheta;
	float yzOne = InRotationAxis.Y * InRotationAxis.Z * oneMinusCosTheta;

	// Calculate rotated x-axis
	RotationMatrix->m00 = InRotationAxis.X * InRotationAxis.X * oneMinusCosTheta + cosTheta;
	RotationMatrix->m01 = xyOne + InRotationAxis.Z * sinTheta;
	RotationMatrix->m02 = xzOne - InRotationAxis.Y * sinTheta;

	// Calculate rotated y-axis
	RotationMatrix->m10 = xyOne - InRotationAxis.Z * sinTheta;
	RotationMatrix->m11 = InRotationAxis.Y * InRotationAxis.Y * oneMinusCosTheta + cosTheta;
	RotationMatrix->m12 = yzOne + InRotationAxis.X * sinTheta;

	// Calculate rotated z-axis
	RotationMatrix->m20 = xzOne + InRotationAxis.Y * sinTheta;
	RotationMatrix->m21 = yzOne - InRotationAxis.X * sinTheta;
	RotationMatrix->m22 = InRotationAxis.Z * InRotationAxis.Z * oneMinusCosTheta + cosTheta;

	// Multiply the source by the rotation matrix we just created to perform the rotation
	return RotationMatrix->Times(InSource);
}


FVector UFabrikUtil::GetAngleLimitedUnitVectorDegs(FVector InVecToLimit, FVector InVecBaseline, float InAngleLimitDegs)
{
	// Get the angle between the two vectors
	// Note: This will ALWAYS be a positive value between 0 and 180 degrees.
	float AngleBetweenVectorsDegs = GetAngleBetweenDegs(InVecBaseline, InVecToLimit);

	if (AngleBetweenVectorsDegs > InAngleLimitDegs)
	{
		// The axis which we need to rotate around is the one perpendicular to the two vectors - so we're
		// rotating around the vector which is the cross-product of our two vectors.
		// Note: We do not have to worry about both vectors being the same or pointing in opposite directions
		// because if they bones are the same direction they will not have an angle greater than the angle limit,
		// and if they point opposite directions we will approach but not quite reach the precise max angle
		// limit of 180.0f (I believe).
		InVecBaseline.Normalize();
		InVecToLimit.Normalize();

		//FVector correctionAxis = FVector::CrossProduct(vecBaseline.normalised(), vecToLimit.normalised()).normalise();
		FVector CorrectionAxis = FVector::CrossProduct(InVecBaseline, InVecToLimit);// .normalise();
		CorrectionAxis.Normalize();
		//CorrectionAxis = FVector(0, 1, 0); //HACK
		// Our new vector is the baseline vector rotated by the max allowable angle about the correction axis

		FVector Res = RotateAboutAxisDegs(InVecBaseline, InAngleLimitDegs, CorrectionAxis);// .normalised();
		Res.Normalize();
		return Res;
	}
	else // Angle not greater than limit? Just return a normalised version of the vecToLimit
	{
		// This may already BE normalised, but we have no way of knowing without calcing the length, so best be safe and normalise.
		// TODO: If performance is an issue, then I could get the length, and if it's not approx. 1.0f THEN normalise otherwise just return as is.
		InVecToLimit.Normalize();
		return InVecToLimit;// .normalised();
	}
}



