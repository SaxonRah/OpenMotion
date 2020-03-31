// Fill out your copyright notice in the Description page of Project Settings.

#include "FabrikDebugComponent.h"

#include "FabrikStructure.h"
#include "FabrikChain.h"
#include "FabrikBone.h"
#include "FabrikUtil.h"
#include "FabrikMat3f.h"
#include "EJointType.h"
#include "EBoneConstraintType.h"

#include "DrawDebugHelpers.h"

#include "OpenMotion.h"

// Sets default values for this component's properties
UFabrikDebugComponent::UFabrikDebugComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	PointSize = 5.0f;
	LineThickness = 2.0f;
	DrawEnabled = true;

	// Constraint colours
	 ANTICLOCKWISE_CONSTRAINT_COLOUR = FColor(255, 0, 0);
	CLOCKWISE_CONSTRAINT_COLOUR = FColor(0, 0, 255);
	 BALL_JOINT_COLOUR = FColor(255, 0, 0);
	 GLOBAL_HINGE_COLOUR = FColor(255, 255, 0);
	 LOCAL_HINGE_COLOUR = FColor(0, 255, 255);
	 REFERENCE_AXIS_COLOUR = FColor(255, 0, 255);

	// The drawn length of the rotor cone and the radius of the cone and circle describing the hinge axes
	CONE_LENGTH_FACTOR = 0.3f;
	RADIUS_FACTOR = 0.25f;
	NUM_CONE_LINES = 12;
	
}


// Called when the game starts
void UFabrikDebugComponent::BeginPlay()
{
	Super::BeginPlay();

	rotStep = 360.0f / (float)NUM_CONE_LINES;
	
}


// Called every frame
void UFabrikDebugComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if (DrawEnabled == false || Structure == NULL)
		return;

	for (UFabrikChain* Chain : Structure->Chains)
	{
		DrawChainBones(Chain);
		DrawChainConstraints(Chain, 1);
	}
}


void UFabrikDebugComponent::DrawChainBones(UFabrikChain* Chain)
{

	for (int i = 0; i < Chain->NumBones; i++)
	{
		UFabrikBone* Bone = Chain->GetBone(i);
		//UFabrikBone* End = Chain->GetBone(i + 1);

		if (i == 0)
		{
			DrawDebugPoint(
				GetWorld(),
				Bone->StartLocation,
				PointSize,  					//size
				FColor(255, 0, 255),  //pink
				false//,  				//persistent (never goes away)
				//0.03 					//point leaves a trail on moving object
			);
		}
		DrawDebugPoint(
			GetWorld(),
			Bone->EndLocation,
			PointSize,  					//size
			FColor(255, 0, 255),  //pink
			false//,  				//persistent (never goes away)
			//0.03 					//point leaves a trail on moving object
		);

		DrawDebugLine(
			GetWorld(),
			Bone->StartLocation,
			Bone->EndLocation,
			Bone->Color, // FColor(255, 0, 0),
			false, -1, 0,
			LineThickness
		);

	}
}

void UFabrikDebugComponent::DrawConstraint(UFabrikBone* bone, FVector referenceDirection, float lineWidth/*, UFabrikMat3f* mvpMatrix*/)
{
	float boneLength = bone->Length;
	FVector lineStart = bone->StartLocation;

	switch (bone->Joint->JointType) // .getJointType())
	{
	case EJointType::JT_Ball: {
		float constraintAngleDegs = bone->Joint->RotorConstraintDegs; // GetBallJointConstraintDegs();

		// If the ball joint constraint is 180 degrees then it's not really constrained, so we won't draw it
		if (UFabrikUtil::ApproximatelyEquals(constraintAngleDegs, 180.0f, 0.01f)) { return; }

		// The constraint direction is the direction of the previous bone rotated about a perpendicular axis by the constraint angle of this bone
		FVector constraintDirection = UFabrikUtil::RotateAboutAxisDegs(referenceDirection, constraintAngleDegs, UFabrikUtil::VectorGenPerpendicularVectorQuick(referenceDirection));// .normalised();
		constraintDirection.Normalize();

		// Draw the lines about the the bone (relative to the reference direction)				
		FVector lineEnd;
		for (int loop = 0; loop < NUM_CONE_LINES; ++loop)
		{
			lineEnd = lineStart + (constraintDirection * (boneLength * CONE_LENGTH_FACTOR));
			constraintDirection = UFabrikUtil::RotateAboutAxisDegs(constraintDirection, rotStep, referenceDirection);// .normalised();
			DrawLine(lineStart, lineEnd, BALL_JOINT_COLOUR, lineWidth);// , mvpMatrix);
		}

		// Draw the circle at the top of the cone
		float pushDistance = (float)FMath::Cos(FMath::DegreesToRadians( constraintAngleDegs )) * boneLength;
		float radius = (float)FMath::Sin( FMath::DegreesToRadians( constraintAngleDegs)) * boneLength;
		FVector circleCentre = lineStart + (referenceDirection * (pushDistance * CONE_LENGTH_FACTOR));
		DrawCircle(circleCentre, referenceDirection, radius * CONE_LENGTH_FACTOR, BALL_JOINT_COLOUR, lineWidth);// , mvpMatrix);
		break;
	}
	case EJointType::JT_GlobalHinge: {
		// Get the hinge rotation axis and draw the circle describing the hinge rotation axis
		FVector hingeRotationAxis = bone->Joint->RotationAxisUV;// .getJoint().getHingeRotationAxis();
		float radius = boneLength * RADIUS_FACTOR;
		DrawCircle(lineStart, hingeRotationAxis, radius, GLOBAL_HINGE_COLOUR, lineWidth);// , mvpMatrix);

		// Note: While ACW rotation is negative and CW rotation about an axis is positive, we store both 
		// of these as positive values between the range 0 to 180 degrees, as such we'll negate the
		// clockwise rotation value for it to turn in the correct direction.
		float anticlockwiseConstraintDegs = bone->Joint->HingeAnticlockwiseConstraintDegs;// getHingeJointAnticlockwiseConstraintDegs();
		float clockwiseConstraintDegs = -bone->Joint->HingeClockwiseConstraintDegs;// getHingeJointClockwiseConstraintDegs();

		// If both the anticlockwise (positive) and clockwise (negative) constraint angles are not 180 degrees (i.e. we
		// are constraining the hinge about a reference direction which lies in the plane of the hinge rotation axis)...
		if (!UFabrikUtil::ApproximatelyEquals(anticlockwiseConstraintDegs, 180.0f, 0.01f) &&
			!UFabrikUtil::ApproximatelyEquals(clockwiseConstraintDegs, 180.0f, 0.01f))
		{
			FVector hingeReferenceAxis = bone->Joint->ReferenceAxisUV;// HingeReferenceAxis;

			// ...then draw the hinge reference axis and ACW/CW constraints about it.
			DrawLine(lineStart, lineStart + (hingeReferenceAxis * (boneLength * RADIUS_FACTOR)), REFERENCE_AXIS_COLOUR, lineWidth);// , mvpMatrix);
			FVector anticlockwiseDirection = UFabrikUtil::RotateAboutAxisDegs(hingeReferenceAxis, anticlockwiseConstraintDegs, hingeRotationAxis);
			FVector clockwiseDirection = UFabrikUtil::RotateAboutAxisDegs(hingeReferenceAxis, clockwiseConstraintDegs, hingeRotationAxis);
			FVector anticlockwisePoint = lineStart + (anticlockwiseDirection * (radius));
			FVector clockwisePoint = lineStart + (clockwiseDirection * (radius));
			DrawLine(lineStart, anticlockwisePoint, ANTICLOCKWISE_CONSTRAINT_COLOUR, lineWidth);// , mvpMatrix);
			DrawLine(lineStart, clockwisePoint, CLOCKWISE_CONSTRAINT_COLOUR, lineWidth);// , mvpMatrix);
		}
		break;
	}
	case EJointType::JT_LocalHinge: {
		// Construct a rotation matrix based on the reference direction (i.e. the previous bone's direction)...
		UFabrikMat3f* m = UFabrikMat3f::CreateRotationMatrix(referenceDirection);

		// ...and transform the hinge rotation axis into the previous bone's frame of reference
		FVector relativeHingeRotationAxis = m->Times(bone->Joint->RotationAxisUV);// GetHingeRotationAxis());// .normalise();
		relativeHingeRotationAxis.Normalize();

		// Draw the circle describing the hinge rotation axis
		float radius = boneLength * RADIUS_FACTOR;
		DrawCircle(lineStart, relativeHingeRotationAxis, radius, LOCAL_HINGE_COLOUR, lineWidth);// , mvpMatrix);

		// Draw the hinge reference and clockwise/anticlockwise constraints if necessary
		float anticlockwiseConstraintDegs = bone->Joint->HingeAnticlockwiseConstraintDegs;// getHingeJointAnticlockwiseConstraintDegs();
		float clockwiseConstraintDegs = -bone->Joint->HingeClockwiseConstraintDegs; //  .getHingeJointClockwiseConstraintDegs();
		if (!UFabrikUtil::ApproximatelyEquals(anticlockwiseConstraintDegs, 180.0f, 0.01f) &&
			!UFabrikUtil::ApproximatelyEquals(clockwiseConstraintDegs, 180.0f, 0.01f))
		{
			// Get the relative hinge rotation axis and draw it...
			FVector relativeHingeReferenceAxis = UFabrikUtil::ProjectOntoPlane(bone->Joint->ReferenceAxisUV, relativeHingeRotationAxis); //;getHingeReferenceAxis().projectOntoPlane(relativeHingeRotationAxis);
			relativeHingeReferenceAxis = m->Times(bone->Joint->ReferenceAxisUV);// getHingeReferenceAxis());// .normalise();
			relativeHingeReferenceAxis.Normalize();

			DrawLine(lineStart, lineStart + (relativeHingeReferenceAxis * radius), REFERENCE_AXIS_COLOUR, lineWidth);// , mvpMatrix);

			// ...as well as the clockwise and anticlockwise constraints.
			FVector anticlockwiseDirection = UFabrikUtil::RotateAboutAxisDegs(relativeHingeReferenceAxis, anticlockwiseConstraintDegs, relativeHingeRotationAxis);
			FVector clockwiseDirection = UFabrikUtil::RotateAboutAxisDegs(relativeHingeReferenceAxis, clockwiseConstraintDegs, relativeHingeRotationAxis);
			FVector anticlockwisePoint = lineStart + (anticlockwiseDirection * radius);
			FVector clockwisePoint = lineStart + (clockwiseDirection * radius);
			DrawLine(lineStart, anticlockwisePoint, ANTICLOCKWISE_CONSTRAINT_COLOUR, lineWidth);// , mvpMatrix);
			DrawLine(lineStart, clockwisePoint, CLOCKWISE_CONSTRAINT_COLOUR, lineWidth);// , mvpMatrix);
		}
		break;
	}

	} // End of switch statement
}

void UFabrikDebugComponent::DrawLine(FVector Start, FVector End, FColor Color, float lineWidth)
{
	DrawDebugLine(
		GetWorld(),
		Start,
		End,
		Color, // 
		false, -1, 0//,
		//1 lineWidth
	);
}

void UFabrikDebugComponent::DrawCircle(FVector Location, FVector Axis, float Radius, FColor Color, float LineWidth)
{
	int NUM_VERTICES = NUM_CONE_LINES; //##

	FVector firstPoint;
	FVector prevPoint;
	// Generate the circle data and put it into the FloatBuffer
	for (int vertexNumLoop = 0; vertexNumLoop < NUM_VERTICES; vertexNumLoop++)
	{
		// Create our circle in the plane perpendicular to the axis provided
		float angleDegs = vertexNumLoop * (360.0f / (float)NUM_VERTICES);
		FVector perpAxis = UFabrikUtil::VectorGenPerpendicularVectorQuick(Axis);
		FVector point(Radius * perpAxis.X, Radius * perpAxis.Y, Radius * perpAxis.Z);

		// Rotate each point about the axis given
		point = UFabrikUtil::RotateAboutAxisDegs(point, angleDegs, Axis);

		// Translate to the given location
		point = point + Location;

		if (vertexNumLoop == 0)
		{
			firstPoint = point;
		}
		else if (vertexNumLoop == NUM_VERTICES - 1)
		{
			// last point
			DrawLine(prevPoint, point, Color, LineWidth);
			DrawLine(point, firstPoint, Color, LineWidth);
		}
		else
		{
			DrawLine(prevPoint, point,  Color, LineWidth);
		}
		
		prevPoint = point;
		// Point x/y/z
		//circleData[(vertexNumLoop * VERTEX_COMPONENTS)] = point.x;
		//circleData[(vertexNumLoop * VERTEX_COMPONENTS) + 1] = point.y;
		//circleData[(vertexNumLoop * VERTEX_COMPONENTS) + 2] = point.z;
	}

}


void UFabrikDebugComponent::DrawChainConstraints(UFabrikChain* Chain, float LineWidth) // , Mat4f mvpMatrix
{
	int numBones = Chain->NumBones;
	if (numBones > 0)
	{
		// Draw the base bone, using the constraint UV as the relative direction
		switch (Chain->BaseboneConstraintType)
		{
		case EBoneConstraintType::BCT_None:
			break;
		case EBoneConstraintType::BCT_GlobalRotor:// GLOBAL_ROTOR:
		case EBoneConstraintType::BCT_GlobalHinge: // GLOBAL_HINGE:
			DrawConstraint(Chain->GetBone(0), Chain->BaseboneConstraintUV, LineWidth);// , mvpMatrix);
			break;
		case EBoneConstraintType::BCT_LocalRotor:// LOCAL_ROTOR:
		case EBoneConstraintType::BCT_LocalHinge: // LOCAL_HINGE:
			// If the structure hasn't been solved yet then we won't have a relative basebone constraint which we require
			// to draw the constraint itself - so our best option is to simply not draw the constraint until we can. 
			if (Chain->BaseboneRelativeConstraintUV.Size() > 0.0f)
			{
				DrawConstraint(Chain->GetBone(0), Chain->BaseboneRelativeConstraintUV, LineWidth);// , mvpMatrix);
			}
			break;
			// No need for a default - constraint types are enums and we've covered them all.
		}

		// Draw all the bones AFTER the base bone, using the previous bone direction as the relative direction
		for (int loop = 1; loop < numBones; ++loop)
		{
			DrawConstraint(Chain->GetBone(loop), Chain->GetBone(loop - 1)->GetDirectionUV(), LineWidth);// , mvpMatrix);
		}

	}

}


/**
public void drawBone(FabrikBone3D bone, Mat4f viewMatrix, Mat4f projectionMatrix, Colour4f colour)

{

	// Clone the model and scale the clone to be twice as wide and deep, and scaled along the z-axis to match the bone length

	Model modelCopy = Model.clone(model);

	modelCopy.scale(2.0f, 2.0f, bone.length());



	// Get our scaled model data

	modelData = modelCopy.getVertexFloatArray();



	// Construct a model matrix for this bone

	Mat4f modelMatrix = new Mat4f(Mat3f.createRotationMatrix(bone.getDirectionUV().normalised()), bone.getStartLocation());



	// Construct a ModelViewProjection and draw the model for this bone

	Mat4f mvpMatrix = projectionMatrix.times(viewMatrix).times(modelMatrix);

	this.drawModel(mLineWidth, colour, mvpMatrix);

}
*/