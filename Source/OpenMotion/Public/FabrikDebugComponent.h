// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Components/ActorComponent.h"
#include "FabrikDebugComponent.generated.h"

class UFabrikStructure;
class UFabrikChain;
class UFabrikMat3f;
class UFabrikBone;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OPENMOTION_API UFabrikDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFabrikDebugComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Setting)
		UFabrikStructure* Structure;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		bool DrawEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		float PointSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		float LineThickness;



	// Constraint colours
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Color)
		FColor ANTICLOCKWISE_CONSTRAINT_COLOUR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Color)
		FColor CLOCKWISE_CONSTRAINT_COLOUR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Color)
		FColor BALL_JOINT_COLOUR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Color)
		FColor GLOBAL_HINGE_COLOUR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Color)
		FColor LOCAL_HINGE_COLOUR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Color)
		FColor REFERENCE_AXIS_COLOUR;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		float CONE_LENGTH_FACTOR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		float RADIUS_FACTOR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		int NUM_CONE_LINES;


	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Color)
		float rotStep;


	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

		
	void DrawChainBones(UFabrikChain* Chain);

	void DrawConstraint(UFabrikBone* bone, FVector referenceDirection, float lineWidth/*, UFabrikMat3f* mvpMatrix*/);
	void DrawLine(FVector Start, FVector End, FColor Color, float lineWidth);
	void DrawCircle(FVector Start, FVector Axis, float Radius, FColor Color, float LineWidth);
	void DrawChainConstraints(UFabrikChain* Chain, float LineWidth);
};
