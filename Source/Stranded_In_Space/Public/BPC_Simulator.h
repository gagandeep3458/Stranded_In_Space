// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <BPC_Body.h>
#include "Logging/LogMacros.h"
#include "BPC_Simulator.generated.h"

UCLASS()
class STRANDED_IN_SPACE_API ABPC_Simulator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABPC_Simulator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetPredectivePointWithCPP(FVector StartingPos, FVector StartingVel, int NumOfPoints, ABPC_Body* body, float DeltaSeconds, TArray<ABPC_Body*> bodies, float GConstant);

	UFUNCTION(BlueprintCallable)
	FVector GetForceOnBodyWithLocationCPP(TArray<ABPC_Body*> bodies, FVector Location, float GConstant, ABPC_Body* BodyA);

	UFUNCTION(BlueprintCallable)
	FVector GetForceOnBodyCPP(TArray<ABPC_Body*> bodies, float GConstant, ABPC_Body* BodyA);

	UFUNCTION(BlueprintCallable)
	void CalculateForcesAndNotifyCPP(TArray<ABPC_Body*> bodies, float GConstant, float DeltaSeconds, UTextureRenderTarget2D* RT, UTexture2D* GridTexture, const FTransform& PlaneTransform, const FVector& PlaneLocalBoundsMin, const FVector& PlaneLocalBoundsMax);

};
