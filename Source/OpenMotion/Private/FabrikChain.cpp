// Fill out your copyright notice in the Description page of Project Settings.

#include "FabrikChain.h"
#include "FabrikBone.h"
#include "FabrikJoint.h"
#include "EJointType.h"
#include "FabrikUtil.h"
#include "FabrikMat3f.h"
#include "FabrikStructure.h"

#include "OpenMotion.h"

UFabrikChain::UFabrikChain(const FObjectInitializer& ObjectInitializer)
{
	SolveDistanceThreshold = 0.1f;
	MaxIterationAttempts = 20;
	MinIterationChange = 0.01f;
	ChainLength = 0;
	NumBones = 0;
	FixedBaseLocation = FVector::ZeroVector;
	FixedBaseMode = true;
	BaseboneConstraintType = EBoneConstraintType::BCT_None;
	BaseboneConstraintUV = FVector::ZeroVector;
	BaseboneRelativeConstraintUV = FVector::ZeroVector;
	BaseboneRelativeReferenceConstraintUV = FVector::ZeroVector;
	LastTargetLocation = CreateMaxVector();
	ConstraintLineWidth = 2.0f;
	LastBaseLocation = CreateMaxVector(); 
	CurrentSolveDistance = FloatMax();
	ConnectedChainNumber = -1;
	ConnectedBoneNumber = -1;
	EmbeddedTarget = FVector::ZeroVector;
	UseEmbeddedTarget = false;
}

FVector UFabrikChain::GetBaseLocation() 
{ 
	return Chain[0]->StartLocation;
}

void UFabrikChain::Init(UFabrikChain* InSource)
{
	// Force copy by value
	Chain = InSource->CloneIkChain();
	FixedBaseLocation = InSource->GetBaseLocation();
	LastTargetLocation = InSource->LastTargetLocation;
	LastBaseLocation = InSource->LastBaseLocation;
	EmbeddedTarget = InSource->EmbeddedTarget;

	// Copy the basebone constraint UV if there is one to copy
	if (InSource->BaseboneConstraintType != EBoneConstraintType::BCT_None)
	{
		BaseboneConstraintUV = InSource->BaseboneConstraintUV;
		BaseboneRelativeConstraintUV = InSource->BaseboneRelativeConstraintUV;
	}

	// Native copy by value for primitive members
	ChainLength = InSource->ChainLength;
	NumBones = InSource->NumBones;
	CurrentSolveDistance = InSource->CurrentSolveDistance;
	ConnectedChainNumber = InSource->ConnectedChainNumber;
	ConnectedBoneNumber = InSource->ConnectedBoneNumber;
	BaseboneConstraintType = InSource->BaseboneConstraintType;
	Name = InSource->Name;
	ConstraintLineWidth = InSource->ConstraintLineWidth;
	UseEmbeddedTarget = InSource->UseEmbeddedTarget;
}

UFabrikChain* UFabrikChain::Init(FName InName) 
{ 
	Name = InName; 
	return this;
}

void UFabrikChain::AddBone(UFabrikBone* InBone)
{
	// Add the new bone to the end of the ArrayList of bones
	Chain.Add(InBone);

	// If this is the basebone...
	if (NumBones == 0)
	{
		// ...then keep a copy of the fixed start location...
		FixedBaseLocation = InBone->StartLocation;

		// ...and set the basebone constraint UV to be around the initial bone direction
		BaseboneConstraintUV = InBone->GetDirectionUV();
	}

	// Increment the number of bones in the chain and update the chain length
	++NumBones;
	UpdateChainLength();
}

void UFabrikChain::AddConsecutiveBone(FVector InDirectionUV, float InLength, FColor InColour)
{
	// Validate the direction unit vector - throws an IllegalArgumentException if it has a magnitude of zero
	UFabrikUtil::ValidateDirectionUV(InDirectionUV);

	// Validate the length of the bone - throws an IllegalArgumentException if it is not a positive value
	UFabrikUtil::ValidateLength(InLength);

	// If we have at least one bone already in the chain...
	if (NumBones > 0)
	{
		// Get the end location of the last bone, which will be used as the start location of the new bone
		FVector PrevBoneEnd = Chain[NumBones - 1]->EndLocation;// getEndLocation();

		// Add a bone to the end of this IK chain
		// Note: We use a normalised version of the bone direction
		UFabrikBone* NewBone = NewObject<UFabrikBone>();
		FVector DirN = InDirectionUV;
		DirN.Normalize();
		NewBone->Init(PrevBoneEnd, DirN, InLength, InColour);
		AddBone(NewBone);// new FabrikBone3D(prevBoneEnd, directionUV.normalised(), length, colour))
	}
	else // Attempting to add a relative bone when there is no basebone for it to be relative to?
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("You cannot add the basebone as a consecutive bone as it does not provide a start location. Use the addBone() method instead."));
		//throw new RuntimeException("You cannot add the basebone as a consecutive bone as it does not provide a start location. Use the addBone() method instead.");
	}
}

void UFabrikChain::AddConsecutiveBone(FVector InDirectionUV, float InLength)
{ 
	AddConsecutiveBone(InDirectionUV, InLength, FColor());
}

void UFabrikChain::AddConsecutiveFreelyRotatingHingedBone(FVector InDirectionUV, float InLength, EJointType InJointType, FVector InHingeRotationAxis)
{
	// Because we aren't constraining this bone to a reference axis within the hinge rotation axis we don't care about the hinge constraint
	// reference axis (7th param) so we'll just generate an axis perpendicular to the hinge rotation axis and use that.
	FVector Perp = UFabrikUtil::VectorGenPerpendicularVectorQuick(InHingeRotationAxis);
	AddConsecutiveHingedBoneC(InDirectionUV, InLength, InJointType, InHingeRotationAxis, 180.0f, 180.0f, Perp, FColor());
}

void UFabrikChain::AddConsecutiveFreelyRotatingHingedBoneC(FVector InDirectionUV, float InLength, EJointType InJointType, FVector InHingeRotationAxis, FColor InColour)
{
	// Because we aren't constraining this bone to a reference axis within the hinge rotation axis we don't care about the hinge constraint
	// reference axis (7th param) so we'll just generate an axis perpendicular to the hinge rotation axis and use that.
	FVector Perp = UFabrikUtil::VectorGenPerpendicularVectorQuick(InHingeRotationAxis);
	AddConsecutiveHingedBoneC(InDirectionUV, InLength, InJointType, InHingeRotationAxis, 180.0f, 180.0f, Perp, InColour);
}

void UFabrikChain::AddConsecutiveHingedBoneC(FVector InDirectionUV,
	float InLength,
	EJointType InJointType,
	FVector InHingeRotationAxis,
	float InClockwiseDegs,
	float InAnticlockwiseDegs,
	FVector InHingeReferenceAxis,
	FColor InColour)
{

	// Validate the direction and rotation axis unit vectors, and the length of the bone.
	UFabrikUtil::ValidateDirectionUV(InDirectionUV);
	UFabrikUtil::ValidateDirectionUV(InHingeRotationAxis);
	UFabrikUtil::ValidateLength(InLength);

	// Cannot add a consectuive bone of any kind if the there is no basebone
	if (NumBones == 0)
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("You must add a basebone before adding a consectutive bone."));
	}

	// Normalise the direction and hinge rotation axis 
	InDirectionUV.Normalize();
	InHingeRotationAxis.Normalize();

	// Get the end location of the last bone, which will be used as the start location of the new bone
	FVector PrevBoneEnd = Chain[NumBones - 1]->EndLocation;

	// Create a bone and set the draw colour...
	UFabrikBone* Bone = NewObject<UFabrikBone>(); //new FabrikBone3D(prevBoneEnd, directionUV, length);
	Bone->Init(PrevBoneEnd, InDirectionUV, InLength);
	Bone->Color = InColour;

	// ...then create and set up a joint which we'll apply to that bone.
	UFabrikJoint* Joint = NewObject<UFabrikJoint>();  

	switch (InJointType)
	{
	case EJointType::JT_GlobalHinge :// GLOBAL_HINGE:
		Joint->SetAsGlobalHinge(InHingeRotationAxis, InClockwiseDegs, InAnticlockwiseDegs, InHingeReferenceAxis);
		break;
	case EJointType::JT_LocalHinge: // LOCAL_HINGE:
		Joint->SetAsLocalHinge(InHingeRotationAxis, InClockwiseDegs, InAnticlockwiseDegs, InHingeReferenceAxis);
		break;
	default:
		UE_LOG(OpenMotionLog, Fatal, TEXT("Hinge joint types may be only JointType.GLOBAL_HINGE or JointType.LOCAL_HINGE."));
	}

	// Set the joint we just set up on the the new bone we just created
	Bone->Joint = Joint; // etJoint(joint);

	// Finally, add the bone to this chain
	AddBone(Bone);
}

void UFabrikChain::AddConsecutiveHingedBone(FVector InDirectionUV,
	float InLength,
	EJointType InJointType,
	FVector InHingeRotationAxis,
	float InClockwiseDegs,
	float InAnticlockwiseDegs,
	FVector InHingeConstraintReferenceAxis)
{
	AddConsecutiveHingedBoneC(InDirectionUV, InLength, InJointType, InHingeRotationAxis, InClockwiseDegs, InAnticlockwiseDegs, InHingeConstraintReferenceAxis, FColor());
}

void UFabrikChain::AddConsecutiveRotorConstrainedBoneC(FVector InBoneDirectionUV, float InBoneLength, float InConstraintAngleDegs, FColor InColor)
{
	// Validate the bone direction and length and that we have a basebone
	UFabrikUtil::ValidateDirectionUV(InBoneDirectionUV);
	UFabrikUtil::ValidateLength(InBoneLength);

	if (NumBones == 0)
	{ 
		UE_LOG(OpenMotionLog, Fatal, TEXT("Add a basebone before attempting to add consectuive bones."));  //throw new RuntimeException("Add a basebone before attempting to add consectuive bones.");
	}

	// Create the bone starting at the end of the previous bone, set its direction, constraint angle and colour

	// then add it to the chain. Note: The default joint type of a new FabrikBone3D is JointType.BALL.
	InBoneDirectionUV.Normalize();
	UFabrikBone* Bone = NewObject<UFabrikBone>();// new FabrikBone3D(mChain.get(mNumBones - 1).getEndLocation(), InBoneDirectionUV, InBoneLength, InColor);
	Bone->Init(Chain[NumBones - 1]->EndLocation, InBoneDirectionUV, InBoneLength, InColor);

	//## setBallJointConstraintDegs => mRotorConstraintDegs 
	Bone->Joint->RotorConstraintDegs = InConstraintAngleDegs;

	AddBone(Bone);
}

void UFabrikChain::AddConsecutiveRotorConstrainedBone(FVector InBoneDirectionUV, float InBoneLength, float InConstraintAngleDegs)
{
	AddConsecutiveRotorConstrainedBoneC(InBoneDirectionUV, InBoneLength, InConstraintAngleDegs, FColor());
}

UFabrikBone* UFabrikChain::GetBone(int InBoneNumber) { return Chain[InBoneNumber]; }

FVector UFabrikChain::GetEffectorLocation() { return Chain[NumBones - 1]->EndLocation; }

float UFabrikChain::GetLiveChainLength()
{
	float length = 0.0f;
	for (int loop = 0; loop < NumBones; ++loop)
	{
		length += Chain[loop]->LiveLength();
	}
	return length;
}

void UFabrikChain::RemoveBone(int InBoneNumber)
{
	// If the bone number is a bone which exists...
	if (InBoneNumber < NumBones)
	{
		// ...then remove the bone, decrease the bone count and update the chain length.
		Chain.RemoveAt(InBoneNumber);
		--NumBones;
		UpdateChainLength();
	}
	else
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("Bone TODO does not exist to be removed from the chain. Bones are zero indexed.")); //throw new IllegalArgumentException("Bone " + boneNumber + " does not exist to be removed from the chain. Bones are zero indexed.");
	}
}

void UFabrikChain::SetRotorBaseboneConstraint(EBoneConstraintType InRotorType, FVector InConstraintAxis, float InAngleDegs)
{
	// Sanity checking
	if (NumBones == 0) 
	{ 
		UE_LOG(OpenMotionLog, Fatal, TEXT("Chain must contain a basebone before we can specify the basebone constraint type.")); //
		//throw new RuntimeException("Chain must contain a basebone before we can specify the basebone constraint type."); 
	}

	if (!(InConstraintAxis.Size() > 0.0f))
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("Constraint axis cannot be zero.")); //
		//throw new IllegalArgumentException("Constraint axis cannot be zero."); 
	}

	if (InAngleDegs < 0.0f) { InAngleDegs = 0.0f; }

	if (InAngleDegs > 180.0f) { InAngleDegs = 180.0f; }

	if (!(InRotorType == EBoneConstraintType::BCT_GlobalRotor || InRotorType == EBoneConstraintType::BCT_LocalRotor))
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("The only valid rotor types for this method are GLOBAL_ROTOR and LOCAL_ROTOR.")); //
		//throw new IllegalArgumentException("The only valid rotor types for this method are GLOBAL_ROTOR and LOCAL_ROTOR.");
	}

	// Set the constraint type, axis and angle
	BaseboneConstraintType = InRotorType;
	InConstraintAxis.Normalize();
	BaseboneConstraintUV = InConstraintAxis;// .normalised();
	BaseboneRelativeConstraintUV = BaseboneConstraintUV; // .set(mBaseboneConstraintUV);
	GetBone(0)->Joint->SetAsBallJoint(InAngleDegs);
}

void UFabrikChain::SetHingeBaseboneConstraint(EBoneConstraintType InHingeType, FVector InHingeRotationAxis, float InCwConstraintDegs, float InAcwConstraintDegs, FVector InHingeReferenceAxis)
{
	// Sanity checking
	if (NumBones == 0) 
	{ 
		UE_LOG(OpenMotionLog, Fatal, TEXT("Chain must contain a basebone before we can specify the basebone constraint type.")); // throw new RuntimeException("Chain must contain a basebone before we can specify the basebone constraint type."); 
	}

	if (!(InHingeRotationAxis.Size() > 0.0f)) 
	{ 
		UE_LOG(OpenMotionLog, Fatal, TEXT("Hinge rotation axis cannot be zero.")); // throw new IllegalArgumentException("Hinge rotation axis cannot be zero."); 
	}

	if (!(InHingeReferenceAxis.Size() > 0.0f))
	{ 
		UE_LOG(OpenMotionLog, Fatal, TEXT("Hinge reference axis cannot be zero.")); //throw new IllegalArgumentException("Hinge reference axis cannot be zero."); 
	}

	if (!(UFabrikUtil::VectorPerpendicular(InHingeRotationAxis, InHingeReferenceAxis))) // Vec3f.perpendicular(hingeRotationAxis, hingeReferenceAxis)))
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("The hinge reference axis must be in the plane of the hinge rotation axis, that is, they must be perpendicular.")); //throw new IllegalArgumentException("The hinge reference axis must be in the plane of the hinge rotation axis, that is, they must be perpendicular.");
	}

	if (!(InHingeType == EBoneConstraintType::BCT_GlobalHinge || InHingeType == EBoneConstraintType::BCT_LocalHinge))
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("The only valid hinge types for this method are GLOBAL_HINGE and LOCAL_HINGE.")); //throw new IllegalArgumentException("The only valid hinge types for this method are GLOBAL_HINGE and LOCAL_HINGE.");
	}

	// Set the constraint type, axis and angle
	BaseboneConstraintType = InHingeType;
	BaseboneConstraintUV = InHingeRotationAxis; //.set(hingeRotationAxis.normalised());
	BaseboneConstraintUV.Normalize();
	UFabrikJoint* Hinge = NewObject<UFabrikJoint>();// new FabrikJoint3D();

	if (InHingeType == EBoneConstraintType::BCT_GlobalHinge) // .GLOBAL_HINGE)
	{
		Hinge->SetHinge(EJointType::JT_GlobalHinge, InHingeRotationAxis, InCwConstraintDegs, InAcwConstraintDegs, InHingeReferenceAxis);
	}
	else
	{
		Hinge->SetHinge(EJointType::JT_LocalHinge, InHingeRotationAxis, InCwConstraintDegs, InAcwConstraintDegs, InHingeReferenceAxis);
	}

	GetBone(0)->Joint = Hinge;// .setJoint(hinge);
}

void UFabrikChain::SetFreelyRotatingGlobalHingedBasebone(FVector InHingeRotationAxis)
{
	SetHingeBaseboneConstraint(EBoneConstraintType::BCT_GlobalHinge, InHingeRotationAxis, 180.0f, 180.0f, UFabrikUtil::VectorGenPerpendicularVectorQuick(InHingeRotationAxis));
}

void UFabrikChain::SetFreelyRotatingLocalHingedBasebone(FVector InHingeRotationAxis)
{
	SetHingeBaseboneConstraint(EBoneConstraintType::BCT_LocalHinge, InHingeRotationAxis, 180.0f, 180.0f, UFabrikUtil::VectorGenPerpendicularVectorQuick(InHingeRotationAxis));
}

void UFabrikChain::SetLocalHingedBasebone(FVector InHingeRotationAxis, float InCwDegs, float InAcwDegs, FVector InHingeReferenceAxis)
{
	SetHingeBaseboneConstraint(EBoneConstraintType::BCT_LocalHinge, InHingeRotationAxis, InCwDegs, InAcwDegs, InHingeReferenceAxis);
}

void UFabrikChain::SetGlobalHingedBasebone(FVector InHingeRotationAxis, float InCwDegs, float InAcwDegs, FVector InHingeReferenceAxis)
{
	SetHingeBaseboneConstraint(EBoneConstraintType::BCT_GlobalHinge, InHingeRotationAxis, InCwDegs, InAcwDegs, InHingeReferenceAxis);
}

/**
void UFabrikChain::SetBaseboneConstraintUV(FVector InConstraintUV)
{
	if (BaseboneConstraintType == EBoneConstraintType::BCT_None)
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("Specify the basebone constraint type with setBaseboneConstraintTypeCannot specify a basebone constraint when the current constraint type is BaseboneConstraint.NONE."));
		//throw new IllegalArgumentException("Specify the basebone constraint type with setBaseboneConstraintTypeCannot specify a basebone constraint when the current constraint type is BaseboneConstraint.NONE.");
	}

	// Validate the constraint direction unit vector
	UFabrikUtil::ValidateDirectionUV(InConstraintUV);

	// All good? Then normalise the constraint direction and set it
	InConstraintUV.Normalize();
	BaseboneConstraintUV = InConstraintUV;
}*/

void UFabrikChain::SetColour(FColor InColour)
{
	for (int Loop = 0; Loop < NumBones; ++Loop)
	{
		GetBone(Loop)->Color = InColour;
	}
}

float UFabrikChain::SolveForEmbeddedTarget()
{
	if (UseEmbeddedTarget)
	{ 
		return SolveForTarget(EmbeddedTarget);
	}
	else 
	{ 
		UE_LOG(OpenMotionLog, Fatal, TEXT("This chain does not have embedded targets enabled - enable with setEmbeddedTargetMode(true).")); //throw new RuntimeException("This chain does not have embedded targets enabled - enable with setEmbeddedTargetMode(true).");
		return 0;
	}
}

/**
public float solveForTarget(float targetX, float targetY, float targetZ)
{
	return solveForTarget(new Vec3f(targetX, targetY, targetZ));
}*/

float UFabrikChain::SolveForTarget(FVector InNewTarget)
{
	// If we have both the same target and base location as the last run then do not solve
	if (UFabrikUtil::VectorApproximatelyEquals(LastTargetLocation, InNewTarget, 0.001f) && // LastTargetLocation.approximatelyEquals(newTarget, 0.001f) &&
		UFabrikUtil::VectorApproximatelyEquals(LastBaseLocation, GetBaseLocation(), 0.001f)) //LastBaseLocation.approximatelyEquals(getBaseLocation(), 0.001f))
	{
		return CurrentSolveDistance;
	}

	/***
	* NOTE: We must allow the best solution of THIS run to be used for a new target or base location - we cannot
	* just use the last solution (even if it's better) - because that solution was for a different target / base
	* location combination and NOT for the current setup.
	*/

	// Declare a list of bones to use to store our best solution
	TArray<UFabrikBone*> BestSolution;// = new ArrayList<FabrikBone3D>();

	// We start with a best solve distance that can be easily beaten
	float BestSolveDistance = FloatMax();// Float.MAX_VALUE;

	// We'll also keep track of the solve distance from the last pass
	float LastPassSolveDistance = FloatMax();// Float.MAX_VALUE;

	// Allow up to our iteration limit attempts at solving the chain
	float SolveDistance;

	for (int Loop = 0; Loop < MaxIterationAttempts; ++Loop)
	{
		// Solve the chain for this target
		SolveDistance = SolveIK(InNewTarget);

		// Did we solve it for distance? If so, update our best distance and best solution, and also
		// update our last pass solve distance. Note: We will ALWAYS beat our last solve distance on the first run. 
		if (SolveDistance < BestSolveDistance)
		{
			BestSolveDistance = SolveDistance;
			BestSolution = this->CloneIkChain();

			// If we are happy that this solution meets our distance requirements then we can exit the loop now
			if (SolveDistance < SolveDistanceThreshold)
			{
				break;
			}
		}
		else // Did not solve to our satisfaction? Okay...
		{
			// Did we grind to a halt? If so break out of loop to set the best distance and solution that we have
			if (FMath::Abs(SolveDistance - LastPassSolveDistance) < MinIterationChange)
			{
				//System.out.println("Ground to halt on iteration: " + loop);
				break;
			}
		}

		// Update the last pass solve distance
		LastPassSolveDistance = SolveDistance;
	} // End of loop

	  // Update our solve distance and chain configuration to the best solution found
	CurrentSolveDistance = BestSolveDistance;
	Chain = BestSolution;

	// Update our base and target locations
	LastBaseLocation = GetBaseLocation(); // .set(getBaseLocation());
	LastTargetLocation = InNewTarget; // .set(newTarget);

	return CurrentSolveDistance;
}

float UFabrikChain::SolveIK(FVector InTarget)
{
	// Sanity check that there are bones in the chain
	if (NumBones == 0) 
	{ 
		UE_LOG(OpenMotionLog, Fatal, TEXT("It makes no sense to solve an IK chain with zero bones."));
	}

	// ---------- Forward pass from end effector to base -----------

	// Loop over all bones in the chain, from the end effector (numBones-1) back to the basebone (0)		
	for (int Loop = NumBones - 1; Loop >= 0; --Loop)
	{
		// Get the length of the bone we're working on
		UFabrikBone* ThisBone = Chain[Loop];// Get(loop);
		float ThisBoneLength = ThisBone->Length;// ();
		UFabrikJoint* ThisBoneJoint = ThisBone->Joint;// .getJoint();
		EJointType ThisBoneJointType = ThisBone->Joint->JointType;//.getJointType();

		// If we are NOT working on the end effector bone
		if (Loop != NumBones - 1)
		{
			// Get the outer-to-inner unit vector of the bone further out
			FVector OuterBoneOuterToInnerUV = Chain[Loop + 1]->GetDirectionUV();//.getDirectionUV().negated();
			OuterBoneOuterToInnerUV = UFabrikUtil::VectorNegated(OuterBoneOuterToInnerUV);// OuterBoneOuterToInnerUV.negated();

			// Get the outer-to-inner unit vector of this bone
			FVector ThisBoneOuterToInnerUV = ThisBone->GetDirectionUV();// .getDirectionUV().negated();
			ThisBoneOuterToInnerUV = UFabrikUtil::VectorNegated(ThisBoneOuterToInnerUV);

			// Get the joint type for this bone and handle constraints on thisBoneInnerToOuterUV
			if (ThisBoneJointType == EJointType::JT_Ball)
			{
				// Constrain to relative angle between this bone and the outer bone if required
				float AngleBetweenDegs = UFabrikUtil::GetAngleBetweenDegs(OuterBoneOuterToInnerUV, ThisBoneOuterToInnerUV); //Vec3f.getAngleBetweenDegs(OuterBoneOuterToInnerUV, ThisBoneOuterToInnerUV);

				// getBallJointConstraintDegs => mRotorConstraintDegs
				float ConstraintAngleDegs = ThisBoneJoint->RotorConstraintDegs;//.getBallJointConstraintDegs();

				if (AngleBetweenDegs > ConstraintAngleDegs)
				{
					ThisBoneOuterToInnerUV = UFabrikUtil::GetAngleLimitedUnitVectorDegs(ThisBoneOuterToInnerUV, OuterBoneOuterToInnerUV, ConstraintAngleDegs); //Vec3f.getAngleLimitedUnitVectorDegs(ThisBoneOuterToInnerUV, OuterBoneOuterToInnerUV, ConstraintAngleDegs);
				}
			}
			else if (ThisBoneJointType == EJointType::JT_GlobalHinge)
			{
				// Project this bone outer-to-inner direction onto the hinge rotation axis
				// Note: The returned vector is normalised.

				// getHingeRotationAxis => mRotationAxisUV
				ThisBoneOuterToInnerUV = UFabrikUtil::ProjectOntoPlane(ThisBoneOuterToInnerUV, ThisBoneJoint->RotationAxisUV); // ThisBoneOuterToInnerUV.projectOntoPlane(ThisBoneJoint.getHingeRotationAxis());
				// NOTE: Constraining about the hinge reference axis on this forward pass leads to poor solutions... so we won't.
			}
			else if (ThisBoneJointType == EJointType::JT_LocalHinge)
			{
				// Not a basebone? Then construct a rotation matrix based on the previous bones inner-to-to-inner direction...
				UFabrikMat3f* M; //  UFabrikMat3f::CreateRotationMatrix(FVector InReferenceDirection) Mat3f M;
				FVector RelativeHingeRotationAxis;
				if (Loop > 0) {
					M = UFabrikMat3f::CreateRotationMatrix(Chain[Loop - 1]->GetDirectionUV());// Mat3f.createRotationMatrix(Chain[Loop - 1]->GetDirectionUV());

					// getHingeRotationAxis => 
					RelativeHingeRotationAxis = M->Times(ThisBoneJoint->RotationAxisUV);// .normalise(); // M.times(ThisBoneJoint->getHingeRotationAxis()).normalise();
					RelativeHingeRotationAxis.Normalize();
				}
				else // ...basebone? Need to construct matrix from the relative constraint UV.
				{
					RelativeHingeRotationAxis = BaseboneRelativeConstraintUV;
				}

				// ...and transform the hinge rotation axis into the previous bones frame of reference.
				//Vec3f 

				// Project this bone's outer-to-inner direction onto the plane described by the relative hinge rotation axis
				// Note: The returned vector is normalised.					
				ThisBoneOuterToInnerUV = UFabrikUtil::ProjectOntoPlane(ThisBoneOuterToInnerUV, RelativeHingeRotationAxis);// ThisBoneOuterToInnerUV.projectOntoPlane(RelativeHingeRotationAxis);

				// NOTE: Constraining about the hinge reference axis on this forward pass leads to poor solutions... so we won't.										
			}

			// At this stage we have a outer-to-inner unit vector for this bone which is within our constraints,
			// so we can set the new inner joint location to be the end joint location of this bone plus the
			// outer-to-inner direction unit vector multiplied by the length of the bone.
			FVector NewStartLocation = ThisBone->EndLocation + (ThisBoneOuterToInnerUV * ThisBoneLength);// ().plus(thisBoneOuterToInnerUV.times(thisBoneLength));

			// Set the new start joint location for this bone
			ThisBone->StartLocation = NewStartLocation;// setStartLocation(newStartLocation);

			// If we are not working on the basebone, then we also set the end joint location of
			// the previous bone in the chain (i.e. the bone closer to the base) to be the new
			// start joint location of this bone.
			if (Loop > 0)
			{
				Chain[Loop - 1]->EndLocation = NewStartLocation; // .setEndLocation(newStartLocation);
			}
		}
		else // If we ARE working on the end effector bone...
		{
			// Snap the end effector's end location to the target
			ThisBone->EndLocation = InTarget;//.setEndLocation(target);

			// Get the UV between the target / end-location (which are now the same) and the start location of this bone
			FVector ThisBoneOuterToInnerUV = ThisBone->GetDirectionUV();// .getDirectionUV().negated();
			ThisBoneOuterToInnerUV = UFabrikUtil::VectorNegated(ThisBoneOuterToInnerUV);

			// If the end effector is global hinged then we have to snap to it, then keep that
			// resulting outer-to-inner UV in the plane of the hinge rotation axis
			switch (ThisBoneJointType)
			{
			case EJointType::JT_Ball: //   BALL:
				// Ball joints do not get constrained on this forward pass
				break;
			case EJointType::JT_GlobalHinge: // GLOBAL_HINGE:
				// Global hinges get constrained to the hinge rotation axis, but not the reference axis within the hinge plane
				// getHingeRotationAxis => mRotationAxisUV
				ThisBoneOuterToInnerUV = UFabrikUtil::ProjectOntoPlane(ThisBoneOuterToInnerUV, ThisBoneJoint->RotationAxisUV);// ThisBoneOuterToInnerUV.projectOntoPlane(ThisBoneJoint.getHingeRotationAxis());
				break;
			case EJointType::JT_LocalHinge: //  LOCAL_HINGE:
				// Local hinges get constrained to the hinge rotation axis, but not the reference axis within the hinge plane

				// Construct a rotation matrix based on the previous bones inner-to-to-inner direction...
				// Mat3f m = Mat3f.createRotationMatrix(Chain[Loop - 1]->GetDirectionUV());
				UFabrikMat3f* M =  UFabrikMat3f::CreateRotationMatrix(Chain[Loop - 1]->GetDirectionUV());

				// ...and transform the hinge rotation axis into the previous bones frame of reference.
				// getHingeRotationAxis => mRotationAxisUV
				FVector RelativeHingeRotationAxis = M->Times(ThisBoneJoint->RotationAxisUV);// .normalise(); //  m.times(ThisBoneJoint.getHingeRotationAxis()).normalise();
				RelativeHingeRotationAxis.Normalize();

				// Project this bone's outer-to-inner direction onto the plane described by the relative hinge rotation axis
				// Note: The returned vector is normalised.					
				ThisBoneOuterToInnerUV = UFabrikUtil::ProjectOntoPlane(ThisBoneOuterToInnerUV, RelativeHingeRotationAxis);// ThisBoneOuterToInnerUV.projectOntoPlane(RelativeHingeRotationAxis);
				break;
			}

			// Calculate the new start joint location as the end joint location plus the outer-to-inner direction UV
			// multiplied by the length of the bone.
			FVector NewStartLocation = InTarget + (ThisBoneOuterToInnerUV * ThisBoneLength); // .plus(thisBoneOuterToInnerUV.times(thisBoneLength));

			// Set the new start joint location for this bone to be new start location...
			ThisBone->StartLocation = NewStartLocation;//.setStartLocation(newStartLocation);

			// ...and set the end joint location of the bone further in to also be at the new start location (if there IS a bone
			// further in - this may be a single bone chain)
			if (Loop > 0)
			{
				Chain[Loop - 1]->EndLocation = NewStartLocation; 
			}
		}

	} // End of forward pass

	// ---------- Backward pass from base to end effector -----------
	for (int Loop = 0; Loop < NumBones; ++Loop)
	{
		UFabrikBone* ThisBone = Chain[Loop];
		float ThisBoneLength = ThisBone->Length; //  Size();// length();

		// If we are not working on the basebone
		if (Loop != 0)
		{
			// Get the inner-to-outer direction of this bone as well as the previous bone to use as a baseline
			FVector ThisBoneInnerToOuterUV = ThisBone->GetDirectionUV();
			FVector PrevBoneInnerToOuterUV = Chain[Loop - 1]->GetDirectionUV();

			// Dealing with a ball joint?
			UFabrikJoint* ThisBoneJoint = ThisBone->Joint;
			EJointType JointType = ThisBone->Joint->JointType;

			if (JointType == EJointType::JT_Ball)
			{
				float AngleBetweenDegs = UFabrikUtil::GetAngleBetweenDegs(PrevBoneInnerToOuterUV, ThisBoneInnerToOuterUV); // Vec3f.getAngleBetweenDegs(prevBoneInnerToOuterUV, thisBoneInnerToOuterUV);

				// getBallJointConstraintDegs => mRotorConstraintDegs
				float ConstraintAngleDegs = ThisBoneJoint->RotorConstraintDegs;

				// Keep this bone direction constrained within the rotor about the previous bone direction
				if (AngleBetweenDegs > ConstraintAngleDegs)
				{
					ThisBoneInnerToOuterUV = UFabrikUtil::GetAngleLimitedUnitVectorDegs(ThisBoneInnerToOuterUV, PrevBoneInnerToOuterUV, ConstraintAngleDegs);//Vec3f.getAngleLimitedUnitVectorDegs(thisBoneInnerToOuterUV, prevBoneInnerToOuterUV, constraintAngleDegs);
				}
			}
			else if (JointType == EJointType::JT_GlobalHinge) 
			{

				// Get the hinge rotation axis and project our inner-to-outer UV onto it

				// getHingeRotationAxis => mRotationAxisUV
				FVector HingeRotationAxis = ThisBoneJoint->RotationAxisUV;// .getHingeRotationAxis();
				ThisBoneInnerToOuterUV = UFabrikUtil::ProjectOntoPlane(ThisBoneInnerToOuterUV, HingeRotationAxis); // ThisBoneInnerToOuterUV.projectOntoPlane(HingeRotationAxis);

				// If there are joint constraints, then we must honour them...
				float CwConstraintDegs = -ThisBoneJoint->HingeClockwiseConstraintDegs;//.getHingeClockwiseConstraintDegs();
				float AcwConstraintDegs = ThisBoneJoint->HingeAnticlockwiseConstraintDegs;// getHingeAnticlockwiseConstraintDegs();
				if (!(UFabrikUtil::ApproximatelyEquals(CwConstraintDegs, -UFabrikJoint::MAX_CONSTRAINT_ANGLE_DEGS, 0.001f)) &&
					!(UFabrikUtil::ApproximatelyEquals(AcwConstraintDegs, UFabrikJoint::MAX_CONSTRAINT_ANGLE_DEGS, 0.001f)))
				{
					// getHingeReferenceAxis => mReferenceAxisUV
					FVector HingeReferenceAxis = ThisBoneJoint->ReferenceAxisUV;// .getHingeReferenceAxis();

					// Get the signed angle (about the hinge rotation axis) between the hinge reference axis and the hinge-rotation aligned bone UV
					// Note: ACW rotation is positive, CW rotation is negative.
					float SignedAngleDegs = UFabrikUtil::GetSignedAngleBetweenDegs(HingeReferenceAxis, ThisBoneInnerToOuterUV, HingeRotationAxis); // Vec3f.getSignedAngleBetweenDegs(HingeReferenceAxis, ThisBoneInnerToOuterUV, HingeRotationAxis);

					// Make our bone inner-to-outer UV the hinge reference axis rotated by its maximum clockwise or anticlockwise rotation as required
					if (SignedAngleDegs > AcwConstraintDegs)
					{
						ThisBoneInnerToOuterUV = UFabrikUtil::RotateAboutAxisDegs(HingeReferenceAxis, AcwConstraintDegs, HingeRotationAxis);// Vec3f.rotateAboutAxisDegs(HingeReferenceAxis, AcwConstraintDegs, HingeRotationAxis).normalised();
						ThisBoneInnerToOuterUV.Normalize();
					}
					else if (SignedAngleDegs < CwConstraintDegs)
					{
						ThisBoneInnerToOuterUV = UFabrikUtil::RotateAboutAxisDegs(HingeReferenceAxis, CwConstraintDegs, HingeRotationAxis); // Vec3f.rotateAboutAxisDegs(HingeReferenceAxis, CwConstraintDegs, HingeRotationAxis).normalised();
						ThisBoneInnerToOuterUV.Normalize();
					}
				}
			}
			else if (JointType == EJointType::JT_LocalHinge)
			{
				// Transform the hinge rotation axis to be relative to the previous bone in the chain
				// getHingeRotationAxis => mRotationAxisUV
				FVector HingeRotationAxis = ThisBoneJoint->RotationAxisUV;// .getHingeRotationAxis();

				// Construct a rotation matrix based on the previous bone's direction
				//Mat3f m = Mat3f.createRotationMatrix(PrevBoneInnerToOuterUV);
				UFabrikMat3f* M = UFabrikMat3f::CreateRotationMatrix(PrevBoneInnerToOuterUV);

				// Transform the hinge rotation axis into the previous bone's frame of reference
				FVector RelativeHingeRotationAxis = M->Times(HingeRotationAxis);// .normalise();
				RelativeHingeRotationAxis.Normalize();

				// Project this bone direction onto the plane described by the hinge rotation axis
				// Note: The returned vector is normalised.
				ThisBoneInnerToOuterUV = UFabrikUtil::ProjectOntoPlane(ThisBoneInnerToOuterUV, RelativeHingeRotationAxis);// ThisBoneInnerToOuterUV.projectOntoPlane(RelativeHingeRotationAxis);

				// Constrain rotation about reference axis if required
				float CwConstraintDegs = -ThisBoneJoint->HingeClockwiseConstraintDegs;//.getHingeClockwiseConstraintDegs();
				float AcwConstraintDegs = ThisBoneJoint->HingeAnticlockwiseConstraintDegs; //  .getHingeAnticlockwiseConstraintDegs();

				if (!(UFabrikUtil::ApproximatelyEquals(CwConstraintDegs, -UFabrikJoint::MAX_CONSTRAINT_ANGLE_DEGS, 0.001f)) &&
					!(UFabrikUtil::ApproximatelyEquals(AcwConstraintDegs, UFabrikJoint::MAX_CONSTRAINT_ANGLE_DEGS, 0.001f)))
				{
					// Calc. the reference axis in local space
					//Vec3f relativeHingeReferenceAxis = mBaseboneRelativeReferenceConstraintUV;//m.times( thisBoneJoint.getHingeReferenceAxis() ).normalise();
					// getHingeReferenceAxis => mReferenceAxisUV
					FVector RelativeHingeReferenceAxis = M->Times(ThisBoneJoint->ReferenceAxisUV);// .normalise();
					RelativeHingeReferenceAxis.Normalize();

					// Get the signed angle (about the hinge rotation axis) between the hinge reference axis and the hinge-rotation aligned bone UV
					// Note: ACW rotation is positive, CW rotation is negative.
					float SignedAngleDegs = UFabrikUtil::GetSignedAngleBetweenDegs(RelativeHingeReferenceAxis, ThisBoneInnerToOuterUV, RelativeHingeRotationAxis); // Vec3f.getSignedAngleBetweenDegs(RelativeHingeReferenceAxis, ThisBoneInnerToOuterUV, RelativeHingeRotationAxis);

					// Make our bone inner-to-outer UV the hinge reference axis rotated by its maximum clockwise or anticlockwise rotation as required
					if (SignedAngleDegs > AcwConstraintDegs)
					{
						FVector RelativeHingeRotationAxisN = RelativeHingeRotationAxis;
						ThisBoneInnerToOuterUV = UFabrikUtil::RotateAboutAxisDegs(RelativeHingeReferenceAxis, AcwConstraintDegs, RelativeHingeRotationAxis);  // Vec3f.rotateAboutAxisDegs(RelativeHingeReferenceAxis, AcwConstraintDegs, RelativeHingeRotationAxis);// .normalise();
						ThisBoneInnerToOuterUV.Normalize();
					}
					else if (SignedAngleDegs < CwConstraintDegs)
					{
						ThisBoneInnerToOuterUV = UFabrikUtil::RotateAboutAxisDegs(RelativeHingeReferenceAxis, CwConstraintDegs, RelativeHingeRotationAxis);  // Vec3f.rotateAboutAxisDegs(RelativeHingeReferenceAxis, CwConstraintDegs, RelativeHingeRotationAxis);// .normalise();
						ThisBoneInnerToOuterUV.Normalize();
					}
				}

			} // End of local hinge section

			  // At this stage we have a outer-to-inner unit vector for this bone which is within our constraints,
			  // so we can set the new inner joint location to be the end joint location of this bone plus the
			  // outer-to-inner direction unit vector multiplied by the length of the bone.
			FVector NewEndLocation = ThisBone->StartLocation + (ThisBoneInnerToOuterUV * ThisBoneLength); // ().plus(thisBoneInnerToOuterUV.times(thisBoneLength));

			// Set the new start joint location for this bone
			ThisBone->EndLocation = NewEndLocation;

			// If we are not working on the end effector bone, then we set the start joint location of the next bone in
			// the chain (i.e. the bone closer to the target) to be the new end joint location of this bone.
			if (Loop < NumBones - 1) 
			{
				Chain[Loop + 1]->StartLocation = NewEndLocation; // setStartLocation(newEndLocation);
			}
		}
		else // If we ARE working on the basebone...
		{
			// If the base location is fixed then snap the start location of the basebone back to the fixed base...
			if (FixedBaseMode)
			{
				ThisBone->StartLocation = FixedBaseLocation;
			}
			else // ...otherwise project it backwards from the end to the start by its length.
			{
				ThisBone->StartLocation = ThisBone->EndLocation - (ThisBone->GetDirectionUV() * ThisBoneLength); //;;.minus(thisBone.getDirectionUV().times(thisBoneLength)));
			}

			// If the basebone is unconstrained then process it as usual...
			if (BaseboneConstraintType == EBoneConstraintType::BCT_None)
			{
				// Set the new end location of this bone, and if there are more bones,
				// then set the start location of the next bone to be the end location of this bone
				FVector NewEndLocation = ThisBone->StartLocation + (ThisBone->GetDirectionUV() * ThisBoneLength);// .getStartLocation().plus(thisBone.getDirectionUV().times(thisBoneLength));
				ThisBone->EndLocation = NewEndLocation;
				
				if (NumBones > 1)
				{
					Chain[1]->StartLocation = NewEndLocation;
				}
			}
			else // ...otherwise we must constrain it to the basebone constraint unit vector
			{
				if (BaseboneConstraintType == EBoneConstraintType::BCT_GlobalRotor)
				{
					// Get the inner-to-outer direction of this bone
					FVector ThisBoneInnerToOuterUV = ThisBone->GetDirectionUV();

					float AngleBetweenDegs = UFabrikUtil::GetAngleBetweenDegs(BaseboneConstraintUV, ThisBoneInnerToOuterUV);;// Vec3f.getAngleBetweenDegs(BaseboneConstraintUV, ThisBoneInnerToOuterUV);

					// getBallJointConstraintDegs => mRotorConstraintDegs
					float ConstraintAngleDegs = ThisBone->Joint->RotorConstraintDegs;// .getBallJointConstraintDegs();

					if (AngleBetweenDegs > ConstraintAngleDegs)
					{
						ThisBoneInnerToOuterUV = UFabrikUtil::GetAngleLimitedUnitVectorDegs(ThisBoneInnerToOuterUV, BaseboneConstraintUV, ConstraintAngleDegs);//Vec3f.getAngleLimitedUnitVectorDegs(ThisBoneInnerToOuterUV, BaseboneConstraintUV, ConstraintAngleDegs);
					}

					FVector NewEndLocation = ThisBone->StartLocation + (ThisBoneInnerToOuterUV * ThisBoneLength); // .plus(thisBoneInnerToOuterUV.times(thisBoneLength));
					ThisBone->EndLocation = NewEndLocation;

					// Also, set the start location of the next bone to be the end location of this bone

					if (NumBones > 1) 
					{
						Chain[1]->StartLocation = NewEndLocation; 
					}
				}
				else if (BaseboneConstraintType == EBoneConstraintType::BCT_LocalRotor)
				{
					// Note: The mBaseboneRelativeConstraintUV is updated in the FabrikStructure3D.solveForTarget()
					// method BEFORE this FabrikChain3D.solveForTarget() method is called. We no knowledge of the
					// direction of the bone we're connected to in another chain and so cannot calculate this 
					// relative basebone constraint direction on our own, but the FabrikStructure3D does it for
					// us so we are now free to use it here.

					// Get the inner-to-outer direction of this bone
					FVector ThisBoneInnerToOuterUV = ThisBone->GetDirectionUV();

					// Constrain about the relative basebone constraint unit vector as neccessary
					float AngleBetweenDegs = UFabrikUtil::GetAngleBetweenDegs(BaseboneRelativeConstraintUV, ThisBoneInnerToOuterUV);// Vec3f.getAngleBetweenDegs(BaseboneRelativeConstraintUV, ThisBoneInnerToOuterUV);

					// getBallJointConstraintDegs => mRotorConstraintDegs
					float ConstraintAngleDegs = ThisBone->Joint->RotorConstraintDegs;// getBallJointConstraintDegs();

					if (AngleBetweenDegs > ConstraintAngleDegs)
					{
						ThisBoneInnerToOuterUV = UFabrikUtil::GetAngleLimitedUnitVectorDegs(ThisBoneInnerToOuterUV, BaseboneRelativeConstraintUV, ConstraintAngleDegs);// Vec3f.getAngleLimitedUnitVectorDegs(ThisBoneInnerToOuterUV, BaseboneRelativeConstraintUV, ConstraintAngleDegs);
					}

					// Set the end location
					FVector NewEndLocation = ThisBone->StartLocation + (ThisBoneInnerToOuterUV * ThisBoneLength); // .plus(thisBoneInnerToOuterUV.times(thisBoneLength));
					ThisBone->EndLocation = NewEndLocation;

					// Also, set the start location of the next bone to be the end location of this bone
					if (NumBones > 1) 
					{ 
						Chain[1]->StartLocation = NewEndLocation; 
					}

				}
				else if (BaseboneConstraintType == EBoneConstraintType::BCT_GlobalHinge)
				{
					UFabrikJoint* ThisJoint = ThisBone->Joint;

					// getHingeRotationAxis => mRotationAxisUV
					FVector HingeRotationAxis = ThisJoint->RotationAxisUV;// getHingeRotationAxis();
					float CwConstraintDegs = -ThisJoint->HingeClockwiseConstraintDegs;// GetHingeClockwiseConstraintDegs();     // Clockwise rotation is negative!
					float AcwConstraintDegs = ThisJoint->HingeAnticlockwiseConstraintDegs;// GetHingeAnticlockwiseConstraintDegs();

					// Get the inner-to-outer direction of this bone and project it onto the global hinge rotation axis
					FVector ThisBoneInnerToOuterUV = UFabrikUtil::ProjectOntoPlane(ThisBone->GetDirectionUV(), HingeRotationAxis);// ThisBone->GetDirectionUV().projectOntoPlane(HingeRotationAxis);

					// If we have a global hinge which is not freely rotating then we must constrain about the reference axis
					if (!(UFabrikUtil::ApproximatelyEquals(CwConstraintDegs, UFabrikJoint::MAX_CONSTRAINT_ANGLE_DEGS, 0.01f) &&
						UFabrikUtil::ApproximatelyEquals(AcwConstraintDegs, UFabrikJoint::MAX_CONSTRAINT_ANGLE_DEGS, 0.01f)))
					{
						// Grab the hinge reference axis and calculate the current signed angle between it and our bone direction (about the hinge
						// rotation axis). Note: ACW rotation is positive, CW rotation is negative.

						// getHingeReferenceAxis => mReferenceAxisUV
						FVector HingeReferenceAxis = ThisJoint->ReferenceAxisUV;// .getHingeReferenceAxis();

						float SignedAngleDegs = UFabrikUtil::GetSignedAngleBetweenDegs(HingeReferenceAxis, ThisBoneInnerToOuterUV, HingeRotationAxis); // Vec3f.getSignedAngleBetweenDegs(HingeReferenceAxis, ThisBoneInnerToOuterUV, HingeRotationAxis);

						// Constrain as necessary
						if (SignedAngleDegs > AcwConstraintDegs)
						{
							ThisBoneInnerToOuterUV = UFabrikUtil::RotateAboutAxisDegs(HingeReferenceAxis, AcwConstraintDegs, HingeRotationAxis);// Vec3f.rotateAboutAxisDegs(HingeReferenceAxis, AcwConstraintDegs, HingeRotationAxis);// .normalise();
							ThisBoneInnerToOuterUV.Normalize();
						}
						else if (SignedAngleDegs < CwConstraintDegs)
						{
							ThisBoneInnerToOuterUV = UFabrikUtil::RotateAboutAxisDegs(HingeReferenceAxis, CwConstraintDegs, HingeRotationAxis); // Vec3f.rotateAboutAxisDegs(HingeReferenceAxis, CwConstraintDegs, HingeRotationAxis);// .normalise();
							ThisBoneInnerToOuterUV.Normalize();
						}
					}

					// Calc and set the end location of this bone
					FVector NewEndLocation = ThisBone->StartLocation + (ThisBoneInnerToOuterUV * ThisBoneLength); // .plus(thisBoneInnerToOuterUV.times(thisBoneLength));
					ThisBone->EndLocation = NewEndLocation;

					// Also, set the start location of the next bone to be the end location of this bone
					if (NumBones > 1) 
					{ 
						Chain[1]->StartLocation = NewEndLocation;
					}

				}
				else if (BaseboneConstraintType == EBoneConstraintType::BCT_LocalHinge)
				{
					UFabrikJoint* ThisJoint = ThisBone->Joint;
					FVector HingeRotationAxis = BaseboneRelativeConstraintUV;                   // Basebone relative constraint is our hinge rotation axis!
					float CwConstraintDegs = -ThisJoint->HingeClockwiseConstraintDegs;     // Clockwise rotation is negative!
					float AcwConstraintDegs = ThisJoint->HingeAnticlockwiseConstraintDegs;

					// Get the inner-to-outer direction of this bone and project it onto the global hinge rotation axis
					FVector ThisBoneInnerToOuterUV = UFabrikUtil::ProjectOntoPlane(ThisBone->GetDirectionUV(), HingeRotationAxis);// ThisBone->GetDirectionUV().projectOntoPlane(HingeRotationAxis);

					//If we have a local hinge which is not freely rotating then we must constrain about the reference axis
					if (!(UFabrikUtil::ApproximatelyEquals(CwConstraintDegs, UFabrikJoint::MAX_CONSTRAINT_ANGLE_DEGS, 0.01f) &&
						UFabrikUtil::ApproximatelyEquals(AcwConstraintDegs, UFabrikJoint::MAX_CONSTRAINT_ANGLE_DEGS, 0.01f)))
					{
						// Grab the hinge reference axis and calculate the current signed angle between it and our bone direction (about the hinge
						// rotation axis). Note: ACW rotation is positive, CW rotation is negative.
						FVector HingeReferenceAxis = BaseboneRelativeReferenceConstraintUV; //thisJoint.getHingeReferenceAxis();
						float SignedAngleDegs = UFabrikUtil::GetSignedAngleBetweenDegs(HingeReferenceAxis, ThisBoneInnerToOuterUV, HingeRotationAxis);// Vec3f.getSignedAngleBetweenDegs(HingeReferenceAxis, ThisBoneInnerToOuterUV, HingeRotationAxis);

						// Constrain as necessary
						if (SignedAngleDegs > AcwConstraintDegs)
						{
							ThisBoneInnerToOuterUV = UFabrikUtil::RotateAboutAxisDegs(HingeReferenceAxis, AcwConstraintDegs, HingeRotationAxis); // Vec3f.rotateAboutAxisDegs(HingeReferenceAxis, AcwConstraintDegs, HingeRotationAxis).normalise();
							ThisBoneInnerToOuterUV.Normalize();
						}
						else if (SignedAngleDegs < CwConstraintDegs)
						{
							ThisBoneInnerToOuterUV = UFabrikUtil::RotateAboutAxisDegs(HingeReferenceAxis, CwConstraintDegs, HingeRotationAxis); // Vec3f.rotateAboutAxisDegs(HingeReferenceAxis, CwConstraintDegs, HingeRotationAxis).normalise();
							ThisBoneInnerToOuterUV.Normalize();
						}
					}

					// Calc and set the end location of this bone
					FVector NewEndLocation = ThisBone->StartLocation + (ThisBoneInnerToOuterUV * ThisBoneLength); //  .plus(thisBoneInnerToOuterUV.times(thisBoneLength));
					ThisBone->EndLocation = NewEndLocation;

					// Also, set the start location of the next bone to be the end location of this bone
					if (NumBones > 1)
					{
						Chain[1]->StartLocation = NewEndLocation;
					}

				}

			} // End of basebone constraint handling section

		} // End of basebone handling section

	} // End of backward-pass loop over all bones

	  // Update our last target location
	LastTargetLocation = InTarget;

	// DEBUG - check the live chain length and the originally calculated chain length are the same
	/*
	if (Math.abs( this.getLiveChainLength() - mChainLength) > 0.01f)
	{
	System.out.println("Chain length off by > 0.01f");
	}
	*/

	// Finally, calculate and return the distance between the current effector location and the target.
	return FVector::Dist(Chain[NumBones - 1]->EndLocation, InTarget);
}


void UFabrikChain::UpdateChainLength()
{
	// We start adding up the length of the bones from an initial length of zero
	ChainLength = 0.0f;

	// Loop over all the bones in the chain, adding the length of each bone to the mChainLength property
	for (int Loop = 0; Loop < NumBones; ++Loop)
	{
		ChainLength += Chain[Loop]->Length;
	}
}

TArray<UFabrikBone*> UFabrikChain::CloneIkChain()
{
	// How many bones are in this chain?
	int NumBonesTmp = Chain.Num();

	// Create a new Vector of FabrikBone3D objects of that size
	TArray<UFabrikBone*> ClonedChain;// = new ArrayList<FabrikBone3D>(numBones);

	// For each bone in the chain being cloned...	
	for (int Loop = 0; Loop < NumBonesTmp; ++Loop)
	{
		// Use the copy constructor to create a new FabrikBone3D with the values set from the source FabrikBone3D.

		// and add it to the cloned chain.
		UFabrikBone* Clone = NewObject<UFabrikBone>();
		Clone->Init(Chain[Loop]);
		ClonedChain.Add(Clone);// new FabrikBone3D(mChain.get(loop)));
	}

	return ClonedChain;
}

void UFabrikChain::ConnectToStructure(UFabrikStructure* InStructure, int InChainNumber, int InBoneNumber)
{
	// Sanity check chain exists
	int NumChains = InStructure->NumChains;
	if (InChainNumber > NumChains)
	{ 
		UE_LOG(OpenMotionLog, Fatal, TEXT("Structure does not contain a chain TODO - it has TODO chains."));// throw new IllegalArgumentException("Structure does not contain a chain " + chainNumber + " - it has " + numChains + " chains.");
	}

	// Sanity check bone exists
	int NumBonesL = InStructure->Chains[InChainNumber]->NumBones;
	if (InBoneNumber > NumBonesL)
	{ 
		UE_LOG(OpenMotionLog, Fatal, TEXT("Chain does not contain a bone TODO - it has TODO bones.")); //throw new IllegalArgumentException("Chain does not contain a bone " + boneNumber + " - it has " + numBones + " bones.");
	}

	// All good? Set the connection details
	ConnectedChainNumber = InChainNumber;
	ConnectedBoneNumber = InBoneNumber;
}