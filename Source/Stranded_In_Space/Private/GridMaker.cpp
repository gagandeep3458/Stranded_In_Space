// Fill out your copyright notice in the Description page of Project Settings.


#include "GridMaker.h"

struct FMyShaderStruct
{
	float a;
	float b;
	float c;
};

// Sets default values
AGridMaker::AGridMaker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    TArray<FMyShaderStruct> StructArray;

    FMyShaderStruct temp;
    temp.a = 1.0;
    temp.b = 1.0;
    temp.c = 0.0;

    StructArray.Add(temp);

    // 2. Create Shader Resource View (SRV)
    //FShaderResourceViewRHIRef SRV = RHICreateShaderResourceView(StructuredBufferRHI);
}

// Called when the game starts or when spawned
void AGridMaker::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGridMaker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

