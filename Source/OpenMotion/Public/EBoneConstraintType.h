// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "CoreMinimal.h"



UENUM()
enum class EBoneConstraintType : uint8
{
	BCT_None = 0 UMETA(Hidden),
	BCT_NoConstraint = 1 UMETA(HiddenDisplayName = "None"), // No constraint - basebone may rotate freely
	BCT_GlobalRotor = 2 UMETA(DisplayName = "Global Rotor"), // World-space rotor constraint
	BCT_LocalRotor = 3 UMETA(DisplayName = "Local Rotor"), // Rotor constraint in the coordinate space of (i.e. relative to) the direction of the connected bone
	BCT_GlobalHinge = 4 UMETA(DisplayName = "Global Hinge"), // World-space hinge constraint
	BCT_LocalHinge = 5 UMETA(DisplayName = "Local Hinge") // Hinge constraint in the coordinate space of (i.e. relative to) the direction of the connected bone
};