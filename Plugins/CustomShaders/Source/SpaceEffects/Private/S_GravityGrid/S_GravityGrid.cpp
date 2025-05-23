#include "S_GravityGrid.h"
#include "SpaceEffects/Public/S_GravityGrid/S_GravityGrid.h"
#include "PixelShaderUtils.h"
#include "MeshPassProcessor.inl"
#include "StaticMeshResources.h"
#include "DynamicMeshBuilder.h"
#include "RenderGraphResources.h"
#include "GlobalShader.h"
#include "UnifiedBuffer.h"
#include "CanvasTypes.h"
#include "MeshDrawShaderBindings.h"
#include "RHIGPUReadback.h"
#include "MeshPassUtils.h"
#include "MaterialShader.h"

DECLARE_STATS_GROUP(TEXT("S_GravityGrid"), STATGROUP_S_GravityGrid, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("S_GravityGrid Execute"), STAT_S_GravityGrid_Execute, STATGROUP_S_GravityGrid);

// This class carries our parameter declarations and acts as the bridge between cpp and HLSL.
class SPACEEFFECTS_API FS_GravityGrid : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FS_GravityGrid);
	SHADER_USE_PARAMETER_STRUCT(FS_GravityGrid, FGlobalShader);


	class FS_GravityGrid_Perm_TEST : SHADER_PERMUTATION_INT("TEST", 1);

	using FPermutationDomain = TShaderPermutationDomain<
		FS_GravityGrid_Perm_TEST
	>;

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		/*
		* Here's where you define one or more of the input parameters for your shader.
		* Some examples:
		*/
		// SHADER_PARAMETER(uint32, MyUint32) // On the shader side: uint32 MyUint32;
		// SHADER_PARAMETER(FVector3f, MyVector) // On the shader side: float3 MyVector;

		// SHADER_PARAMETER_TEXTURE(Texture2D, MyTexture) // On the shader side: Texture2D<float4> MyTexture; (float4 should be whatever you expect each pixel in the texture to be, in this case float4(R,G,B,A) for 4 channels)
		// SHADER_PARAMETER_SAMPLER(SamplerState, MyTextureSampler) // On the shader side: SamplerState MySampler; // CPP side: TStaticSamplerState<ESamplerFilter::SF_Bilinear>::GetRHI();

		// SHADER_PARAMETER_ARRAY(float, MyFloatArray, [3]) // On the shader side: float MyFloatArray[3];

		// SHADER_PARAMETER_UAV(RWTexture2D<FVector4f>, MyTextureUAV) // On the shader side: RWTexture2D<float4> MyTextureUAV;
		// SHADER_PARAMETER_UAV(RWStructuredBuffer<FMyCustomStruct>, MyCustomStructs) // On the shader side: RWStructuredBuffer<FMyCustomStruct> MyCustomStructs;
		// SHADER_PARAMETER_UAV(RWBuffer<FMyCustomStruct>, MyCustomStructs) // On the shader side: RWBuffer<FMyCustomStruct> MyCustomStructs;

		// SHADER_PARAMETER_SRV(StructuredBuffer<FMyCustomStruct>, MyCustomStructs) // On the shader side: StructuredBuffer<FMyCustomStruct> MyCustomStructs;
		// SHADER_PARAMETER_SRV(Buffer<FMyCustomStruct>, MyCustomStructs) // On the shader side: Buffer<FMyCustomStruct> MyCustomStructs;
		// SHADER_PARAMETER_SRV(Texture2D<FVector4f>, MyReadOnlyTexture) // On the shader side: Texture2D<float4> MyReadOnlyTexture;

		// SHADER_PARAMETER_STRUCT_REF(FMyCustomStruct, MyCustomStruct)


		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, RenderTarget)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StrucutredBuffer<FSimBodiesData>, SimBodiesData)
	    SHADER_PARAMETER_RDG_TEXTURE_SRV(RWTexture2D, GridTexture)
	    SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)

	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		const FPermutationDomain PermutationVector(Parameters.PermutationId);

		return true;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters,
	                                         FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		const FPermutationDomain PermutationVector(Parameters.PermutationId);

		/*
		* Here you define constants that can be used statically in the shader code.
		* Example:
		*/
		// OutEnvironment.SetDefine(TEXT("MY_CUSTOM_CONST"), TEXT("1"));

		/*
		* These defines are used in the thread count section of our shader
		*/
		OutEnvironment.SetDefine(TEXT("THREADS_X"), NUM_THREADS_S_GravityGrid_X);
		OutEnvironment.SetDefine(TEXT("THREADS_Y"), NUM_THREADS_S_GravityGrid_Y);
		OutEnvironment.SetDefine(TEXT("THREADS_Z"), NUM_THREADS_S_GravityGrid_Z);

		// This shader must support typed UAV load and we are testing if it is supported at runtime using RHIIsTypedUAVLoadSupported
		//OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);

		// FForwardLightingParameters::ModifyCompilationEnvironment(Parameters.Platform, OutEnvironment);
	}

private:
};

// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FS_GravityGrid, "/SpaceEffectsShaders/S_GravityGrid/S_GravityGrid.usf", "S_GravityGrid",
                        SF_Compute);

void FS_GravityGridInterface::DispatchRenderThread(FRHICommandListImmediate& RHICmdList,
                                                   FSimDrawDispatchParams Params)
{
	FRDGBuilder GraphBuilder(RHICmdList);

	{
		SCOPE_CYCLE_COUNTER(STAT_S_GravityGrid_Execute);
		DECLARE_GPU_STAT(S_GravityGrid)
		RDG_EVENT_SCOPE(GraphBuilder, "S_GravityGrid");
		RDG_GPU_STAT_SCOPE(GraphBuilder, S_GravityGrid);

		typename FS_GravityGrid::FPermutationDomain PermutationVector;

		// Add any static permutation options here
		// PermutationVector.Set<FS_GravityGrid::FMyPermutationName>(12345);

		TShaderMapRef<FS_GravityGrid> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel), PermutationVector);


		bool bIsShaderValid = ComputeShader.IsValid();

		if (bIsShaderValid)
		{
			FS_GravityGrid::FParameters* PassParameters = GraphBuilder.AllocParameters<FS_GravityGrid::FParameters>();

			// Passing Render Texture
			FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(Params.RenderTarget->GetSizeXY(), PF_B8G8R8A8,
			                                               FClearValueBinding::White,
			                                               TexCreate_RenderTargetable | TexCreate_ShaderResource |
			                                               TexCreate_UAV));
			FRDGTextureRef TmpTexture = GraphBuilder.CreateTexture(Desc, TEXT("S_GravityGrid_TempTexture"));
			FRDGTextureRef TargetTexture = RegisterExternalTexture(GraphBuilder,
			                                                       Params.RenderTarget->GetRenderTargetTexture(),
			                                                       TEXT("S_GravityGrid_RT"));
			PassParameters->RenderTarget = GraphBuilder.CreateUAV(TmpTexture);
			// ******************

			// Passing Grid Texture
			FIntPoint GridTextureSize(Params.GridTexture->GetSizeX(), Params.GridTexture->GetSizeY());

			FRDGTextureDesc GridTexDesc(FRDGTextureDesc::Create2D(GridTextureSize, PF_B8G8R8A8,
											   FClearValueBinding::White,
											   TexCreate_ShaderResource));
			FRDGTextureRef TmpTexture2 = GraphBuilder.CreateTexture(GridTexDesc, TEXT("S_GravityGrid_TempTextureGrid"));
			FRDGTextureRef TargetTexture2 = RegisterExternalTexture(GraphBuilder,
																   Params.GridTexture->GetResource()->GetTextureRHI(),
																   TEXT("S_GravityGrid_RT_Grid"));
			PassParameters->GridTexture = GraphBuilder.CreateSRV(TargetTexture2);


			auto Sampler = TStaticSamplerState<ESamplerFilter::SF_Bilinear>::GetRHI();
			PassParameters->InputSampler = Sampler;

			// *****************

			// Passing SimState Data

			FSimBodiesData SimBodiesData = Params.SimBodiesData;

			TArray<FSimBodiesData> SimBodiesDataArray;
			SimBodiesDataArray.Add(SimBodiesData);

			const FRDGBufferRef SimBodiesBuffer = CreateStructuredBuffer(
				GraphBuilder, TEXT("FSimBodiesData"), sizeof(FSimBodiesData), 1, SimBodiesDataArray.GetData(),
				sizeof(FSimBodiesData) * 1);

			PassParameters->SimBodiesData = GraphBuilder.CreateSRV(SimBodiesBuffer);
			// *****************


			// Passing Group Count
			auto GroupCount = FComputeShaderUtils::GetGroupCount(FIntVector(Params.X, Params.Y, Params.Z),
			                                                     FComputeShaderUtils::kGolden2DGroupSize);
			// Adding Pass
			GraphBuilder.AddPass(
				RDG_EVENT_NAME("ExecuteS_GravityGrid"),
				PassParameters,
				ERDGPassFlags::AsyncCompute,
				[&PassParameters, ComputeShader, GroupCount](FRHIComputeCommandList& RHICmdList)
				{
					FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *PassParameters, GroupCount);
				});


			// The copy will fail if we don't have matching formats, let's check and make sure we do.
			if (TargetTexture->Desc.Format == PF_B8G8R8A8)
			{
				AddCopyTexturePass(GraphBuilder, TmpTexture, TargetTexture, FRHICopyTextureInfo());
			}
			else
			{
#if WITH_EDITOR
				GEngine->AddOnScreenDebugMessage((uint64)42145125184, 6.f, FColor::Red,
				                                 FString(TEXT(
					                                 "The provided render target has an incompatible format (Please change the RT format to: RGBA8).")));
#endif
			}

			// The copy will fail if we don't have matching formats, let's check and make sure we do.
			if (TargetTexture2->Desc.Format == PF_B8G8R8A8)
			{
				AddCopyTexturePass(GraphBuilder, TmpTexture2, TargetTexture2, FRHICopyTextureInfo());
			}
			else
			{
#if WITH_EDITOR
				GEngine->AddOnScreenDebugMessage((uint64)42145125184, 6.f, FColor::Red,
												 FString(TEXT(
													 "The provided render target has an incompatible format (Please change the RT format to: RGBA8).")));
#endif
			}
		}
		else
		{
#if WITH_EDITOR
			GEngine->AddOnScreenDebugMessage((uint64)42145125184, 6.f, FColor::Red,
			                                 FString(TEXT("The compute shader has a problem.")));
#endif

			// We exit here as we don't want to crash the game if the shader is not found or has an error.
		}
	}

	GraphBuilder.Execute();
}
