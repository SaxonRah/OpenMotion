// Fill out your copyright notice in the Description page of Project Settings.
// https://github.com/FedUni/caliko/blob/master/caliko-demo/src/au/edu/federation/alansley/CalikoDemo3D.java

#include "FabrikDemoActor.h"

#include "FabrikStructure.h"
#include "FabrikChain.h"
#include "FabrikBone.h"
#include "FabrikUtil.h"
#include "FabrikDebugComponent.h"

#include "DrawDebugHelpers.h"

#include "OpenMotion.h"

// Sets default values
AFabrikDemoActor::AFabrikDemoActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	XAxis = FVector(1, 0, 0);
	YAxis = FVector(0, 1, 0);
	ZAxis = FVector(0, 0, 1);
	XAxis = FVector(1, 0, 0);

	DefaultBoneDirection = FVector(0, 0, -1);
	DefaultBoneLength = 10.0f;
	BoneLineWidth = 5.0f;
	ConstraintLineWidth = 2.0f;
	BaseRotationAmountDegs = 0.3f;

	PointSize = 5.0f;
	LineThickness = 2.0f;

	DemoType = EFabrikDemoType::FD_UnconstrainedBones;
}

// Called when the game starts or when spawned
void AFabrikDemoActor::BeginPlay()
{
	Super::BeginPlay();

	FabrikDebugComponent = FindComponentByClass<UFabrikDebugComponent>();
	
	switch (DemoType)
	{
	case EFabrikDemoType::FD_UnconstrainedBones: DemoUnconstrainedBones();break;
	case EFabrikDemoType::FD_RotorBallJointConstrainedBones: DemoRotorBallJointConstrainedBones(); break;
	case EFabrikDemoType::FD_RotorConstrainedBaseBones: DemoRotorConstrainedBaseBones(); break;
	case EFabrikDemoType::FD_FreelyRotatingGlobalHinges: DemoFreelyRotatingGlobalHinges(); break;
	case EFabrikDemoType::FD_GlobalHingesWithReferenceAxisConstraints: DemoGlobalHingesWithReferenceAxisConstraints(); break;
	case EFabrikDemoType::FD_FreelyRotatingLocalHinges: DemoFreelyRotatingLocalHinges(); break;
	case EFabrikDemoType::FD_LocalHingesWithReferenceAxisConstraints: DemoLocalHingesWithReferenceAxisConstraints(); break;
	case EFabrikDemoType::FD_ConnectedChains: DemoConnectedChains(); break;
	case EFabrikDemoType::FD_GlobalRotorConstrainedConnectedChains: DemoGlobalRotorConstrainedConnectedChains(); break;
	case EFabrikDemoType::FD_LocalRotorConstrainedConnectedChains: DemoLocalRotorConstrainedConnectedChains(); break;
	case EFabrikDemoType::FD_ConnectedChainsWithFreelyRotatingGlobalHingedBaseboneConstraints: DemoConnectedChainsWithFreelyRotatingGlobalHingedBaseboneConstraints(); break;
	case EFabrikDemoType::FD_ConnectedChainsWithEmbeddedTargets: DemoConnectedChainsWithEmbeddedTargets(); break;
	}

	if (FabrikDebugComponent != NULL)
	{
		FabrikDebugComponent->Structure = this->Structure;
	}

}

// Called every frame
void AFabrikDemoActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	Structure->SolveForTarget(TargetActor->GetActorLocation());

	/*for (UFabrikChain* Chain : Structure->Chains)
	{
		DrawChain(Chain);
	}*/
}

void AFabrikDemoActor::DrawChain(UFabrikChain* Chain)
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
				false,  				//persistent (never goes away)
				0.03 					//point leaves a trail on moving object
			);
		}
		DrawDebugPoint(
			GetWorld(),
			Bone->EndLocation,
			PointSize,  					//size
			FColor(255, 0, 255),  //pink
			false,  				//persistent (never goes away)
			0.03 					//point leaves a trail on moving object
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

// http://www.pvladov.com/2012/09/make-color-lighter-or-darker.html
FColor AFabrikDemoActor::Brightness(FColor color, float correctionFactor)
{
	float red = (float)color.R;
	float green = (float)color.G;
	float blue = (float)color.B;

	if (correctionFactor < 0)
	{
		correctionFactor = 1 + correctionFactor;
		red *= correctionFactor;
		green *= correctionFactor;
		blue *= correctionFactor;
	}
	else
	{
		red = (255 - red) * correctionFactor + red;
		green = (255 - green) * correctionFactor + green;
		blue = (255 - blue) * correctionFactor + blue;
	}

	return FColor((int)red, (int)green, (int)blue);// Color.FromArgb(color.A, (int)red, (int)green, (int)blue);
}


void AFabrikDemoActor::DemoUnconstrainedBones()
{
	// Demo 1
	//demoName = "Demo 1 - Unconstrained bones";
	Structure = NewObject<UFabrikStructure>(); // new FabrikStructure3D(demoName);
	FColor BoneColour = UFabrikUtil::GREEN;// FColor::Green;// new Colour4f(Utils.GREEN);

									  // Create a new chain...				
	UFabrikChain* Chain = NewObject<UFabrikChain>();  // new FabrikChain3D();

													  // ...then create a basebone, set its draw colour and add it to the chain.
	FVector StartLoc = FVector(0.0f, 0.0f, 40.0f);
	FVector EndLoc = StartLoc + (DefaultBoneDirection * DefaultBoneLength);

	UFabrikBone* Basebone = NewObject<UFabrikBone>(); // new FabrikBone3D(startLoc, endLoc);
	Basebone->Init(StartLoc, EndLoc);

	Basebone->Color = BoneColour;
	Chain->AddBone(Basebone);

	// Add additional consecutive, unconstrained bones to the chain				
	for (int BoneLoop = 0; BoneLoop < 7; BoneLoop++)
	{
		//BoneColour = FColor(FMath::RandRange(0, 255), FMath::RandRange(0, 255), FMath::RandRange(0, 255));// (BoneLoop % 2 == 0) ? BoneColour.lighten(0.4f) : BoneColour.darken(0.4f);
		BoneColour = (BoneLoop % 2 == 0) ? UFabrikUtil::Lighten(BoneColour, 0.4f) : UFabrikUtil::Darken(BoneColour, 0.4f);// Brightness(BoneColour, 0.4f) : Brightness(BoneColour, -0.4f);

		Chain->AddConsecutiveBone(DefaultBoneDirection, DefaultBoneLength, BoneColour);
	}

	//Chain->FixedBaseMode = false; // # not part of the original demo

	// Finally, add the chain to the structure
	Structure->AddChain(Chain);
}

void AFabrikDemoActor::DemoRotorBallJointConstrainedBones()
{
	//demoName = "Demo 2 - Rotor / Ball Joint Constrained Bones";
	Structure = NewObject<UFabrikStructure>(); // new FabrikStructure3D(demoName);
	int numChains = 3;
	float rotStep = 360.0f / (float)numChains;
	float constraintAngleDegs = 45.0f;
	FColor boneColour;// = new Colour4f();
	
	for (int chainLoop = 0; chainLoop < numChains; ++chainLoop)
	{
		// Create a new chain
		UFabrikChain* chain = NewObject<UFabrikChain>(); // new FabrikChain3D();

		// Choose the bone colour
		switch (chainLoop % numChains)
		{
		case 0:	boneColour=UFabrikUtil::MID_RED;   break;
		case 1:	boneColour=UFabrikUtil::MID_GREEN; break;
		case 2:	boneColour=UFabrikUtil::MID_BLUE;  break;
		}

		// Set up the initial base bone location...
		FVector startLoc = FVector(0.0f, 0.0f, -40.0f);
		startLoc = UFabrikUtil::RotateYDegs(startLoc, rotStep * (float)chainLoop);
		FVector endLoc = FVector(startLoc);
		endLoc.Z -= DefaultBoneLength;

		// ...then create a base bone, set its colour and add it to the chain.
		UFabrikBone* basebone = NewObject<UFabrikBone>(); //new FabrikBone3D(startLoc, endLoc);
		basebone->Init(startLoc, endLoc);
		basebone->Color = boneColour;
		chain->AddBone(basebone);
		//basebone->Joint->RotorConstraintDegs = constraintAngleDegs;
		// Add additional consecutive rotor (i.e. ball joint) constrained bones to the chain					
		for (int boneLoop = 0; boneLoop < 7; boneLoop++)
		{
			boneColour = (boneLoop % 2 == 0) ? UFabrikUtil::Lighten(boneColour, 0.4f) : UFabrikUtil::Darken(boneColour, 0.4f);// boneColour.lighten(0.4f) : boneColour.darken(0.4f);
			chain->AddConsecutiveRotorConstrainedBoneC(DefaultBoneDirection, DefaultBoneLength, constraintAngleDegs, boneColour);
			//chain->AddConsecutiveBone(DefaultBoneDirection, DefaultBoneLength, boneColour);
		}

		// chain->FixedBaseMode = false;

		// Finally, add the chain to the structure
		Structure->AddChain(chain);
	}
}

void AFabrikDemoActor::DemoRotorConstrainedBaseBones()
{
	//demoName = "Demo 3 - Rotor Constrained Base Bones";
	Structure = NewObject<UFabrikStructure>(); //  new FabrikStructure3D(demoName);
	int numChains = 3;
	float rotStep = 360.0f / (float)numChains;
	float baseBoneConstraintAngleDegs = 20.0f;

	// ... and add multiple chains to it.
	FColor boneColour;// = new Colour4f();
	FColor baseBoneColour;// = new Colour4f();
	FVector baseBoneConstraintAxis;// = new Vec3f();

	for (int chainLoop = 0; chainLoop < numChains; ++chainLoop)
	{
		// Choose the bone colours and base bone constraint axes
		switch (chainLoop % 3)
		{
		case 0:
			boneColour = UFabrikUtil::MID_RED;//.set(Utils.MID_RED);
			baseBoneColour = UFabrikUtil::RED; // .set(Utils.RED);
			baseBoneConstraintAxis = UFabrikUtil::X_AXIS;
			break;
		case 1:
			boneColour = UFabrikUtil::MID_GREEN;
			baseBoneColour = UFabrikUtil::MID_GREEN;
			baseBoneConstraintAxis = UFabrikUtil::Y_AXIS;
			break;
		case 2:
			boneColour = UFabrikUtil::MID_BLUE;
			baseBoneColour = UFabrikUtil::BLUE;
			baseBoneConstraintAxis = UFabrikUtil::VectorNegated(UFabrikUtil::Z_AXIS);// .negated();
			break;
		}

		// Create a new chain
		UFabrikChain* chain = NewObject<UFabrikChain>(); // new FabrikChain3D();

		// Set up the initial base bone location...
		FVector startLoc = FVector(0.0f, 0.0f, -40.0f);
		startLoc = UFabrikUtil::RotateYDegs(startLoc, rotStep * (float)chainLoop); // Vec3f.rotateYDegs(startLoc, rotStep * (float)chainLoop);
		FVector endLoc = startLoc + (baseBoneConstraintAxis * (DefaultBoneLength * 2.0f));

		// ...then create a base bone, set its colour, add it to the chain and specify that it should be global rotor constrained.
		UFabrikBone* basebone = NewObject<UFabrikBone>(); //  new FabrikBone3D(startLoc, endLoc);
		basebone->Init(startLoc, endLoc);
		basebone->Color = baseBoneColour;
		chain->AddBone(basebone);
		chain->SetRotorBaseboneConstraint(EBoneConstraintType::BCT_GlobalRotor, baseBoneConstraintAxis, baseBoneConstraintAngleDegs);

		// Add additional consecutive, unconstrained bones to the chain
		for (int boneLoop = 0; boneLoop < 7; boneLoop++)
		{
			boneColour = (boneLoop % 2 == 0) ? UFabrikUtil::Lighten(boneColour, 0.5f) : UFabrikUtil::Darken(boneColour, 0.5f); // boneColour.lighten(0.5f) : boneColour.darken(0.5f);
			chain->AddConsecutiveBone(DefaultBoneDirection, DefaultBoneLength, boneColour);
		}

		// Finally, add the chain to the structure
		Structure->AddChain(chain);
	}
}

void AFabrikDemoActor::DemoFreelyRotatingGlobalHinges()
{
	//demoName = "Demo 4 - Freely Rotating Global Hinges";
	Structure = NewObject<UFabrikStructure>();// new FabrikStructure3D(demoName);
	int numChains = 3;
	float rotStep = 360.0f / (float)numChains;

	// We'll create a circular arrangement of 3 chains which are each constrained about different global axes.
	// Note: Although I've used the cardinal X/Y/Z axes here, any axis can be used.
	FVector globalHingeAxis;// = new Vec3f();
	for (int chainLoop = 0; chainLoop < numChains; ++chainLoop)
	{
		// Set colour and axes							
		FColor chainColour;// = new Colour4f();
		switch (chainLoop % numChains)
		{
		case 0:
			chainColour = UFabrikUtil::RED;
			globalHingeAxis = UFabrikUtil::X_AXIS;
			break;
		case 1:
			chainColour = UFabrikUtil::GREEN;
			globalHingeAxis = UFabrikUtil::Y_AXIS;
			break;
		case 2:
			chainColour = UFabrikUtil::BLUE;
			globalHingeAxis = UFabrikUtil::Z_AXIS;
			break;
		}

		// Create a new chain
		UFabrikChain* chain = NewObject<UFabrikChain>();// new FabrikChain3D();

		// Set up the initial base bone location...
		FVector startLoc(0.0f, 0.0f, -40.0f);
		startLoc = UFabrikUtil::RotateYDegs(startLoc, rotStep * (float)chainLoop);
		FVector endLoc = startLoc + (DefaultBoneDirection * DefaultBoneLength);

		// ...then create a base bone, set its colour, and add it to the chain.
		UFabrikBone* basebone = NewObject<UFabrikBone>();// new FabrikBone3D(startLoc, endLoc);
		basebone->Init(startLoc, endLoc);
		basebone->Color = chainColour;
		chain->AddBone(basebone);

		// Add alternating global hinge constrained and unconstrained bones to the chain
		for (int boneLoop = 0; boneLoop < 7; boneLoop++)
		{
			if (boneLoop % 2 == 0)
			{
				chain->AddConsecutiveFreelyRotatingHingedBoneC(DefaultBoneDirection, DefaultBoneLength, EJointType::JT_GlobalHinge, globalHingeAxis, UFabrikUtil::GREY);
			}
			else
			{
				chain->AddConsecutiveBone(DefaultBoneDirection, DefaultBoneLength, chainColour);
			}
		}

		// Finally, add the chain to the structure
		Structure->AddChain(chain);
	}
}

void AFabrikDemoActor::DemoGlobalHingesWithReferenceAxisConstraints()
{
	//demoName = "Demo 5 - Global Hinges With Reference Axis Constraints";
	Structure = NewObject<UFabrikStructure>();//mStructure = new FabrikStructure3D(demoName);

	// Create a new chain				
	UFabrikChain* chain = NewObject<UFabrikChain>();// new FabrikChain3D();

	// Set up the initial base bone location...
	FVector startLoc(0.0f, 30.0f, -40.0f);
	FVector endLoc(startLoc);
	endLoc.Y -= DefaultBoneLength;

	// ...then create a base bone, set its colour, and add it to the chain.
	UFabrikBone* basebone = NewObject<UFabrikBone>();// new FabrikBone3D(startLoc, endLoc);
	basebone->Init(startLoc, endLoc);
	basebone->Color = UFabrikUtil::YELLOW;
	chain->AddBone(basebone);

	// Add alternating global hinge constrained and unconstrained bones to the chain
	float cwDegs = 120.0f;
	float acwDegs = 120.0f;
	for (int boneLoop = 0; boneLoop < 8; ++boneLoop)
	{
		if (boneLoop % 2 == 0)
		{
			// Params: bone direction, bone length, joint type, hinge rotation axis, clockwise constraint angle, anticlockwise constraint angle, hinge constraint reference axis, colour
			// Note: There is a version of this method where you do not specify the colour - the default is to draw the bone in white.
			chain->AddConsecutiveHingedBoneC(UFabrikUtil::VectorNegated( UFabrikUtil::Y_AXIS), DefaultBoneLength, EJointType::JT_GlobalHinge, UFabrikUtil::Z_AXIS, cwDegs, acwDegs, UFabrikUtil::VectorNegated( UFabrikUtil::Y_AXIS), UFabrikUtil::GREY);
		}
		else
		{
			chain->AddConsecutiveBone(UFabrikUtil::VectorNegated( UFabrikUtil::Y_AXIS ), DefaultBoneLength, UFabrikUtil::MID_GREEN);
		}
	}

	// Finally, add the chain to the structure
	Structure->AddChain(chain);
}

void AFabrikDemoActor::DemoFreelyRotatingLocalHinges()
{
	//demoName = "Demo 6 - Freely Rotating Local Hinges";
	Structure = NewObject<UFabrikStructure>();//mStructure = new FabrikStructure3D(demoName);
	int numChains = 3;

	// We'll create a circular arrangement of 3 chains with alternate bones each constrained about different local axes.
	// Note: Local hinge rotation axes are relative to the rotation matrix of the previous bone in the chain.
	FVector hingeRotationAxis;// = new Vec3f();;

	float rotStep = 360.0f / (float)numChains;
	for (int loop = 0; loop < numChains; loop++)
	{
		// Set colour and axes							
		FColor chainColour;// = new Colour4f();
		switch (loop % 3)
		{
		case 0:
			chainColour = UFabrikUtil::RED;
			hingeRotationAxis = UFabrikUtil::X_AXIS;
			break;
		case 1:
			chainColour = UFabrikUtil::GREEN;
			hingeRotationAxis = UFabrikUtil::Y_AXIS;
			break;
		case 2:
			chainColour = UFabrikUtil::BLUE;
			hingeRotationAxis = UFabrikUtil::Z_AXIS;
			break;
		}

		// Create a new chain
		UFabrikChain* chain = NewObject<UFabrikChain>();//FabrikChain3D chain = new FabrikChain3D();

		// Set up the initial base bone location...
		FVector startLoc(0.0f, 0.0f, -40.0f);
		startLoc = UFabrikUtil::RotateYDegs(startLoc, rotStep * (float)loop);
		FVector endLoc = startLoc + (DefaultBoneDirection * DefaultBoneLength);

		// ...then create a base bone, set its colour, and add it to the chain.
		//FabrikBone3D basebone = new FabrikBone3D(startLoc, endLoc);
		UFabrikBone* basebone = NewObject<UFabrikBone>();// new FabrikBone3D(startLoc, endLoc);
		basebone->Init(startLoc, endLoc);
		basebone->Color = chainColour;
		chain->AddBone(basebone);

		// Add alternating local hinge constrained and unconstrained bones to the chain
		for (int boneLoop = 0; boneLoop < 6; boneLoop++)
		{
			if (boneLoop % 2 == 0)
			{
				chain->AddConsecutiveFreelyRotatingHingedBoneC(DefaultBoneDirection, DefaultBoneLength, EJointType::JT_LocalHinge, hingeRotationAxis, UFabrikUtil::GREY);
			}
			else
			{
				chain->AddConsecutiveBone(DefaultBoneDirection, DefaultBoneLength, chainColour);
			}
		}

		// Finally, add the chain to the structure
		Structure->AddChain(chain);
	}
}

void AFabrikDemoActor::DemoLocalHingesWithReferenceAxisConstraints()
{
	//demoName = "Demo 7 - Local Hinges with Reference Axis Constraints";
	Structure = NewObject<UFabrikStructure>();//mStructure = new FabrikStructure3D(demoName);
	int numChains = 3;

	// We'll create a circular arrangement of 3 chains with alternate bones each constrained about different local axes.
	// Note: Local hinge rotation axes are relative to the rotation matrix of the previous bone in the chain.
	FVector hingeRotationAxis;// = new Vec3f();
	FVector hingeReferenceAxis;// = new Vec3f();

	float rotStep = 360.0f / (float)numChains;
	for (int loop = 0; loop < numChains; loop++)
	{
		// Set colour and axes							
		FColor chainColour;// = new Colour4f();
		switch (loop % 3)
		{
		case 0:
			chainColour = UFabrikUtil::RED;
			hingeRotationAxis = UFabrikUtil::X_AXIS;
			hingeReferenceAxis = UFabrikUtil::Y_AXIS;
			break;
		case 1:
			chainColour = UFabrikUtil::GREEN;
			hingeRotationAxis = UFabrikUtil::Y_AXIS;
			hingeReferenceAxis = UFabrikUtil::X_AXIS;
			break;
		case 2:
			chainColour = UFabrikUtil::BLUE;
			hingeRotationAxis = UFabrikUtil::Z_AXIS;
			hingeReferenceAxis = UFabrikUtil::Y_AXIS;
			break;
		}

		// Create a new chain
		UFabrikChain* chain = NewObject<UFabrikChain>();//FabrikChain3D chain = new FabrikChain3D();

		// Set up the initial base bone location...
		FVector startLoc(0.0f, 0.0f, -40.0f);
		startLoc = UFabrikUtil::RotateYDegs(startLoc, rotStep * (float)loop);
		FVector endLoc = startLoc + (DefaultBoneDirection * DefaultBoneLength);

		// ...then create a base bone, set its colour, and add it to the chain.
		//FabrikBone3D basebone = new FabrikBone3D(startLoc, endLoc);
		UFabrikBone* basebone = NewObject<UFabrikBone>();// new FabrikBone3D(startLoc, endLoc);
		basebone->Init(startLoc, endLoc);
		basebone->Color = chainColour;
		chain->AddBone(basebone);

		// Add alternating local hinge constrained and unconstrained bones to the chain
		float constraintAngleDegs = 90.0f;
		for (int boneLoop = 0; boneLoop < 6; boneLoop++)
		{
			if (boneLoop % 2 == 0)
			{
				chain->AddConsecutiveHingedBoneC(DefaultBoneDirection, DefaultBoneLength, EJointType::JT_LocalHinge, hingeRotationAxis, constraintAngleDegs, constraintAngleDegs, hingeReferenceAxis, UFabrikUtil::GREY);
			}
			else
			{
				chain->AddConsecutiveBone(DefaultBoneDirection, DefaultBoneLength, chainColour);
			}
		}

		// Finally, add the chain to the structure
		Structure->AddChain(chain);
	}
}

void AFabrikDemoActor::DemoConnectedChains()
{
	//demoName = "Demo 8 - Connected Chains";
	Structure = NewObject<UFabrikStructure>();//mStructure = new FabrikStructure3D(demoName);
	FColor boneColour = UFabrikUtil::GREEN;

	// Create a new chain...				
	UFabrikChain* chain = NewObject<UFabrikChain>();//FabrikChain3D chain = new FabrikChain3D();

	// ...then create a basebone, set its draw colour and add it to the chain.
	FVector startLoc(0.0f, 0.0f, 40.0f);
	FVector endLoc = startLoc + (DefaultBoneDirection * DefaultBoneLength);
	UFabrikBone* basebone = NewObject<UFabrikBone>();//FabrikBone3D basebone = new FabrikBone3D(startLoc, endLoc);
	basebone->Init(startLoc, endLoc);
	basebone->Color = boneColour;
	chain->AddBone(basebone);

	// Add additional consecutive, unconstrained bones to the chain				
	for (int boneLoop = 0; boneLoop < 5; boneLoop++)
	{
		boneColour = (boneLoop % 2 == 0) ? UFabrikUtil::Lighten( boneColour,0.4f) : UFabrikUtil::Darken( boneColour, 0.4f);
		chain->AddConsecutiveBone(DefaultBoneDirection, DefaultBoneLength, boneColour);
	}

	// Finally, add the chain to the structure
	Structure->AddChain(chain);
	UFabrikChain* secondChain = NewObject<UFabrikChain>();// new FabrikChain3D("Second Chain");
	//FabrikBone3D base = new FabrikBone3D(FVector(100.0f), new FVector(110.0f));
	UFabrikBone* base = NewObject<UFabrikBone>();// new FabrikBone3D(startLoc, endLoc);
	base->Init(FVector(100.0f), FVector(110.0f));
	secondChain->AddBone(base);
	secondChain->AddConsecutiveBone(UFabrikUtil::X_AXIS, 20.0f);
	secondChain->AddConsecutiveBone(UFabrikUtil::Y_AXIS, 20.0f);
	secondChain->AddConsecutiveBone(UFabrikUtil::Z_AXIS, 20.0f);

	// Set the colour of all bones in the chain in a single call, then connect it to the chain...
	secondChain->SetColour (UFabrikUtil::RED);
	Structure->ConnectChain(secondChain, 0, 0, EBoneConnectionPoint::BCP_Start);

	// ...we can keep adding the same chain at various points if we like, because the chain we
	// connect is actually a clone of the one we provide, and not the original 'secondChain' argument.
	secondChain->SetColour(UFabrikUtil::WHITE);
	Structure->ConnectChain(secondChain, 0, 2, EBoneConnectionPoint::BCP_Start);

	// We can also set connect the chain to the end of a specified bone (this overrides the START/END 
	// setting of the bone we connect to).
	secondChain->SetColour(UFabrikUtil::BLUE);
	Structure->ConnectChain(secondChain, 0, 4, EBoneConnectionPoint::BCP_End);
}

void AFabrikDemoActor::DemoGlobalRotorConstrainedConnectedChains()
{
	//demoName = "Demo 9 - Global Rotor Constrained Connected Chains";
	Structure = NewObject<UFabrikStructure>();//mStructure = new FabrikStructure3D(demoName);
	FColor boneColour = UFabrikUtil::GREEN;//new Colour4f(Utils.GREEN);

	// Create a new chain...				
	UFabrikChain* chain = NewObject<UFabrikChain>();//FabrikChain3D chain = new FabrikChain3D();

	// ...then create a basebone, set its draw colour and add it to the chain.
	FVector startLoc(0.0f, 0.0f, 40.0f);
	FVector endLoc = startLoc + (DefaultBoneDirection * DefaultBoneLength);
	UFabrikBone* basebone = NewObject<UFabrikBone>();// ->Init(startLoc, endLoc);  // FabrikBone3D basebone = new FabrikBone3D(startLoc, endLoc);
	basebone->Init(startLoc, endLoc);

	basebone->Color = boneColour;
	chain->AddBone(basebone);

	// Add additional consecutive, unconstrained bones to the chain				
	for (int boneLoop = 0; boneLoop < 7; boneLoop++)
	{
		boneColour = (boneLoop % 2 == 0) ? UFabrikUtil::Lighten(boneColour,0.4f) : UFabrikUtil::Darken(boneColour, 0.4f);
		chain->AddConsecutiveBone(DefaultBoneDirection, DefaultBoneLength, boneColour);
	}

	// Finally, add the chain to the structure
	Structure->AddChain(chain);
	UFabrikChain* secondChain = NewObject<UFabrikChain>();//FabrikChain3D secondChain = new FabrikChain3D("Second Chain");
	UFabrikBone* base = NewObject<UFabrikBone>();
	base->Init(FVector::ZeroVector, FVector(15.0f, 0.0f, 0.0f));  // FabrikBone3D base = new FabrikBone3D(FVector(), FVector(15.0f, 0.0f, 0.0f));
	secondChain->AddBone(base);
	secondChain->SetRotorBaseboneConstraint(EBoneConstraintType::BCT_GlobalRotor, UFabrikUtil::X_AXIS, 45.0f);

	secondChain->AddConsecutiveBone(UFabrikUtil::X_AXIS, 15.0f);
	secondChain->AddConsecutiveBone(UFabrikUtil::X_AXIS, 15.0f);
	secondChain->AddConsecutiveBone(UFabrikUtil::X_AXIS, 15.0f);
	secondChain->SetColour(UFabrikUtil::RED);

	Structure->ConnectChain(secondChain, 0, 3, EBoneConnectionPoint::BCP_Start);

	UFabrikChain* thirdChain = NewObject<UFabrikChain>();//FabrikChain3D thirdChain = new FabrikChain3D("Second Chain");
	base = NewObject<UFabrikBone>();
	base->Init(FVector::ZeroVector, FVector(0.0f, 15.0f, 0.0f));  //base = new FabrikBone3D( FVector(),  FVector(0.0f, 15.0f, 0.0f));
	thirdChain->AddBone(base);
	thirdChain->SetRotorBaseboneConstraint(EBoneConstraintType::BCT_GlobalRotor, UFabrikUtil::Y_AXIS, 45.0f);

	thirdChain->AddConsecutiveBone(UFabrikUtil::Y_AXIS, 15.0f);
	thirdChain->AddConsecutiveBone(UFabrikUtil::Y_AXIS, 15.0f);
	thirdChain->AddConsecutiveBone(UFabrikUtil::Y_AXIS, 15.0f);
	thirdChain->SetColour(UFabrikUtil::BLUE);

	Structure->ConnectChain(thirdChain, 0, 6, EBoneConnectionPoint::BCP_Start);
}

void AFabrikDemoActor::DemoLocalRotorConstrainedConnectedChains()
{
	//demoName = "Demo 10 - Local Rotor Constrained Connected Chains";
	Structure = NewObject<UFabrikStructure>();//mStructure = new FabrikStructure3D(demoName);
	FColor boneColour = UFabrikUtil::GREEN;//new Colour4f(Utils.GREEN);

	// Create a new chain...				
	UFabrikChain* chain = NewObject<UFabrikChain>();//FabrikChain3D chain = new FabrikChain3D();

	// ...then create a basebone, set its draw colour and add it to the chain.
	FVector startLoc(0.0f, 0.0f, 40.0f);
	FVector endLoc = startLoc + (DefaultBoneDirection * DefaultBoneLength);
	UFabrikBone* basebone = NewObject<UFabrikBone>()->Init(startLoc, endLoc);  //FabrikBone3D basebone = new FabrikBone3D(startLoc, endLoc);
	basebone->Color = boneColour;
	chain->AddBone(basebone);

	// Add additional consecutive, unconstrained bones to the chain				
	for (int boneLoop = 0; boneLoop < 7; boneLoop++)
	{
		boneColour = (boneLoop % 2 == 0) ? UFabrikUtil::Lighten(boneColour,0.4f) : UFabrikUtil::Darken(boneColour, 0.4f);
		chain->AddConsecutiveBone(DefaultBoneDirection, DefaultBoneLength, boneColour);
	}

	// Finally, add the chain to the structure
	Structure->AddChain(chain);

	// Create a second chain which will have a relative (i.e. local) rotor basebone constraint about the X axis.
	UFabrikChain* secondChain = NewObject<UFabrikChain>();//FabrikChain3D secondChain = new FabrikChain3D("Second Chain");
	basebone = NewObject<UFabrikBone>()->Init(FVector::ZeroVector, FVector(15.0f, 0.0f, 0.0f));  //new FabrikBone3D(FVector(), FVector(15.0f, 0.0f, 0.0f));
	secondChain->AddBone(basebone);
	secondChain->SetRotorBaseboneConstraint(EBoneConstraintType::BCT_LocalRotor, UFabrikUtil::X_AXIS, 45.0f);

	// Add some additional bones
	secondChain->AddConsecutiveBone(UFabrikUtil::X_AXIS, 15.0f);
	secondChain->AddConsecutiveBone(UFabrikUtil::X_AXIS, 15.0f);
	secondChain->AddConsecutiveBone(UFabrikUtil::X_AXIS, 15.0f);
	secondChain->SetColour(UFabrikUtil::RED);

	// Connect this second chain to the start point of bone 3 in chain 0 of the structure
	Structure->ConnectChain(secondChain, 0, 3, EBoneConnectionPoint::BCP_Start);
}

void AFabrikDemoActor::DemoConnectedChainsWithFreelyRotatingGlobalHingedBaseboneConstraints()
{
	// demoName = "Demo 11 - Connected Chains with Freely-Rotating Global Hinged Basebone Constraints";
	Structure = NewObject<UFabrikStructure>();//mStructure = new FabrikStructure3D(demoName);
	FColor boneColour = UFabrikUtil::GREEN;//new Colour4f(Utils.GREEN);

	// Create a new chain...				
	UFabrikChain* chain = NewObject<UFabrikChain>();//FabrikChain3D chain = new FabrikChain3D();

	// ...then create a basebone, set its draw colour and add it to the chain.
	FVector startLoc(0.0f, 0.0f, 40.0f);
	FVector endLoc = startLoc + (DefaultBoneDirection * DefaultBoneLength);
	UFabrikBone* basebone = NewObject<UFabrikBone>()->Init(startLoc, endLoc);  //FabrikBone3D basebone = new FabrikBone3D(startLoc, endLoc);
	basebone->Color = boneColour;
	chain->AddBone(basebone);

	// Add additional consecutive, unconstrained bones to the chain				
	for (int boneLoop = 0; boneLoop < 7; boneLoop++)
	{
		boneColour = (boneLoop % 2 == 0) ? UFabrikUtil::Lighten(boneColour,0.4f) : UFabrikUtil::Darken(boneColour, 0.4f);
		chain->AddConsecutiveBone(DefaultBoneDirection, DefaultBoneLength, boneColour);
	}

	// Finally, add the chain to the structure
	Structure->AddChain(chain);

	// Create a second chain which will have a relative (i.e. local) rotor basebone constraint about the X axis.
	UFabrikChain* secondChain = NewObject<UFabrikChain>();//FabrikChain3D secondChain = new FabrikChain3D("Second Chain");
	UFabrikBone* base = NewObject<UFabrikBone>()->Init(FVector::ZeroVector, FVector(15.0f, 0.0f, 0.0f)); //FabrikBone3D base = new FabrikBone3D(FVector(), FVector(15.0f, 0.0f, 0.0f));
	secondChain->AddBone(base);

	// Set this second chain to have a freely rotating global hinge which rotates about the Y axis
	// Note: We MUST add the basebone to the chain before we can set the basebone constraint on it.
	secondChain->SetFreelyRotatingGlobalHingedBasebone(UFabrikUtil::Y_AXIS);

	// Add some additional bones
	secondChain->AddConsecutiveBone(UFabrikUtil::X_AXIS, 15.0f);
	secondChain->AddConsecutiveBone(UFabrikUtil::X_AXIS, 15.0f);
	secondChain->AddConsecutiveBone(UFabrikUtil::X_AXIS, 15.0f);
	secondChain->SetColour(UFabrikUtil::GREY);

	// Connect this second chain to the start point of bone 3 in chain 0 of the structure
	Structure->ConnectChain(secondChain, 0, 3, EBoneConnectionPoint::BCP_Start );
}

void AFabrikDemoActor::DemoConnectedChainsWithEmbeddedTargets()
{
	//demoName = "Demo 12 - Connected chains with embedded targets";
	Structure = NewObject<UFabrikStructure>();//mStructure = new FabrikStructure3D(demoName);
	FColor boneColour = UFabrikUtil::GREEN;//new Colour4f(Utils.GREEN);

	// Create a new chain...				
	UFabrikChain* chain = NewObject<UFabrikChain>();//FabrikChain3D chain = new FabrikChain3D();

	// ...then create a basebone, set its draw colour and add it to the chain.
	FVector startLoc(0.0f, 0.0f, 40.0f);
	FVector endLoc = startLoc + (DefaultBoneDirection * DefaultBoneLength);
	UFabrikBone* basebone = NewObject<UFabrikBone>()->Init(startLoc, endLoc);  //FabrikBone3D basebone = new FabrikBone3D(startLoc, endLoc);
	basebone->Color = boneColour;
	chain->AddBone(basebone);

	// Add additional consecutive, unconstrained bones to the chain				
	for (int boneLoop = 0; boneLoop < 7; boneLoop++)
	{
		boneColour = (boneLoop % 2 == 0) ? UFabrikUtil::Lighten(boneColour,0.4f) : UFabrikUtil::Darken(boneColour, 0.4f);
		chain->AddConsecutiveBone(DefaultBoneDirection, DefaultBoneLength, boneColour);
	}

	// Finally, add the chain to the structure
	Structure->AddChain(chain);

	// Create a second chain which will have a relative (i.e. local) rotor basebone constraint about the X axis.
	UFabrikChain* secondChain = NewObject<UFabrikChain>();//FabrikChain3D secondChain = new FabrikChain3D("Second Chain");
	secondChain->UseEmbeddedTarget = true; //  SetEmbeddedTargetMode(true);
	UFabrikBone* base = NewObject<UFabrikBone>()->Init(FVector::ZeroVector, FVector(15.0f, 0.0f, 0.0f));  //FabrikBone3D base = new FabrikBone3D(FVector(), FVector(15.0f, 0.0f, 0.0f));
	secondChain->AddBone(base);

	// Set this second chain to have a freely rotating global hinge which rotates about the Y axis
	// Note: We MUST add the basebone to the chain before we can set the basebone constraint on it.				
	secondChain->SetHingeBaseboneConstraint(EBoneConstraintType::BCT_GlobalHinge, UFabrikUtil::Y_AXIS, 90.0f, 45.0f, UFabrikUtil::X_AXIS);

	/** Other potential options for basebone constraint types **/
	//secondChain.setFreelyRotatingGlobalHingedBasebone(Y_AXIS);
	//secondChain.setFreelyRotatingLocalHingedBasebone(Y_AXIS);
	//secondChain.setHingeBaseboneConstraint(BaseboneConstraintType3D.GLOBAL_HINGE, Y_AXIS, 90.0f, 45.0f, X_AXIS);
	//secondChain.setRotorBaseboneConstraint(BaseboneConstraintType3D.GLOBAL_ROTOR, Z_AXIS, 30.0f, 60.0f, Y_AXIS);
	//secondChain.setRotorBaseboneConstraint(BaseboneConstraintType3D.LOCAL_ROTOR, Z_AXIS, 30.0f, 60.0f, Y_AXIS);

	// Add some additional bones
	secondChain->AddConsecutiveBone(UFabrikUtil::X_AXIS, 20.0f);
	secondChain->AddConsecutiveBone(UFabrikUtil::X_AXIS, 20.0f);
	secondChain->AddConsecutiveBone(UFabrikUtil::X_AXIS, 20.0f);
	secondChain->SetColour(UFabrikUtil::GREY);

	// Connect this second chain to the start point of bone 3 in chain 0 of the structure
	Structure->ConnectChain(secondChain, 0, 3, EBoneConnectionPoint::BCP_Start);
}
