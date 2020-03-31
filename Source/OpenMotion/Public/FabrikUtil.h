// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"

#include "OpenMotion.h"

#include "FabrikUtil.generated.h"

// https://wiki.unrealengine.com/Static_Function_Libraries,_Your_Own_Version_of_UE4_C%2B%2B,_No_Engine_Compile_Times
UCLASS()
class UFabrikUtil : public UObject
{
	GENERATED_UCLASS_BODY()

		// static FORCEINLINE FVector VectorGenPerpendicularVectorQuick(FVector U)
		//FORCEINLNE function
		static FVector VectorGenPerpendicularVectorQuick(FVector U)
	{
		FVector Perp;
		
		if (FMath::Abs(U.Y) < 0.99f)
		{
			Perp =  FVector(-U.Z, 0.0f, U.X); // cross(u, UP)
		}
		else
		{
			Perp = FVector(0.0f, U.Z, -U.Y); // cross(u, RIGHT)
		}

		Perp.Normalize();
		return Perp;
	}

	static float ColorClamp( float InComponentValue)
	{
		float MIN_COMPONENT_VALUE = 0.0f;
		float MAX_COMPONENT_VALUE = 1.0f;
		if (InComponentValue > MAX_COMPONENT_VALUE) { return MAX_COMPONENT_VALUE; }
		else if (InComponentValue < MIN_COMPONENT_VALUE) { return MIN_COMPONENT_VALUE; }
		else { return InComponentValue; }
	}

	static uint8 ColorClamp8(float InComponentValue)
	{
		float C = ColorClamp(InComponentValue);
		return (uint8)(C * 255);
	}

	static  FColor AddRGB(FColor InColor, float InRed, float InGreen, float InBlue)
	{
		FColor C;
		C.R = ColorClamp8((InColor.R / 255.0f) + InRed);
		C.G = ColorClamp8((InColor.G / 255.0f) + InGreen);
		C.B = ColorClamp8((InColor.B / 255.0f) + InBlue);
		return C;
	}

	static FColor SubtractRGB(FColor InColor, float InRed, float InGreen, float InBlue)
	{
		FColor C;
		C.R = ColorClamp8((InColor.R / 255.0f) - InRed);
		C.G = ColorClamp8((InColor.G / 255.0f) - InGreen);
		C.B = ColorClamp8((InColor.B / 255.0f) - InBlue);
		return C;
	}


	static  FColor Lighten(FColor InColor, float InAmount) { return AddRGB(InColor, InAmount, InAmount, InAmount); }
	static  FColor Darken(FColor InColor, float InAmount) { return SubtractRGB(InColor, InAmount, InAmount, InAmount); }
	static  FColor Color(float InRed, float InGreen, float InBlue) { return FColor(ColorClamp8(InRed), ColorClamp8(InGreen), ColorClamp8(InBlue)); }
	
	static const FColor RED;
	static const FColor GREEN;
	static const FColor BLUE;
	static const FColor MID_RED;
	static const FColor MID_GREEN;
	static const FColor MID_BLUE;
	static const FColor BLACK;
	static const FColor GREY;
	static const FColor WHITE;
	static const FColor YELLOW;
	static const FColor CYAN;
	static const FColor MAGENTA;

	static const FVector X_AXIS;
	static const FVector Y_AXIS;
	static const FVector Z_AXIS;

	static void ValidateDirectionUV(FVector InDirectionUV)
	{
		if (!(InDirectionUV.Size() > 0.0f))
		{
			UE_LOG(OpenMotionLog, Fatal, TEXT("FVector direction unit vector cannot be zero."));
		}
	}

	static void ValidateLength(float InLength)
	{
		// Ensure that the magnitude of this direction unit vector is not zero
		if (InLength < 0.0f)
		{
			UE_LOG(OpenMotionLog, Fatal, TEXT("Length must be a greater than or equal to zero."));
		}
	}

	static  bool  VectorPerpendicular(FVector InA, FVector InB)
	{
		return UFabrikUtil::ApproximatelyEquals(FVector::DotProduct(InA, InB), 0.0f, 0.01f) ? true : false;
	}

	static bool ApproximatelyEquals(float InA, float InB, float InTolerance)
	{
		return (FMath::Abs(InA - InB) <= InTolerance) ? true : false;
	}
	
	/**
	static FORCEINLINE FVector VectorGenPerpendicularVectorQuick(FVector InU)
	{
		FVector Perp;

		if (FMath::Abs(InU.Y) < 0.99f)
		{
			Perp = FVector(-InU.Z, 0.0f, InU.X); // cross(u, UP)
		}
		else
		{
			Perp = FVector(0.0f, InU.Z, -InU.Y); // cross(u, RIGHT)
		}

		Perp.Normalize();
		return Perp; //  perp.normalise();

	}*/


	static bool VectorApproximatelyEquals(FVector InSrc, FVector InV, float InTolerance)
	{
		if (InTolerance < 0.0f)
		{
			UE_LOG(OpenMotionLog, Fatal, TEXT("Equality threshold must be greater than or equal to 0.0f"));
		}

		// Get the absolute differences between the components
		float xDiff = FMath::Abs(InSrc.X - InV.X);
		float yDiff = FMath::Abs(InSrc.Y - InV.Y);
		float zDiff = FMath::Abs(InSrc.Z - InV.Z);

		// Return true or false
		return (xDiff < InTolerance && yDiff < InTolerance && zDiff < InTolerance);
	}

	static  FVector VectorNegated(FVector InV) { return FVector(-InV.X, -InV.Y, -InV.Z); }


	static  FVector ProjectOntoPlane(FVector Inv, FVector InPlaneNormal)
	{
		if (!(InPlaneNormal.Size() > 0.0f)) 
		{ 
			UE_LOG(OpenMotionLog, Fatal, TEXT("Plane normal cannot be a zero vector.")); 
		}

		// Projection of vector b onto plane with normal n is defined as: b - ( b.n / ( |n| squared )) * n
		// Note: |n| is length or magnitude of the vector n, NOT its (component-wise) absolute value		
		FVector B = Inv;// .normalised();
		B.Normalize();

		FVector N = InPlaneNormal;// .normalised();
		N.Normalize();

		FVector Res = B - (N * (FVector::DotProduct(B, InPlaneNormal)));// b.minus(n.times(Vec3f.dotProduct(b, planeNormal))).normalise();
		Res.Normalize();
		return Res;

		/** IMPORTANT: We have to be careful here - even code like the below (where dotProduct uses normalised
		*             versions of 'this' and planeNormal is off by enough to make the IK solutions oscillate:
		*
		*             return this.minus( planeNormal.times( Vec3f.dotProduct(this, planeNormal) ) ).normalised();
		*
		*/

		// Note: For non-normalised plane vectors we can use:
		// float planeNormalLength = planeNormal.length();
		// return b.minus( n.times( Vec3f.dotProduct(b, n) / (planeNormalLength * planeNormalLength) ).normalised();
	}

	static float Sign(float InValue)
	{
		if (InValue >= 0.0f) { return 1.0f; } // Implied else...		
		return -1.0f;
	}


	static  float GetAngleBetweenRads(FVector InV1, FVector InV2)
	{
		// Note: a and b are normalised within the dotProduct method.
		// return (float)Math.acos( Vec3f.dotProduct(v1,  v2) );
		return (float)FMath::Acos(FVector::DotProduct(InV1, InV2));
	}

	static float GetAngleBetweenDegs(FVector InV1, FVector InV2)
	{ 
		return FMath::RadiansToDegrees(GetAngleBetweenRads(InV1, InV2));
	}

	static  float GetSignedAngleBetweenDegs(FVector InReferenceVector, FVector InOtherVector, FVector InNormalVector)
	{
		float UnsignedAngle = GetAngleBetweenDegs(InReferenceVector, InOtherVector);
		float SignV = Sign(FVector::DotProduct(FVector::CrossProduct(InReferenceVector, InOtherVector), InNormalVector));
		return UnsignedAngle * SignV;
	}

	static  FVector RotateAboutAxisRads(FVector InSource, float InAngleRads, FVector InRotationAxis);
	/*{
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
	}*/

	static FVector RotateAboutAxisDegs(FVector InSource, float InAngleDegs, FVector InRotationAxis)
	{
		return RotateAboutAxisRads(InSource, FMath::DegreesToRadians(InAngleDegs), InRotationAxis); //  * DEGS_TO_RADS,
	}

	static FVector GetAngleLimitedUnitVectorDegs(FVector InVecToLimit, FVector InVecBaseline, float InAngleLimitDegs);


	static FVector RotateYRads(FVector InSource, float InAngleRads)
	{
		float CosTheta = (float)FMath::Cos(InAngleRads);
		float SinTheta = (float)FMath::Sin(InAngleRads);
		return FVector(InSource.Z * SinTheta + InSource.X * CosTheta, InSource.Y, InSource.Z * CosTheta - InSource.X * SinTheta);
	}


	static FVector RotateYDegs(FVector source, float angleDegs) { return RotateYRads(source, FMath::DegreesToRadians(angleDegs)); }

	//cpp function
	//static int32 ComplicatedGameDataAnalysis();
};