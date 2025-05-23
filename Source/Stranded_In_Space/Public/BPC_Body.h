// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BPC_Body.generated.h"

UCLASS()
class STRANDED_IN_SPACE_API ABPC_Body : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABPC_Body();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	// UPROPERTY(EditAnywhere)
	// USceneComponent* Root;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* Sphere;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetMass();

	UFUNCTION(BlueprintCallable, BlueprintPure)
    FVector GetWorldLocation();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCurrentForceCPP(FVector force, float DeltaSeconds);

};
