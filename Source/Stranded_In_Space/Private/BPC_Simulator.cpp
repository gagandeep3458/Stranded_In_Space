// Fill out your copyright notice in the Description page of Project Settings.


#include "BPC_Simulator.h"

#include "Kismet/BlueprintTypeConversions.h"
#include "SpaceEffects/Public/S_GravityGrid/S_GravityGrid.h"

// Sets default values
ABPC_Simulator::ABPC_Simulator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABPC_Simulator::BeginPlay()
{
	Super::BeginPlay();
	
}

TArray<FVector> ABPC_Simulator::GetPredectivePointWithCPP(const FVector StartingPos, const FVector StartingVel, const int NumOfPoints, ABPC_Body* body, const float DeltaSeconds, const TArray<ABPC_Body*> bodies, const float GConstant)
{
	TArray<FVector> Points;

	FVector CurrentLocation = StartingPos;
	FVector CurrentVelocity = StartingVel;
	const float DeltaSecondsAdjusted = DeltaSeconds * 3;

	for (int i = 0;i < NumOfPoints;i++) 
	{
		FVector f = GetForceOnBodyWithLocationCPP(bodies, CurrentLocation, GConstant, body);

		CurrentVelocity += (((f / body->GetMass())) * DeltaSecondsAdjusted);

		CurrentLocation += CurrentVelocity * DeltaSecondsAdjusted;

		Points.Add(CurrentLocation);
	}

	return Points;
}

FVector ABPC_Simulator::GetForceOnBodyWithLocationCPP(TArray<ABPC_Body*> bodies, FVector Location, float GConstant, ABPC_Body* BodyA)
{
	FVector ForceOnBody = FVector();

	int bodiesSize = bodies.Num();

	for (int i = 0;i < bodiesSize;i++)
	{
		ABPC_Body* OtherBody = bodies[i];

		if (BodyA != OtherBody)
		{

			FVector distVector = (OtherBody->GetActorLocation() - Location);
			FVector unitVector = distVector.GetSafeNormal();

			float dist = distVector.SizeSquared();

			ForceOnBody += ((GConstant * BodyA->GetMass() * OtherBody->GetMass()) / (dist)) * unitVector;
		}
	}

	return ForceOnBody;
}

FVector ABPC_Simulator::GetForceOnBodyCPP(TArray<ABPC_Body*> bodies, float GConstant, ABPC_Body* BodyA)
{
	FVector ForceOnBody = FVector();

	int bodiesSize = bodies.Num();

	for (int i = 0;i < bodiesSize;i++)
	{
		ABPC_Body* OtherBody = bodies[i];

		if (BodyA != OtherBody)
		{

			FVector distVector = (OtherBody->GetActorLocation() - BodyA->GetActorLocation());
			FVector unitVector = distVector.GetSafeNormal();

			float distSquared = distVector.SizeSquared();

			ForceOnBody += ((GConstant * BodyA->Sphere->GetMass() * OtherBody->Sphere->GetMass()) / (distSquared)) * unitVector;
		}
	}

	return ForceOnBody;
}

float MapRangeClamped(float InputValue, float InRangeA, float InRangeB, float OutRangeA, float OutRangeB)
{
	// Handle the case where the input range is zero to prevent division by zero.
	if (InRangeA == InRangeB)
	{
		return FMath::Clamp(OutRangeA, FMath::Min(OutRangeA, OutRangeB), FMath::Max(OutRangeA, OutRangeB));
	}

	float Alpha = (InputValue - InRangeA) / (InRangeB - InRangeA);
	float OutputValue = FMath::Lerp(OutRangeA, OutRangeB, Alpha);
	return FMath::Clamp(OutputValue, FMath::Min(OutRangeA, OutRangeB), FMath::Max(OutRangeA, OutRangeB));
}

FVector2f ConvertWorldSpaceToScreenSpace(const FVector& WorldLocation, const FTransform& PlaneTransform, const FVector& PlaneLocalBoundsMin, const FVector& PlaneLocalBoundsMax)
{

	TArray<float> VectorArray;
	VectorArray.Add(WorldLocation.X);
	VectorArray.Add(WorldLocation.Y);
	VectorArray.Add(WorldLocation.Z);
	
	FVector3f InverseLocation = static_cast<FVector3f>(PlaneTransform.InverseTransformPosition(WorldLocation));
	
	float U = MapRangeClamped(InverseLocation[0], PlaneLocalBoundsMin[0], PlaneLocalBoundsMax[0], 0.f, 1.f);
	float V = MapRangeClamped(InverseLocation[1], PlaneLocalBoundsMin[1], PlaneLocalBoundsMax[1], 0.f, 1.f);
	
	return FVector2f(U, V);
}

void ABPC_Simulator::CalculateForcesAndNotifyCPP(TArray<ABPC_Body*> bodies, float GConstant, float DeltaSeconds, UTextureRenderTarget2D* RT, UTexture2D* GridTexture, const FTransform& PlaneTransform, const FVector& PlaneLocalBoundsMin, const FVector& PlaneLocalBoundsMax)
{
	int bodiesSize = bodies.Num();

	// Preparing Data for Dispatch
	FSimDrawDispatchParams Params = FSimDrawDispatchParams(RT->SizeX, RT->SizeY, 1);
	Params.RenderTarget = RT->GameThread_GetRenderTargetResource();
	Params.GridTexture = GridTexture;

	FSimBodiesData TestBodyData;
	TestBodyData.NumActive = bodiesSize;

	for (int i = 0;i < bodiesSize;i++)
	{
		// Simulation work
		ABPC_Body* body = bodies[i];

		FVector force = GetForceOnBodyCPP(bodies, GConstant, body);

		body->SetCurrentForceCPP(force, DeltaSeconds);

		// Dispatch work for Drawing

		const ABPC_Body* TestBody = bodies[i];
		const FVector TestBodyLocation = TestBody->GetActorLocation();

		TestBodyData.Positions[i] = ConvertWorldSpaceToScreenSpace(TestBodyLocation, PlaneTransform, PlaneLocalBoundsMin, PlaneLocalBoundsMax);
		TestBodyData.Masses[i] = TestBody->Sphere->GetMass();
	}

	Params.SimBodiesData = TestBodyData;

	UE_LOG(LogTemp, Display, TEXT("MyVector: X=%.2f, Y=%.2f"), Params.SimBodiesData.Positions[0].X, Params.SimBodiesData.Positions[0].Y);

	// Dispatching Data to Shader Plugin
	FS_GravityGridInterface::Dispatch(Params);
}

// Called every frame
void ABPC_Simulator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

