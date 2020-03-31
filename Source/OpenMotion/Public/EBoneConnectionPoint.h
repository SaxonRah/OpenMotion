// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "UObject/NoExportTypes.h"
#include "CoreMinimal.h"

UENUM()
enum class EBoneConnectionPoint : uint8
{
	BCP_None = 0 UMETA(Hidden),
	BCP_Start = 1 UMETA(DisplayName = "Start"),
	BCP_End = 2 UMETA(DisplayName = "End")
};