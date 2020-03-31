// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "FabrikMat3f.generated.h"

UCLASS()
class OPENMOTION_API UFabrikMat3f : public UObject
{
	GENERATED_BODY()

public:
	float m00, m01, m02; // First  column - typically the direction of the positive X-axis
	float m10, m11, m12; // Second column - typically the direction of the positive Y-axis
	float m20, m21, m22; // Third  column - typically the direction of the positive Z-axis

	
	static UFabrikMat3f* CreateRotationMatrix(FVector InReferenceDirection);
	void Init(FVector xAxis, FVector yAxis, FVector zAxis);
	FVector Times(FVector source);
};
