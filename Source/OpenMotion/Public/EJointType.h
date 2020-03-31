// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "CoreMinimal.h"


UENUM()
enum class EJointType : uint8
{
	JT_None = 0 UMETA(Hidden),
	JT_Ball			= 1 UMETA(DisplayName = "Ball"),
	JT_GlobalHinge = 2 UMETA(DisplayName = "Global Hinge"),
	JT_LocalHinge = 3 UMETA(DisplayName = "Local Hinge")
};