#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialRenderProxy.h"

#include "S_GravityGrid.generated.h"

struct SPACEEFFECTS_API FSimBodiesData
{
	FVector2f Positions[20];
	float Masses[20];
	int NumActive;
};

struct SPACEEFFECTS_API FSimDrawDispatchParams
{
	int X;
	int Y;
	int Z;
	
	FRenderTarget* RenderTarget;
	UTexture2D* GridTexture;
	FSimBodiesData SimBodiesData;
	
	FSimDrawDispatchParams(int x, int y, int z) : X(x), Y(y), Z(z), RenderTarget(nullptr), GridTexture(nullptr),
	                                              SimBodiesData()
	{
	}
};

// This is a public interface that we define so outside code can invoke our compute shader.
class SPACEEFFECTS_API FS_GravityGridInterface {
public:
	// Executes this shader on the render thread
	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList,
		FSimDrawDispatchParams Params
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void DispatchGameThread(
		FSimDrawDispatchParams Params
	)
	{
		ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
		[Params](FRHICommandListImmediate& RHICmdList)
		{
			DispatchRenderThread(RHICmdList, Params);
		});
	}

	// Dispatches this shader. Can be called from any thread
	static void Dispatch(
		FSimDrawDispatchParams Params
	)
	{
		if (IsInRenderingThread()) {
			DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), Params);
		}else{
			DispatchGameThread(Params);
		}
	}
};

// This is a static blueprint library that can be used to invoke our compute shader from blueprints.
UCLASS()
class SPACEEFFECTS_API US_GravityGridLibrary : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static void ExecuteRTComputeShader(UTextureRenderTarget2D* RT)
	{
		// Create a dispatch parameters struct and fill it the input array with our args
		FSimDrawDispatchParams Params(RT->SizeX, RT->SizeY, 1);
		Params.RenderTarget = RT->GameThread_GetRenderTargetResource();

		FS_GravityGridInterface::Dispatch(Params);
	}
};
