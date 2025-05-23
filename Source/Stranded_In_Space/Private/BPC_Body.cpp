// Fill out your copyright notice in the Description page of Project Settings.


#include "BPC_Body.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ABPC_Body::ABPC_Body()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	// RootComponent = Root;

	Sphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
	RootComponent = Sphere;
	// Sphere->SetupAttachment(Root);
}

// Called when the game starts or when spawned
void ABPC_Body::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABPC_Body::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float ABPC_Body::GetMass()
{
	return Sphere->GetMass();
}

FVector ABPC_Body::GetWorldLocation()
{
	return Sphere->GetComponentLocation();
}
