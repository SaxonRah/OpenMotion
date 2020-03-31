// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameFramework/Actor.h"
#include "FabrikDemoActor.generated.h"

class UFabrikStructure;
class UFabrikChain;
class UFabrikDebugComponent;

UENUM(BlueprintType)
enum class EFabrikDemoType : uint8
{
	FD_UnconstrainedBones 	UMETA(DisplayName = "Demo 1 - Unconstrained bones"),
	FD_RotorBallJointConstrainedBones 	UMETA(DisplayName = "Demo 2 - Rotor / Ball Joint Constrained Bones"),
	FD_RotorConstrainedBaseBones	UMETA(DisplayName = "Demo 3 - Rotor Constrained Base Bones"),
	FD_FreelyRotatingGlobalHinges	UMETA(DisplayName = "Demo 4 - Freely Rotating Global Hinges"),
	FD_GlobalHingesWithReferenceAxisConstraints	UMETA(DisplayName = "Demo 5 - Global Hinges With Reference Axis Constraints"),
	FD_FreelyRotatingLocalHinges	UMETA(DisplayName = "Demo 6 - Freely Rotating Local Hinges"),
	FD_LocalHingesWithReferenceAxisConstraints	UMETA(DisplayName = "Demo 7 - Local Hinges with Reference Axis Constraints"),
	FD_ConnectedChains	UMETA(DisplayName = "Demo 8 - Connected Chains"),
	FD_GlobalRotorConstrainedConnectedChains	UMETA(DisplayName = "Demo 9 - Global Rotor Constrained Connected Chains"),
	FD_LocalRotorConstrainedConnectedChains	UMETA(DisplayName = "Demo 10 - Local Rotor Constrained Connected Chains"),
	FD_ConnectedChainsWithFreelyRotatingGlobalHingedBaseboneConstraints	UMETA(DisplayName = "Demo 11 - Connected Chains with Freely-Rotating Global Hinged Basebone Constraints"),
	FD_ConnectedChainsWithEmbeddedTargets	UMETA(DisplayName = "Demo 12 - Connected chains with embedded targets")
};

UCLASS()
class OPENMOTION_API AFabrikDemoActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFabrikDemoActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		UFabrikDebugComponent* FabrikDebugComponent;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		EFabrikDemoType DemoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FVector XAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FVector YAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FVector ZAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		FVector DefaultBoneDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		float DefaultBoneLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		float BoneLineWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		float ConstraintLineWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		float BaseRotationAmountDegs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		AActor* TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		UFabrikStructure* Structure;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		float PointSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		float LineThickness;

	void DrawChain(UFabrikChain* Chain);

	FColor Brightness(FColor color, float correctionFactor);

	void DemoUnconstrainedBones();
	void DemoRotorBallJointConstrainedBones();
	void DemoRotorConstrainedBaseBones();
	void DemoFreelyRotatingGlobalHinges();
	void DemoGlobalHingesWithReferenceAxisConstraints();
	void DemoFreelyRotatingLocalHinges();
	void DemoLocalHingesWithReferenceAxisConstraints();
	void DemoConnectedChains();
	void DemoGlobalRotorConstrainedConnectedChains();
	void DemoLocalRotorConstrainedConnectedChains();
	void DemoConnectedChainsWithFreelyRotatingGlobalHingedBaseboneConstraints();
	void DemoConnectedChainsWithEmbeddedTargets();
};
