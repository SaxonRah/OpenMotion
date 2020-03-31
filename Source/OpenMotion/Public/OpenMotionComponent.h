// Copyright (c) Name 2020

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

//

#include "OpenMotionComponent.generated.h"


USTRUCT(BlueprintType)
struct FWorldTransforms
{
	GENERATED_USTRUCT_BODY()

		// any non-UPROPERTY() struct vars are not replicated

	UPROPERTY(BlueprintReadWrite)
		FTransform HeadEffector;

	UPROPERTY(BlueprintReadWrite)
		FTransform LeftHandEffector;

	UPROPERTY(BlueprintReadWrite)
		FTransform RightHandEffector;

	UPROPERTY(BlueprintReadWrite)
		FTransform Component;

	UPROPERTY(BlueprintReadWrite)
		FTransform Shoulder;

	UPROPERTY(BlueprintReadWrite)
		FTransform LeftUpperArm;

	UPROPERTY(BlueprintReadWrite)
		FTransform LeftLowerArm;

	UPROPERTY(BlueprintReadWrite)
		FTransform RightUpperArm;

	UPROPERTY(BlueprintReadWrite)
		FTransform RightLowerArm;
};


USTRUCT(BlueprintType)
struct FShoulderTransforms
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadWrite)
		FTransform HeadEffector;

	UPROPERTY(BlueprintReadWrite)
		FTransform LeftHandEffector;

	UPROPERTY(BlueprintReadWrite)
		FTransform RightHandEffector;

	UPROPERTY(BlueprintReadWrite)
		FTransform Shoulder;

	UPROPERTY(BlueprintReadWrite)
		FTransform LeftUpperArm;

	UPROPERTY(BlueprintReadWrite)
		FTransform LeftLowerArm;

	UPROPERTY(BlueprintReadWrite)
		FTransform RightUpperArm;

	UPROPERTY(BlueprintReadWrite)
		FTransform RightLowerArm;
};

USTRUCT(BlueprintType)
struct FComponentTransforms
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadWrite)
		FTransform HeadEffector;

	UPROPERTY(BlueprintReadWrite)
		FTransform LeftHandEffector;

	UPROPERTY(BlueprintReadWrite)
		FTransform RightHandEffector;

	UPROPERTY(BlueprintReadWrite)
		FTransform Shoulder;

	UPROPERTY(BlueprintReadWrite)
		FTransform LeftClavicle;

	UPROPERTY(BlueprintReadWrite)
		FTransform RightClavicle;

	UPROPERTY(BlueprintReadWrite)
		FTransform LeftUpperArm;

	UPROPERTY(BlueprintReadWrite)
		FTransform LeftLowerArm;

	UPROPERTY(BlueprintReadWrite)
		FTransform RightUpperArm;

	UPROPERTY(BlueprintReadWrite)
		FTransform RightLowerArm;
};


USTRUCT(BlueprintType)
struct FTransformSettings
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadWrite)
		bool DrawDebug = true;

	UPROPERTY(BlueprintReadWrite)
		float UpperArmsDistance = 30.f;

	UPROPERTY(BlueprintReadWrite)
		float DistinctShoulderRotationMultiplier = 60.f;

	UPROPERTY(BlueprintReadWrite)
		float DistinctShoulderRotationLimit = 45.f;

	UPROPERTY(BlueprintReadWrite)
		float ClavicleOffset = 90.f;

	UPROPERTY(BlueprintReadWrite)
		float ElbowBaseOffsetAngle = 30.f;

	UPROPERTY(BlueprintReadWrite)
		float ElbowYDistanceStart = 0.2f;

	UPROPERTY(BlueprintReadWrite)
		float ElbowYWeight = 130.f;

	UPROPERTY(BlueprintReadWrite)
		float ElbowHandRotSpeed = 15.f;

	UPROPERTY(BlueprintReadWrite)
		float HeadHandAngleLimit = 50.f;

	UPROPERTY(BlueprintReadWrite)
		float HeadHandAngleOkSpan = 80.f;
};


USTRUCT(BlueprintType)
struct FCharacterSettings
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadWrite)
		FTransform CharacterBaseTransform;

	UPROPERTY(BlueprintReadWrite)
		float ArmLength = 0.f;

	UPROPERTY(BlueprintReadWrite)
		float LowerArmLength = 0.f;

	UPROPERTY(BlueprintReadWrite)
		float UpperArmLength = 0.f;

	UPROPERTY(BlueprintReadWrite)
		float LeftElbowHandAngle = 0.f;

	UPROPERTY(BlueprintReadWrite)
		float RightElbowHandAngle = 0.f;

	UPROPERTY(BlueprintReadWrite)
		float LeftHeadHandAngle = 0.f;

	UPROPERTY(BlueprintReadWrite)
		float RightHeadHandAngle = 0.f;

	UPROPERTY(BlueprintReadWrite)
		float HeadHandAngleLimitDot = 0.f;
};


USTRUCT(BlueprintType)
struct FArmTransforms
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadWrite)
		FTransform UpperArmTransform;
	UPROPERTY(BlueprintReadWrite)
		FTransform LowerArmTransform;

	FArmTransforms()
	{
		UpperArmTransform = FTransform();
		LowerArmTransform = FTransform();
	}

	FArmTransforms(FTransform UpperArm, FTransform LowerArm)
	{
		UpperArmTransform = UpperArm;
		LowerArmTransform = LowerArm;
	}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OPENMOTION_API UOpenMotionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOpenMotionComponent();

	UPROPERTY(BlueprintReadWrite)
	FWorldTransforms WorldTransforms;
	UPROPERTY(BlueprintReadWrite)
	FShoulderTransforms ShoulderTransforms;
	UPROPERTY(BlueprintReadWrite)
	FComponentTransforms ComponentTransforms;
	UPROPERTY(BlueprintReadWrite)
	FTransformSettings TransformSettings;
	UPROPERTY(BlueprintReadWrite)
	FCharacterSettings CharacterSettings;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "")
		void CustomTick(USkeletalMeshComponent* OwnerMesh);
	UFUNCTION(BlueprintCallable, Category = "")
		void ConvertTransforms();
	UFUNCTION(BlueprintCallable, Category = "")
		void SetCharacterTransforms(FTransform LeftHandEffector, FTransform RightHandEffector, FTransform HeadEffector, FTransform ComponentTransform);
	UFUNCTION(BlueprintCallable, Category = "")
		void SetShoulder();
	UFUNCTION(BlueprintCallable, Category = "")
		void SetLeftUpperArm();
	UFUNCTION(BlueprintCallable, Category = "")
		void SetRightUpperArm();
	UFUNCTION(BlueprintCallable, Category = "")
		void ResetUpperArmsLocation(USkeletalMeshComponent* OwnerMesh);
	UFUNCTION(BlueprintCallable, Category = "")
		void SolveArms();
	UFUNCTION(BlueprintCallable, Category = "")
		void DebugDraw();
	UFUNCTION(BlueprintCallable, Category = "")
		void Calibrate(float CharacterHeight);
	UFUNCTION(BlueprintCallable, Category = "")
		FTransform RotateUpperArm(bool bIsLeftArm, FVector HandLocation);
	UFUNCTION(BlueprintCallable, Category = "")
		FArmTransforms SetElbowBasePosition(bool bIsLeftArm, FVector UpperArmLocation, FVector HandLocation);
	UFUNCTION(BlueprintCallable, Category = "")
		float RotateElbowByHandPosition(bool bIsLeftArm, FVector HandLocation);
	UFUNCTION(BlueprintCallable, Category = "")
		float RotateElbowByHandRotation(FTransform LowerArmLocation, FRotator HandRotation);

	UFUNCTION(BlueprintCallable, Category = "")
		float CosineRule(float AdjacentA, float AdjacentB, float Opposite);
	UFUNCTION(BlueprintCallable, Category = "")
		FArmTransforms RotateElbow(bool bIsLeftArm, float Angle, FArmTransforms UpperAndLowerArms, FVector HandLocation);
	UFUNCTION(BlueprintCallable, Category = "")
		float SafeguardAngle(float Current, float Last, float Threshold);
	UFUNCTION(BlueprintCallable, Category = "")
		FTransform RotatePointAroundPivot(FTransform Point, FTransform Pivot, FRotator Delta);
	UFUNCTION(BlueprintCallable, Category = "")
		FVector GetDebugValues();
	UFUNCTION(BlueprintCallable, Category = "")
		float GetHeadHandAngle(float LastAngle, FVector HandLocation, FVector HandHeadDelta);
	UFUNCTION(BlueprintCallable, Category = "")
		FTransform GetBaseCharTransform();
	UFUNCTION(BlueprintCallable, Category = "")
		FRotator GetShoulderRotationFromHead();
	UFUNCTION(BlueprintCallable, Category = "")
		FRotator GetShoulderRotationFromHands();
	UFUNCTION(BlueprintCallable, Category = "")
		FRotator FindBetweenNormals(FVector A, FVector B);
};
