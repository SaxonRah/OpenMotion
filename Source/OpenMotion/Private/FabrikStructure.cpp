// Fill out your copyright notice in the Description page of Project Settings.

#include "FabrikStructure.h"
#include "FabrikChain.h"
#include "FabrikBone.h"
#include "FabrikMat3f.h"
#include "EBoneConstraintType.h"

#include "OpenMotion.h"

UFabrikStructure::UFabrikStructure(const FObjectInitializer& ObjectInitializer)
{
	NumChains = 0;
}

 void UFabrikStructure::SolveForTarget(FVector InNewTargetLocation)
{
	int NumChainsL = Chains.Num();
	int ConnectedChainNumber;

	// Loop over all chains in this structure...
	for (int Loop = 0; Loop < NumChainsL; ++Loop)
	{
		// Get this chain, and get the number of the chain in this structure it's connected to (if any)
		UFabrikChain* ThisChain = Chains[Loop];

		ConnectedChainNumber = ThisChain->ConnectedChainNumber;// getConnectedChainNumber();

		// If this chain isn't connected to another chain then update as normal...
		if (ConnectedChainNumber == -1)
		{
			ThisChain->SolveForTarget(InNewTargetLocation);
		}
		else // ...however, if this chain IS connected to another chain...
		{
			// ... get the host chain and bone which this chain is connected to
			UFabrikChain* HostChain = Chains[ConnectedChainNumber];
			UFabrikBone* HostBone = HostChain->GetBone(ThisChain->ConnectedBoneNumber); //.getConnectedBoneNumber());

			if (HostBone->BoneConnectionPoint == EBoneConnectionPoint::BCP_Start)
			{ 
				// setBaseLocation => mFixedBaseLocation 
				ThisChain->FixedBaseLocation = HostBone->StartLocation; // setBaseLocation(hostBone.getStartLocation());
			}
			else 
			{ 
				// setBaseLocation => mFixedBaseLocation 
				ThisChain->FixedBaseLocation = HostBone->EndLocation;  // setBaseLocation(hostBone.getEndLocation()); } 
			}

			// Now that we've clamped the base location of this chain to the start or end point of the bone in the chain we are connected to, it's
			// time to deal with any base bone constraints...

			// What type of base bone constraint is this (connected to another) chain using? 
			EBoneConstraintType ConstraintType = ThisChain->BaseboneConstraintType;

			switch (ConstraintType)
			{
				// None or global basebone constraints? Nothing to do, because these will be handled in FabrikChain3D.solveIK() as we do not
				// need information from another chain to handle them.
			case EBoneConstraintType::BCT_None: // NONE:         // Nothing to do because there's no basebone constraint
			case EBoneConstraintType::BCT_GlobalRotor:// GLOBAL_ROTOR: // Nothing to do because the basebone constraint is not relative to bones in other chains in this structure
			case EBoneConstraintType::BCT_GlobalHinge: //  GLOBAL_HINGE: // Nothing to do because the basebone constraint is not relative to bones in other chains in this structure
				break;

				// If we have a local rotor or hinge constraint then we must calculate the relative basebone constraint before calling updateTarget
			case EBoneConstraintType::BCT_LocalRotor: // LOCAL_ROTOR:
			case EBoneConstraintType::BCT_LocalHinge: { // LOCAL_HINGE: {
				// Get the direction of the bone this chain is connected to and create a rotation matrix from it.
				//Mat3f ConnectionBoneMatrix = Mat3f.createRotationMatrix(HostBone.getDirectionUV());
				UFabrikMat3f* ConnectionBoneMatrix = UFabrikMat3f::CreateRotationMatrix(HostBone->GetDirectionUV());
				// We'll then get the basebone constraint UV and multiply it by the rotation matrix of the connected bone 
				// to make the basebone constraint UV relative to the direction of bone it's connected to.
				FVector RelativeBaseboneConstraintUV = ConnectionBoneMatrix->Times(ThisChain->BaseboneConstraintUV);// .normalised();
				RelativeBaseboneConstraintUV.Normalize();

				// Update our basebone relative constraint UV property
				ThisChain->BaseboneRelativeConstraintUV = RelativeBaseboneConstraintUV;

				// Update the relative reference constraint UV if we hav a local hinge
				if (ConstraintType == EBoneConstraintType::BCT_LocalHinge)
				{
					//getHingeReferenceAxis() => mReferenceAxisUV
					ThisChain->BaseboneRelativeReferenceConstraintUV = ConnectionBoneMatrix->Times(ThisChain->GetBone(0)->Joint->ReferenceAxisUV);// .getHingeReferenceAxis()));
				}
				break;
			}
							  // No need for a default - constraint types are enums and we've covered them all.

			}

			// NOTE: If the base bone constraint type is NONE then we don't do anything with the base bone constraint of the connected chain.

			// Finally, update the target and solve the chain
			// Update the target and solve the chain
			if (!ThisChain->UseEmbeddedTarget) // GetEmbeddedTargetMode)
			{
				ThisChain->SolveForTarget(InNewTargetLocation);
			}
			else
			{
				ThisChain->SolveForEmbeddedTarget();
			}

		} // End of if chain is connected to another chain section

	} // End of loop over chains

} // End of updateTarget method


void UFabrikStructure::SolveForTarget(float InTargetX, float InTargetY, float InTargetZ)
 {
	 // Call our Vec3f version of updateTarget using a constructed Vec3f target location
	 // Note: This will loop over all chains, attempting to solve each for the same target location
	 SolveForTarget(FVector(InTargetX, InTargetY, InTargetZ));
 }


void UFabrikStructure::AddChain(UFabrikChain* InChain)
{
	Chains.Add(InChain);
	++NumChains;
}

void UFabrikStructure::RemoveChain(int InChainIndex)
{

	Chains.RemoveAt(InChainIndex);
	--NumChains;
}

void UFabrikStructure::ConnectChain(UFabrikChain* InNewChain, int InExistingChainNumber, int InExistingBoneNumber)
{
	// Does this chain exist? If not throw an IllegalArgumentException
	if (InExistingChainNumber > NumChains)
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("Cannot connect to chain TODO - no such chain (remember that chains are zero indexed).")); // throw new IllegalArgumentException("Cannot connect to chain " + existingChainNumber + " - no such chain (remember that chains are zero indexed).");
	}

	// Do we have this bone in the specified chain? If not throw an IllegalArgumentException
	if (InExistingBoneNumber > Chains[InExistingChainNumber]->NumBones)
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("Cannot connect to bone TODO of chain TODO - no such bone (remember that bones are zero indexed).")); //throw new IllegalArgumentException("Cannot connect to bone " + existingBoneNumber + " of chain " + existingChainNumber + " - no such bone (remember that bones are zero indexed).");
	}

	// Make a copy of the provided chain so any changes made to the original do not affect this chain
	UFabrikChain* RelativeChain = NewObject<UFabrikChain>();// new FabrikChain3D(newChain);
	RelativeChain->Init(InNewChain);

	// Connect the copy of the provided chain to the specified chain and bone in this structure
	RelativeChain->ConnectToStructure(this, InExistingChainNumber, InExistingBoneNumber);

	// The chain as we were provided should be centred on the origin, so we must now make it
	// relative to the start location of the given bone in the given chain.

	// Get the connection point so we know to connect at the start or end location of the bone we're connecting to
	EBoneConnectionPoint ConnectionPoint = Chains[InExistingChainNumber]->GetBone(InExistingBoneNumber)->BoneConnectionPoint;// this->GetChain(existingChainNumber).getBone(existingBoneNumber).getBoneConnectionPoint();
	FVector ConnectionLocation;
	if (ConnectionPoint == EBoneConnectionPoint::BCP_Start)
	{
		ConnectionLocation = Chains[InExistingChainNumber]->GetBone(InExistingBoneNumber)->StartLocation;
	}
	else // If it's BoneConnectionPoint.END then we set the connection point to be the end location of the bone we're connecting to
	{
		ConnectionLocation = Chains[InExistingChainNumber]->GetBone(InExistingBoneNumber)->EndLocation;
	}

	// setBaseLocation => mFixedBaseLocation 
	RelativeChain->FixedBaseLocation = ConnectionLocation;// .setBaseLocation(ConnectionLocation);

	// When we have a chain connected to a another 'host' chain, the chain is which is connecting in
	// MUST have a fixed base, even though that means the base location is 'fixed' to the connection
	// point on the host chain, rather than a static location.
	RelativeChain->FixedBaseMode = true; //.setFixedBaseMode(true);

	// Translate the chain we're connecting to the connection point
	for (int Loop = 0; Loop < RelativeChain->NumBones; ++Loop)
	{
		FVector OrigStart = RelativeChain->GetBone(Loop)->StartLocation;
		FVector OrigEnd = RelativeChain->GetBone(Loop)->EndLocation;

		FVector TranslatedStart = OrigStart + ConnectionLocation;
		FVector TranslatedEnd = OrigEnd + ConnectionLocation;

		RelativeChain->GetBone(Loop)->StartLocation = TranslatedStart;// .setStartLocation(translatedStart);
		RelativeChain->GetBone(Loop)->EndLocation = TranslatedEnd; // setEndLocation(TranslatedEnd);
	}

	this->AddChain(RelativeChain);
}

void UFabrikStructure::ConnectChain(UFabrikChain* InNewChain, int InExistingChainNumber, int InExistingBoneNumber, EBoneConnectionPoint InBoneConnectionPoint)
{
	// Does this chain exist? If not throw an IllegalArgumentException
	if (InExistingChainNumber > NumChains)
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("Cannot connect to chain TODO - no such chain (remember that chains are zero indexed).")); //throw new IllegalArgumentException("Cannot connect to chain " + existingChainNumber + " - no such chain (remember that chains are zero indexed).");
	}

	// Do we have this bone in the specified chain? If not throw an IllegalArgumentException
	if (InExistingBoneNumber > Chains[InExistingChainNumber]->NumBones)
	{
		UE_LOG(OpenMotionLog, Fatal, TEXT("Cannot connect to bone TODO of chain TODO - no such bone (remember that bones are zero indexed).")); //throw new IllegalArgumentException("Cannot connect to bone " + existingBoneNumber + " of chain " + existingChainNumber + " - no such bone (remember that bones are zero indexed).");
	}

	// Make a copy of the provided chain so any changes made to the original do not affect this chain
	UFabrikChain* RelativeChain = NewObject<UFabrikChain>();// new FabrikChain3D(newChain);
	RelativeChain->Init(InNewChain);

	// Connect the copy of the provided chain to the specified chain and bone in this structure
	RelativeChain->ConnectToStructure(this, InExistingChainNumber, InExistingBoneNumber);

	// The chain as we were provided should be centred on the origin, so we must now make it
	// relative to the start location of the given bone in the given chain.

	// Set the connection point and use it to get the connection location
	Chains[InExistingChainNumber]->GetBone(InExistingBoneNumber)->BoneConnectionPoint = InBoneConnectionPoint;  // this->GetChain(existingChainNumber).getBone(existingBoneNumber).setBoneConnectionPoint(boneConnectionPoint);
	FVector ConnectionLocation;
	if (InBoneConnectionPoint == EBoneConnectionPoint::BCP_Start)
	{
		ConnectionLocation = Chains[InExistingChainNumber]->GetBone(InExistingBoneNumber)->StartLocation;
	}
	else // If it's BoneConnectionPoint.END then we set the connection point to be the end location of the bone we're connecting to
	{
		ConnectionLocation = Chains[InExistingChainNumber]->GetBone(InExistingBoneNumber)->EndLocation;
	}

	// setBaseLocation() => mFixedBaseLocation 
	RelativeChain->FixedBaseLocation = ConnectionLocation;// .setBaseLocation(connectionLocation);

	// When we have a chain connected to a another 'host' chain, the chain is which is connecting in
	// MUST have a fixed base, even though that means the base location is 'fixed' to the connection
	// point on the host chain, rather than a static location.
	RelativeChain->FixedBaseMode = true;//.setFixedBaseMode(true);

	// Translate the chain we're connecting to the connection point
	for (int Loop = 0; Loop < RelativeChain->NumBones; ++Loop)
	{
		FVector OrigStart = RelativeChain->GetBone(Loop)->StartLocation;
		FVector OrigEnd = RelativeChain->GetBone(Loop)->EndLocation;

		FVector TranslatedStart = OrigStart + ConnectionLocation;
		FVector TranslatedEnd = OrigEnd + ConnectionLocation;

		RelativeChain->GetBone(Loop)->StartLocation = TranslatedStart;
		RelativeChain->GetBone(Loop)->EndLocation = TranslatedEnd;
	}

	this->AddChain(RelativeChain);
}

void UFabrikStructure::SetFixedBaseMode(bool InFixedBaseMode)
{
	for (int Loop = 0; Loop < NumChains; ++Loop)
	{
		Chains[Loop]->FixedBaseMode = InFixedBaseMode;
	}
}