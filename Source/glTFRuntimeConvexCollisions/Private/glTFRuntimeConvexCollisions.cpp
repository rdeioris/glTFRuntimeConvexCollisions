// Copyright 2020-2022, Roberto De Ioris.

#include "glTFRuntimeConvexCollisions.h"


THIRD_PARTY_INCLUDES_START
#define ENABLE_VHACD_IMPLEMENTATION 1
#define VHACD_DISABLE_THREADING 1
#include "VHACD/VHACD.h"
THIRD_PARTY_INCLUDES_END

#include "glTFRuntimeParser.h"
#include "glTFRuntimeConvexCollisionConfig.h"
#include "PhysicsEngine/ConvexElem.h"
#include "PhysicsEngine/BodySetup.h"

#define LOCTEXT_NAMESPACE "FglTFRuntimeConvexCollisionsModule"

namespace
{
	static void GenerateConvexCollision(UStaticMesh* StaticMesh, UglTFRuntimeConvexCollisionConfig* Config, TArray<FKConvexElem>& ConvexElements)
	{
		FStaticMeshRenderData* RenderData = StaticMesh->GetRenderData();
		if (!RenderData)
		{
			return;
		}

		VHACD::IVHACD::Parameters Params;
		Params.m_resolution = Config->Resolution;
		Params.m_maxNumVerticesPerCH = Config->MaxVertices;
		Params.m_maxConvexHulls = Config->MaxHulls;
		Params.m_shrinkWrap = true;
		Params.m_minimumVolumePercentErrorAllowed = 0;

		VHACD::IVHACD* VHACDInterface = VHACD::CreateVHACD();

		uint32 NumVertices = RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer.GetNumVertices();
		uint32 NumIndices = RenderData->LODResources[0].IndexBuffer.GetNumIndices();

		TArray<double> Points;
		Points.AddUninitialized(NumVertices * 3);

		for (uint32 VertexIndex = 0; VertexIndex < NumVertices; VertexIndex++)
		{
			const FVector3f& Position = RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex);
			Points[VertexIndex * 3] = Position.X;
			Points[VertexIndex * 3 + 1] = Position.Y;
			Points[VertexIndex * 3 + 2] = Position.Z;
		}

		TArray<uint32> Indices;
		RenderData->LODResources[0].IndexBuffer.GetCopy(Indices);

		if (VHACDInterface->Compute(Points.GetData(), Points.Num() / 3, Indices.GetData(), Indices.Num() / 3, Params))
		{

			int32 NumHulls = VHACDInterface->GetNConvexHulls();
			for (int32 HullIndex = 0; HullIndex < NumHulls; HullIndex++)
			{
				FKConvexElem ConvexElem;

				VHACD::IVHACD::ConvexHull Hull;
				VHACDInterface->GetConvexHull(HullIndex, Hull);

				for (uint32 VertexIndex = 0; VertexIndex < Hull.m_points.size(); VertexIndex++)
				{
					FVector Vertex;
					Vertex.X = static_cast<float>(Hull.m_points[VertexIndex].mX);
					Vertex.Y = static_cast<float>(Hull.m_points[VertexIndex].mY);
					Vertex.Z = static_cast<float>(Hull.m_points[VertexIndex].mZ);
					ConvexElem.VertexData.Add(Vertex);
				}
				ConvexElem.UpdateElemBox();

				ConvexElements.Add(MoveTemp(ConvexElem));
			}
		}

		VHACDInterface->Clean();
		VHACDInterface->Release();
	}

	static void UpdateBodySetup(UStaticMesh* StaticMesh, const TArray<FKConvexElem>& ConvexElements)
	{
		if (ConvexElements.Num() > 0)
		{
			UBodySetup* BodySetup = StaticMesh->GetBodySetup();

			BodySetup->AggGeom.ConvexElems = ConvexElements;

			BodySetup->bCreatedPhysicsMeshes = false;
			BodySetup->bNeverNeedsCookedCollisionData = false;

			BodySetup->CreatePhysicsMeshes();

			if (UActorComponent* ActorComponent = Cast<UActorComponent>(StaticMesh->GetOuter()))
			{
				ActorComponent->RecreatePhysicsState();
			}
		}
	}
}

void FglTFRuntimeConvexCollisionsModule::BuildConvexCollisions(UStaticMesh* StaticMesh, UglTFRuntimeConvexCollisionConfig* Config)
{
	TArray<FKConvexElem> ConvexElements;
	GenerateConvexCollision(StaticMesh, Config, ConvexElements);
	UpdateBodySetup(StaticMesh, ConvexElements);
}

void FglTFRuntimeConvexCollisionsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FglTFRuntimeParser::OnFinalizedStaticMesh.AddLambda([](TSharedRef<FglTFRuntimeParser> Parser, UStaticMesh* StaticMesh, const FglTFRuntimeStaticMeshConfig& StaticMeshConfig)
		{

			UglTFRuntimeConvexCollisionConfig* Config = StaticMeshConfig.GetCustomConfig<UglTFRuntimeConvexCollisionConfig>();
			if (!Config)
			{
				return;
			}

			if (!StaticMeshConfig.Outer || !StaticMesh->GetWorld() || !StaticMesh->GetWorld()->IsGameWorld())
			{
				Parser->AddError("FinalizeStaticMesh", "Unable to generate Convex collision without a valid StaticMesh Outer (consider setting it to the related StaticMeshComponent)");
				return;
			}

			if (Config->bAutoConvexCollision)
			{
				if (!Config->bConvexCollisionsAsyncGeneration)
				{
					TArray<FKConvexElem> ConvexElements;
					GenerateConvexCollision(StaticMesh, Config, ConvexElements);
					UpdateBodySetup(StaticMesh, ConvexElements);
				}
				else
				{
					Async(EAsyncExecution::Thread, [StaticMesh, Config]()
						{
							TArray<FKConvexElem> ConvexElements;
							GenerateConvexCollision(StaticMesh, Config, ConvexElements);

							FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([StaticMesh, Config, &ConvexElements]()
								{
									if (StaticMesh->IsValidLowLevel())
									{
										UpdateBodySetup(StaticMesh, ConvexElements);

										TArray<FglTFRuntimeConvexElem> ReturnedConvexElements;
										for (const FKConvexElem& ConvexElem : ConvexElements)
										{
											FglTFRuntimeConvexElem NewConvexElem;
											NewConvexElem.Vertices = ConvexElem.VertexData;
											ReturnedConvexElements.Add(MoveTemp(NewConvexElem));
										}
										Config->OnConvexCollisionGenerated.Broadcast(StaticMesh, ReturnedConvexElements);
									}
								}, TStatId(), nullptr, ENamedThreads::GameThread);
							FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
						});
				}
			}
			// manual setup
			else
			{
				TArray<FKConvexElem> ConvexElements;
				for (const FglTFRuntimeConvexElem& ConvexElem : Config->ConvexElements)
				{
					FKConvexElem NewConvexElem;
					NewConvexElem.VertexData = ConvexElem.Vertices;
					NewConvexElem.UpdateElemBox();

					ConvexElements.Add(MoveTemp(NewConvexElem));
				}

				UpdateBodySetup(StaticMesh, ConvexElements);
			}
		});
}

void FglTFRuntimeConvexCollisionsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FglTFRuntimeConvexCollisionsModule, glTFRuntimeConvexCollisions)