// Copyright (c) Name 2020


#include "OpenMotionComponent.h"
#include "..\Public\OpenMotionComponent.h"

#include "Engine.h"
//#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "GameFramework/Character.h"

// Sets default values for this component's properties
UOpenMotionComponent::UOpenMotionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UOpenMotionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UOpenMotionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// CustomTick(); Call this in bp, to set mesh reliably

	// FWorldTransforms WorldTransforms;
	// FShoulderTransforms ShoulderTransforms;
	// FComponentTransforms ComponentTransforms;
	// FTransformSettings TransformSettings;
	// FCharacterSettings CharacterSettings;

}

void UOpenMotionComponent::CustomTick(USkeletalMeshComponent* OwnerMesh)
{
	ConvertTransforms();
	SetShoulder();
	SetLeftUpperArm(); SetRightUpperArm();
	ResetUpperArmsLocation(OwnerMesh); SolveArms();
	CharacterSettings.CharacterBaseTransform = GetBaseCharTransform();

	if (TransformSettings.DrawDebug)
	{
		DebugDraw();
	}
}

void UOpenMotionComponent::ConvertTransforms()
{
	FTransform LeftHandInverseLocation = WorldTransforms.LeftHandEffector.Inverse();
	FTransform RightHandInverseLocation = WorldTransforms.RightHandEffector.Inverse();

	WorldTransforms.LeftHandEffector = FTransform(LeftHandInverseLocation.GetRotation(), LeftHandInverseLocation.GetLocation() + FVector(8.0f, 0.f, 0.f), LeftHandInverseLocation.GetScale3D());
	WorldTransforms.RightHandEffector = FTransform(RightHandInverseLocation.GetRotation(), RightHandInverseLocation.GetLocation() + FVector(8.0f, 0.f, 0.f), RightHandInverseLocation.GetScale3D()).Inverse();

	FTransform LocalComponentTransform = WorldTransforms.Component.Inverse();
	ComponentTransforms.HeadEffector = WorldTransforms.HeadEffector * LocalComponentTransform;
	ComponentTransforms.LeftHandEffector = WorldTransforms.LeftHandEffector * LocalComponentTransform;
	ComponentTransforms.RightHandEffector = WorldTransforms.RightHandEffector * LocalComponentTransform;

	WorldTransforms.Shoulder = ComponentTransforms.Shoulder * WorldTransforms.Component;
	FTransform LocalShoulderTransform = WorldTransforms.Shoulder.Inverse();
	ShoulderTransforms.HeadEffector = WorldTransforms.HeadEffector * LocalShoulderTransform;
	ShoulderTransforms.LeftHandEffector = WorldTransforms.LeftHandEffector * LocalShoulderTransform;
	ShoulderTransforms.RightHandEffector = WorldTransforms.RightHandEffector * LocalShoulderTransform;
}

void UOpenMotionComponent::SetCharacterTransforms(FTransform LeftHandEffector, FTransform RightHandEffector, FTransform HeadEffector, FTransform ComponentTransform)
{
	WorldTransforms.LeftHandEffector = LeftHandEffector;
	WorldTransforms.RightHandEffector = RightHandEffector;
	WorldTransforms.HeadEffector = HeadEffector;
	WorldTransforms.Component = ComponentTransform;
}

void UOpenMotionComponent::SetShoulder()
{
	FVector A = UKismetMathLibrary::Multiply_VectorFloat(UKismetMathLibrary::Vector_Down(), 0.7f) + UKismetMathLibrary::Multiply_VectorFloat(UKismetMathLibrary::Vector_Backward(), 9.f);
	FRotator B = FRotator(0.f, ComponentTransforms.HeadEffector.GetRotation().Z, ComponentTransforms.HeadEffector.GetRotation().X);
	FVector RotatedVector = UKismetMathLibrary::GreaterGreater_VectorRotator(A, B);

	FRotator Delta = FRotator(ComponentTransforms.HeadEffector.GetRotation().Y, 0.f, 0.f);
	FVector TransformedLocation = ComponentTransforms.HeadEffector.GetLocation() + RotatedVector;
	FTransform RotationPoint = FTransform(FRotator(0.f, 0.f, 0.f), TransformedLocation, FVector(1, 1, 1));

	FVector ComponentShoulderLocation = RotatePointAroundPivot(RotationPoint, ComponentTransforms.HeadEffector, Delta).GetLocation() + FVector(0, 0, -17);
	FRotator ComponentShoulderRotation = FRotator(0.f, UKismetMathLibrary::RLerp(GetShoulderRotationFromHead(), GetShoulderRotationFromHands(), 0.7f, true).Yaw, 0.f);

	ComponentTransforms.Shoulder = FTransform(ComponentShoulderRotation, ComponentShoulderLocation, FVector(1, 1, 1));
}

void UOpenMotionComponent::SetLeftUpperArm()
{
	ShoulderTransforms.LeftUpperArm = RotateUpperArm(true, ShoulderTransforms.LeftHandEffector.GetLocation());
	FRotator ModifiedRotator = UKismetMathLibrary::MakeRotFromXZ((ShoulderTransforms.LeftUpperArm * WorldTransforms.Shoulder).GetLocation() - WorldTransforms.Shoulder.GetLocation(), UKismetMathLibrary::Vector_Up());
	ComponentTransforms.LeftClavicle = FTransform(ModifiedRotator, FVector(0, 0, 0), FVector(1, 1, 1)) * WorldTransforms.Component.Inverse();
}

void UOpenMotionComponent::SetRightUpperArm()
{
	ShoulderTransforms.RightUpperArm = RotateUpperArm(false, ShoulderTransforms.RightHandEffector.GetLocation());
	FRotator ModifiedRotator = UKismetMathLibrary::MakeRotFromXZ((ShoulderTransforms.RightUpperArm * WorldTransforms.Shoulder).GetLocation() - WorldTransforms.Shoulder.GetLocation(), UKismetMathLibrary::Vector_Up());
	ComponentTransforms.RightClavicle = FTransform(ModifiedRotator, FVector(0, 0, 0), FVector(1, 1, 1)) * WorldTransforms.Component.Inverse();
}

void UOpenMotionComponent::ResetUpperArmsLocation(USkeletalMeshComponent* OwnerMesh)
{
	if (OwnerMesh != nullptr)
	{
		FVector LeftUpperArmLocation = OwnerMesh->GetSocketLocation("UpperArm_l");
		FVector RightUpperArmLocation = OwnerMesh->GetSocketLocation("UpperArm_r");
		ShoulderTransforms.LeftUpperArm = FTransform(FRotator(0.f, 0.f, 0.f), WorldTransforms.Shoulder.InverseTransformPosition(LeftUpperArmLocation), FVector(1, 1, 1));
		ShoulderTransforms.RightUpperArm = FTransform(FRotator(0.f, 0.f, 0.f), WorldTransforms.Shoulder.InverseTransformPosition(RightUpperArmLocation), FVector(1, 1, 1));
	}
}

void UOpenMotionComponent::SolveArms()
{
	FVector Local_HandLocation = FVector(ShoulderTransforms.LeftHandEffector.GetLocation());
	FRotator Local_HandRotation = FRotator(ShoulderTransforms.LeftHandEffector.GetRotation());
	FArmTransforms Local_BasePosition = SetElbowBasePosition(true, ShoulderTransforms.LeftLowerArm.GetLocation(), Local_HandLocation);
	float Local_Angle = RotateElbowByHandPosition(true, Local_HandLocation);
	FArmTransforms TempArms = RotateElbow(true, Local_Angle, Local_BasePosition, Local_HandLocation);
	float TempAngle = RotateElbowByHandRotation(TempArms.LowerArmTransform, Local_HandRotation);

	CharacterSettings.LeftElbowHandAngle = UKismetMathLibrary::FInterpTo(
		CharacterSettings.LeftElbowHandAngle,
		SafeguardAngle(CharacterSettings.LeftElbowHandAngle, TempAngle, 120.f),
		UGameplayStatics::GetWorldDeltaSeconds(GEngine->GetWorldContexts()[0].World()),
		TransformSettings.ElbowHandRotSpeed);

	FArmTransforms TempArms2 = RotateElbow(true, CharacterSettings.LeftElbowHandAngle + Local_Angle, Local_BasePosition, Local_HandLocation);
	ShoulderTransforms.LeftUpperArm = TempArms2.UpperArmTransform;
	WorldTransforms.LeftUpperArm = ShoulderTransforms.LeftUpperArm * WorldTransforms.Shoulder;

	ShoulderTransforms.LeftLowerArm = TempArms2.LowerArmTransform;
	WorldTransforms.LeftLowerArm = ShoulderTransforms.LeftLowerArm * WorldTransforms.Shoulder;
	FTransform TempTransform = WorldTransforms.LeftUpperArm * WorldTransforms.Component.Inverse();
	FVector TempLocation = WorldTransforms.LeftHandEffector.GetLocation() - WorldTransforms.LeftUpperArm.GetLocation();
	FRotator TempRotator = UKismetMathLibrary::ComposeRotators(FRotator(0.f, 0.f, UKismetMathLibrary::Max(TempLocation.Z, 0.f)), FRotator(TempTransform.GetRotation()));

	ComponentTransforms.LeftUpperArm = FTransform(TempRotator, TempTransform.GetLocation(), FVector(1, 1, 1));

	ComponentTransforms.LeftLowerArm = WorldTransforms.LeftLowerArm * WorldTransforms.Component.Inverse();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FVector Local_HandLocation2 = FVector(ShoulderTransforms.RightHandEffector.GetLocation());
	FRotator Local_HandRotation2 = FRotator(ShoulderTransforms.RightHandEffector.GetRotation());
	FArmTransforms Local_BasePosition2 = SetElbowBasePosition(true, ShoulderTransforms.RightLowerArm.GetLocation(), Local_HandLocation2);
	float Local_Angle2 = RotateElbowByHandPosition(true, Local_HandLocation2);
	FArmTransforms TempArms3 = RotateElbow(true, Local_Angle2, Local_BasePosition2, Local_HandLocation2);
	float TempAngle2 = RotateElbowByHandRotation(TempArms3.LowerArmTransform, Local_HandRotation2);

	CharacterSettings.RightElbowHandAngle = UKismetMathLibrary::FInterpTo(
		CharacterSettings.RightElbowHandAngle,
		SafeguardAngle(CharacterSettings.RightElbowHandAngle, TempAngle2, 120.f),
		UGameplayStatics::GetWorldDeltaSeconds(GEngine->GetWorldContexts()[0].World()),
		TransformSettings.ElbowHandRotSpeed);

	FArmTransforms TempArms4 = RotateElbow(true, CharacterSettings.RightElbowHandAngle + Local_Angle2, Local_BasePosition2, Local_HandLocation2);
	ShoulderTransforms.RightUpperArm = TempArms4.UpperArmTransform;
	WorldTransforms.RightUpperArm = ShoulderTransforms.RightUpperArm * WorldTransforms.Shoulder;

	ShoulderTransforms.RightLowerArm = TempArms4.LowerArmTransform;
	WorldTransforms.RightLowerArm = ShoulderTransforms.RightLowerArm * WorldTransforms.Shoulder;
	FTransform TempTransform2 = WorldTransforms.RightUpperArm * WorldTransforms.Component.Inverse();
	FVector TempLocation2 = WorldTransforms.RightHandEffector.GetLocation() - WorldTransforms.RightUpperArm.GetLocation();
	FRotator TempRotator2 = UKismetMathLibrary::ComposeRotators(FRotator(0.f, 0.f, UKismetMathLibrary::Max(TempLocation2.Z, 0.f) * -1.f), FRotator(TempTransform2.GetRotation()));

	ComponentTransforms.RightUpperArm = FTransform(TempRotator2, TempTransform2.GetLocation(), FVector(1, 1, 1));

	ComponentTransforms.RightLowerArm = WorldTransforms.LeftLowerArm * WorldTransforms.Component.Inverse();
}

void UOpenMotionComponent::DebugDraw()
{
}

void UOpenMotionComponent::Calibrate(float CharacterHeight)
{
	CharacterSettings.ArmLength = (CharacterHeight / 2) - TransformSettings.UpperArmsDistance;
	CharacterSettings.UpperArmLength = CharacterSettings.ArmLength * (1.f - 0.48f);
	CharacterSettings.LowerArmLength = CharacterSettings.ArmLength * 0.48f;
	CharacterSettings.HeadHandAngleLimitDot = UKismetMathLibrary::DegCos(TransformSettings.HeadHandAngleLimit);
}

FTransform UOpenMotionComponent::RotateUpperArm(bool bIsLeftArm, FVector HandLocation)
{
	float TempFloat = TransformSettings.UpperArmsDistance / 2.f * UKismetMathLibrary::SelectFloat(1.f, -1.f, bIsLeftArm);
	FVector LInitialUpperArmPos = UKismetMathLibrary::Multiply_VectorFloat(UKismetMathLibrary::Vector_Right(), TempFloat);
	FVector LHandUpperArmDir = HandLocation - LInitialUpperArmPos;
	float LForwardDistanceRatio = UKismetMathLibrary::Dot_VectorVector(LHandUpperArmDir, UKismetMathLibrary::Vector_Forward()) / CharacterSettings.ArmLength;
	float LUpwardsDistanceRatio = UKismetMathLibrary::Dot_VectorVector(LHandUpperArmDir, UKismetMathLibrary::Vector_Up()) / CharacterSettings.ArmLength;

	float LYaw = 0.f;
	float LRoll = 0.f;

	if (LForwardDistanceRatio > 0.f)
	{
		LYaw = UKismetMathLibrary::FClamp((LForwardDistanceRatio - 0.5f) * TransformSettings.DistinctShoulderRotationMultiplier, 0.f, TransformSettings.DistinctShoulderRotationLimit) + TransformSettings.ClavicleOffset;
	}
	else
	{
		LYaw = UKismetMathLibrary::FClamp((LForwardDistanceRatio - 0.08f) * TransformSettings.DistinctShoulderRotationMultiplier, TransformSettings.DistinctShoulderRotationLimit * -1.f, 0.f) + TransformSettings.ClavicleOffset;
	}

	LRoll = UKismetMathLibrary::FClamp((LUpwardsDistanceRatio - 0.2f) * TransformSettings.DistinctShoulderRotationMultiplier, 0.f, TransformSettings.DistinctShoulderRotationLimit) + TransformSettings.ClavicleOffset;

	return FTransform(FRotator(0.f, LYaw * UKismetMathLibrary::SelectFloat(-1.f, 1.f, bIsLeftArm), LRoll * UKismetMathLibrary::SelectFloat(-1.f, 1.f, bIsLeftArm)), LInitialUpperArmPos, FVector(1, 1, 1)).Inverse();
}

FArmTransforms UOpenMotionComponent::SetElbowBasePosition(bool bIsLeftArm, FVector UpperArmLocation, FVector HandLocation)
{
	float LUpperArmToHandLen = (UpperArmLocation - HandLocation).Size();
	float LBeta = CosineRule(CharacterSettings.UpperArmLength, LUpperArmToHandLen, CharacterSettings.LowerArmLength) * UKismetMathLibrary::SelectFloat(-1.f, 1.f, bIsLeftArm);
	float CRule = CosineRule(CharacterSettings.LowerArmLength, CharacterSettings.UpperArmLength, LUpperArmToHandLen);
	float LOmega = UKismetMathLibrary::SelectFloat(180.f - CRule, CRule + 180, bIsLeftArm);
	FVector B = HandLocation - UpperArmLocation;
	UKismetMathLibrary::Vector_Normalize(B, 0.0001);
	FRotator Normal = FindBetweenNormals(UKismetMathLibrary::Vector_Forward(), B);
	FRotator Rotator = FRotator(UKismetMathLibrary::ComposeRotators(Normal, UKismetMathLibrary::RotatorFromAxisAndAngle(UKismetMathLibrary::GetUpVector(Normal), LBeta)));
	FTransform T1 = FTransform(Rotator, UpperArmLocation, FVector(1, 1, 1));
	FTransform T2 = FTransform(FRotator(0.f, LOmega, 0.f), UKismetMathLibrary::Multiply_VectorFloat(UKismetMathLibrary::Vector_Forward(), CharacterSettings.UpperArmLength), FVector(1, 1, 1));
	return FArmTransforms(T1, T2 * T1);
}

float UOpenMotionComponent::RotateElbowByHandPosition(bool bIsLeftArm, FVector HandLocation)
{
	float Temp = UKismetMathLibrary::Divide_VectorFloat(HandLocation, CharacterSettings.ArmLength).Y * UKismetMathLibrary::SelectFloat(1.f, -1.f, bIsLeftArm);
	return TransformSettings.ElbowBaseOffsetAngle + (TransformSettings.ElbowYWeight + UKismetMathLibrary::Max(0.f, Temp + TransformSettings.ElbowYDistanceStart));
}

float UOpenMotionComponent::RotateElbowByHandRotation(FTransform LowerArmLocation, FRotator HandRotation)
{
	FVector RightVector = UKismetMathLibrary::GetRightVector(FRotator(LowerArmLocation.GetRotation()));
	FVector NormalizedVector = UKismetMathLibrary::ProjectVectorOnToPlane(UKismetMathLibrary::GetForwardVector(HandRotation), RightVector);
	UKismetMathLibrary::Vector_Normalize(NormalizedVector, 0.0001);
	FVector ForwardVector = UKismetMathLibrary::GetForwardVector(FRotator(LowerArmLocation.GetRotation()));
	float Length = (NormalizedVector - ForwardVector).Size();
	float Dot = UKismetMathLibrary::Dot_VectorVector(UKismetMathLibrary::Cross_VectorVector(NormalizedVector, ForwardVector), RightVector);
	return (CosineRule(1.f, 1.f, Length) * UKismetMathLibrary::SelectFloat(1.f, -1.f, Dot < 0.f)) * 0.6f;
}

float UOpenMotionComponent::CosineRule(float AdjacentA, float AdjacentB, float Opposite)
{
	return UKismetMathLibrary::Acos((((AdjacentA * AdjacentA) + (AdjacentB * AdjacentB)) - (Opposite * Opposite)) / (AdjacentA * AdjacentB * 2));
}

FArmTransforms UOpenMotionComponent::RotateElbow(bool bIsLeftArm, float Angle, FArmTransforms UpperAndLowerArms, FVector HandLocation)
{
	FVector UpperArmHandAxis = UpperAndLowerArms.UpperArmTransform.GetLocation() - HandLocation;
	FVector PointDirectionVector = UKismetMathLibrary::ProjectVectorOnToVector(UpperAndLowerArms.UpperArmTransform.GetLocation() - UpperAndLowerArms.LowerArmTransform.GetLocation(), UpperArmHandAxis);
	FVector PivotLocation = UpperAndLowerArms.UpperArmTransform.GetLocation() + PointDirectionVector;
	FVector UpperArmRotationUpVector = UpperAndLowerArms.UpperArmTransform.GetRotation().GetUpVector();
	FRotator PivotRotation = FRotator(UKismetMathLibrary::MakeRotationFromAxes(UpperArmHandAxis, UKismetMathLibrary::Cross_VectorVector(UpperArmRotationUpVector, UpperArmHandAxis), UpperArmRotationUpVector));
	FTransform TempTransform = FTransform(PivotRotation, PivotLocation, FVector(1, 1, 1));
	FRotator TempRotator = FRotator(0.f, 0.f, UKismetMathLibrary::SelectFloat(180.f - Angle, 180.f + Angle, bIsLeftArm));
	return FArmTransforms(RotatePointAroundPivot(UpperAndLowerArms.UpperArmTransform, TempTransform, TempRotator), RotatePointAroundPivot(UpperAndLowerArms.LowerArmTransform, TempTransform, TempRotator));
}

float UOpenMotionComponent::SafeguardAngle(float Current, float Last, float Threshold)
{	
	return UKismetMathLibrary::SelectFloat(Last, Current, bool(UKismetMathLibrary::Abs(Last - Current) > Threshold));
}

FTransform UOpenMotionComponent::RotatePointAroundPivot(FTransform Point, FTransform Pivot, FRotator Delta)
{
	return (((Point * Pivot.Inverse()) * FTransform(Delta, FVector(0, 0, 0), FVector(1, 1, 1))) * Pivot);
}

FVector UOpenMotionComponent::GetDebugValues()
{
	return FVector();
}

float UOpenMotionComponent::GetHeadHandAngle(float LastAngle, FVector HandLocation, FVector HandHeadDelta)
{
	float Local_LastAngle = LastAngle;
	FVector Local_SubAngle = FVector(FVector(HandLocation.X, HandLocation.Y, 0).Normalize(0.0001));
	float Local_Angle = UKismetMathLibrary::Atan2(Local_SubAngle.Y, Local_SubAngle.X);

	float Local_Alpha = UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::VSizeXY(HandHeadDelta), 20.f, 50.f, 0.f, 1.f);

	bool OR = ((UKismetMathLibrary::SignOfFloat(Local_LastAngle) == UKismetMathLibrary::SignOfFloat(Local_Angle)) || Local_Angle < TransformSettings.HeadHandAngleOkSpan && Local_Angle > TransformSettings.HeadHandAngleOkSpan * -1.f);
	bool bPickA = ((UKismetMathLibrary::Dot_VectorVector(Local_SubAngle, UKismetMathLibrary::Vector_Forward()) > CharacterSettings.HeadHandAngleLimitDot) && OR);
	return UKismetMathLibrary::Lerp(0.f, UKismetMathLibrary::SelectFloat(Local_Angle, TransformSettings.HeadHandAngleLimit * UKismetMathLibrary::SignOfFloat(Local_LastAngle), bPickA), Local_Alpha);
}

FTransform UOpenMotionComponent::GetBaseCharTransform()
{
	return FTransform(ComponentTransforms.Shoulder.GetRotation(), ComponentTransforms.Shoulder.GetLocation() - FVector(0.f, 0.f, 12.f + 43.25f), FVector(1, 1, 1));
}

FRotator UOpenMotionComponent::GetShoulderRotationFromHead()
{
	return FRotator(0.f, ComponentTransforms.HeadEffector.GetRotation().Z, 0.f);
}

FRotator UOpenMotionComponent::GetShoulderRotationFromHands()
{
	FVector Local_TopHead = UKismetMathLibrary::GreaterGreater_VectorRotator(
		UKismetMathLibrary::Multiply_VectorFloat(UKismetMathLibrary::Vector_Up(), 15.f) + UKismetMathLibrary::Multiply_VectorFloat(UKismetMathLibrary::Vector_Forward(), 0.f),
		FRotator(WorldTransforms.HeadEffector.GetRotation())) + WorldTransforms.HeadEffector.GetLocation();
	CharacterSettings.LeftHeadHandAngle = GetHeadHandAngle(CharacterSettings.LeftHeadHandAngle, UKismetMathLibrary::ComposeTransforms(WorldTransforms.LeftHandEffector, WorldTransforms.HeadEffector.Inverse()).GetLocation(), WorldTransforms.LeftHandEffector.GetLocation() - Local_TopHead);
	CharacterSettings.RightHeadHandAngle = GetHeadHandAngle(CharacterSettings.RightHeadHandAngle, UKismetMathLibrary::ComposeTransforms(WorldTransforms.RightHandEffector, WorldTransforms.HeadEffector.Inverse()).GetLocation(), WorldTransforms.RightHandEffector.GetLocation() - Local_TopHead);
	
	FTransform TempTransform = FTransform(FRotator(0.f, CharacterSettings.LeftHeadHandAngle + CharacterSettings.RightHeadHandAngle / 2, 0.f), FVector(0.f, 0.f, 0.f), FVector(1, 1, 1)) * WorldTransforms.HeadEffector * WorldTransforms.Component.Inverse();
	return FRotator(TempTransform.GetRotation());
}

FRotator UOpenMotionComponent::FindBetweenNormals(FVector A, FVector B)
{
	float W = UKismetMathLibrary::Dot_VectorVector(A, B) + 1.f;
	if (W > 0.000001)
	{
		float X = UKismetMathLibrary::Cross_VectorVector(A, B).X;
		float Y = UKismetMathLibrary::Cross_VectorVector(A, B).Y;
		float Z = UKismetMathLibrary::Cross_VectorVector(A, B).Z;
		FQuat QQQ = FQuat(X, Y, Z, W);

		UKismetMathLibrary::Quat_Normalize(QQQ, 0.0001);
		return FRotator(QQQ.Rotator());
	}
	else
	{
		W = 0;

		FVector AVec = FVector(A.Z * 1.f, 0.f, A.X);
		FVector BVec = FVector(0.f, A.Z * -1.f, A.Y);
		FVector www = UKismetMathLibrary::SelectVector(AVec, BVec, bool(UKismetMathLibrary::Abs(A.X) > UKismetMathLibrary::Abs(A.Y)));
		FQuat QQQ = FQuat(www.X, www.Y, www.Z, W);

		UKismetMathLibrary::Quat_Normalize(QQQ, 0.0001);
		return FRotator(QQQ.Rotator());
	}
}

