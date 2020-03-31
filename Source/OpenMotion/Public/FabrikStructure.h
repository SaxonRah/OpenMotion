// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "EBoneConnectionPoint.h"
#include "FabrikStructure.generated.h"

class UFabrikChain;

/**
 * 
 */
UCLASS()
class OPENMOTION_API UFabrikStructure : public UObject
{
	GENERATED_BODY()
	
public:

	UFabrikStructure(const FObjectInitializer& ObjectInitializer);

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
			FName Name;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
			TArray<UFabrikChain*> Chains;// = new ArrayList<FabrikChain3D>();

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
			int NumChains;
	
		void SolveForTarget(FVector InNewTargetLocation);
		void SolveForTarget(float InTargetX, float InTargetY, float InTargetZ);
		void AddChain(UFabrikChain* InChain);
		void RemoveChain(int InChainIndex);
		void ConnectChain(UFabrikChain* InNewChain, int InExistingChainNumber, int InExistingBoneNumber);
		void ConnectChain(UFabrikChain* InNewChain, int InExistingChainNumber, int InExistingBoneNumber, EBoneConnectionPoint InBoneConnectionPoint);

		void SetFixedBaseMode(bool InFixedBaseMode);
};
