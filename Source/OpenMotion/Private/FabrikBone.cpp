// Fill out your copyright notice in the Description page of Project Settings.
// https://github.com/FedUni/caliko/blob/master/caliko/src/au/edu/federation/caliko/FabrikBone3D.java

#include "FabrikBone.h"

#include "OpenMotion.h"


UFabrikBone::UFabrikBone(const FObjectInitializer& ObjectInitializer)
{
	BoneConnectionPoint = EBoneConnectionPoint::BCP_End;
	Joint = NewObject<UFabrikJoint>();
}


UFabrikBone* UFabrikBone::Init(FVector InStartLocation, FVector InEndLocation)

{
	this->StartLocation = InStartLocation;
	this->EndLocation = InEndLocation;
	Length = FVector::Dist(InStartLocation, InEndLocation);
	return this;
}

UFabrikBone* UFabrikBone::Init(FVector InStartLocation, FVector InEndLocation, FName InName)
{
	Init(InStartLocation, InEndLocation);
	this->Name  = InName;
	return this;
}

UFabrikBone* UFabrikBone::Init(FVector InStartLocation, FVector InDirectionUV, float InLength)
{
	if (!(InDirectionUV.Size() > 0.0f))
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("Direction cannot be a zero vector!"));
	}

	this->Length = InLength;
	this->StartLocation = InStartLocation;
	InDirectionUV.Normalize();
	this->EndLocation = InStartLocation + (InDirectionUV * InLength);
	return this;
}

UFabrikBone* UFabrikBone::Init(FVector InStartLocation, FVector InDirectionUV, float InLength, FName InName)
{
	Init(InStartLocation, InDirectionUV, InLength);
	this->Name = InName;
	return this;
}

UFabrikBone* UFabrikBone::Init(FVector InStartLocation, FVector InDirectionUV, float InLength, FColor InColor)
{

	Init(InStartLocation, InDirectionUV, InLength);
	Color = InColor;
	return this;
}

UFabrikBone* UFabrikBone::Init(UFabrikBone* Source)
{
	StartLocation = Source->StartLocation;
	EndLocation = Source->EndLocation;

	// TODO: CHECK: Clone instead of ref
	UFabrikJoint* NewJoint = NewObject<UFabrikJoint>();
	Joint = NewJoint->Init(Source->Joint);
	//Joint = Source->Joint; // Direct copy? Maybe make a clone instead?
	//# TODO: Joint.set(source.mJoint);

	Color = Source->Color;
	Name = Source->Name;
	Length = Source->Length;
	LineWidth = Source->LineWidth;
	BoneConnectionPoint = Source->BoneConnectionPoint;
	return this;
}

float UFabrikBone::LiveLength() 
{ 
	return FVector::Dist(StartLocation, EndLocation);
}

TArray<float>  UFabrikBone::GetStartLocationAsArray()
{ 
	TArray<float> Ret;
	Ret.Add(StartLocation.X);
	Ret.Add(StartLocation.Y);
	Ret.Add(StartLocation.Z);
	return Ret;
	//return new float[] { StartLocation.x, StartLocation.y, StartLocation.z }; 
}

TArray<float> UFabrikBone::GetEndLocationAsArray()
{
	TArray<float> Ret;
	Ret.Add(StartLocation.X);
	Ret.Add(StartLocation.Y);
	Ret.Add(StartLocation.Z);
	return Ret;
	//return new float[] { StartLocation.x, StartLocation.y, StartLocation.z }; 
}

/**
void UFabrikBone::SetLineWidth(float InLineWidth)
{
	// If the specified argument is within the valid range then set it...
	/*if (lineWidth >= FabrikBone3D.MIN_LINE_WIDTH && lineWidth <= FabrikBone3D.MAX_LINE_WIDTH)
	{
	*/
		//mLineWidth = lineWidth;
	/*}
	else // ...otherwise throw an IllegalArgumentException.
	{

		throw new IllegalArgumentException("Line width must be between " +

			FabrikBone3D.MIN_LINE_WIDTH + " and " +

			FabrikBone3D.MAX_LINE_WIDTH + " inclusive.");

	}*
}*/

FString UFabrikBone::GetDebugMsg()
{
	//Vec3f direction = this.getDirectionUV();
	/**
	StringBuilder sb = new StringBuilder();
	sb.append("Start joint location : " + mStartLocation + NEW_LINE);
	sb.append("End   joint location : " + mEndLocation + NEW_LINE);
	sb.append("Bone length          : " + mLength + NEW_LINE);
	sb.append("Colour               : " + mColour + NEW_LINE);
	return sb.toString();*/
	return "TODO";
}