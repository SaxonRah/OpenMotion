// Fill out your copyright notice in the Description page of Project Settings.

// https://github.com/FedUni/caliko/blob/master/caliko/src/au/edu/federation/utils/Mat3f.java

#include "FabrikMat3f.h"

#include "OpenMotion.h"


UFabrikMat3f* UFabrikMat3f::CreateRotationMatrix(FVector InReferenceDirection)
{
	FVector xAxis;
	FVector yAxis;
	InReferenceDirection.Normalize();
	FVector zAxis = InReferenceDirection;// .normalise();
	
	// Handle the singularity (i.e. bone pointing along negative Z-Axis)...
	if (InReferenceDirection.Z < -0.9999999f)
	{
		xAxis = FVector(1.0f, 0.0f, 0.0f); // ...in which case positive X runs directly to the right...
		yAxis = FVector(0.0f, 1.0f, 0.0f); // ...and positive Y runs directly upwards.
	}
	else
	{
		float a = 1.0f / (1.0f + zAxis.Z);
		float b = -zAxis.X * zAxis.Y * a;
		xAxis = FVector(1.0f - zAxis.X * zAxis.X * a, b, -zAxis.X);// .normalise();
		xAxis.Normalize();
		yAxis = FVector(b, 1.0f - zAxis.Y * zAxis.Y * a, -zAxis.Y);// .normalise();
	}

	UFabrikMat3f* Res = NewObject<UFabrikMat3f>();
	Res->Init(xAxis, yAxis, zAxis);
	return Res;// new Mat3f(xAxis, yAxis, zAxis);
}

void UFabrikMat3f::Init(FVector xAxis, FVector yAxis, FVector zAxis)
{
	m00 = xAxis.X;
	m01 = xAxis.Y;
	m02 = xAxis.Z;

	m10 = yAxis.X;
	m11 = yAxis.Y;
	m12 = yAxis.Z;

	m20 = zAxis.X;
	m21 = zAxis.Y;
	m22 = zAxis.Z;
}

FVector UFabrikMat3f::Times(FVector source)
{
	return FVector(this->m00 * source.X + this->m10 * source.Y + this->m20 * source.Z,
		this->m01 * source.X + this->m11 * source.Y + this->m21 * source.Z,
		this->m02 * source.X + this->m12 * source.Y + this->m22 * source.Z);
}