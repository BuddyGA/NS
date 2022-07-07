#include "nsNavigationManager.h"
#include "nsWorld.h"
#include "nsRenderComponents.h"
#include "nsMesh.h"
#include "nsRenderer.h"
#include "nsConsole.h"
#include "Recast.h"
#include "DebugDraw.h"
#include "RecastDebugDraw.h"



static nsLogCategory NavigationLog("nsNavigationLog", nsELogVerbosity::LV_DEBUG);



class nsRecastContext : public rcContext
{
public:
	virtual void doLog(const rcLogCategory category, const char* msg, const int len) override
	{
		if (category == RC_LOG_PROGRESS)
		{
			NS_CONSOLE_Debug(NavigationLog, "Recast: %s", msg);
		}
		else if (category == RC_LOG_WARNING)
		{
			NS_CONSOLE_Warning(NavigationLog, "Recast: %s", msg);
		}
		else if (category == RC_LOG_ERROR)
		{
			NS_CONSOLE_Error(NavigationLog, "Recast: %s", msg);
		}
	}

};



class nsRecastDebugDraw : public duDebugDraw
{
private:
	nsTArray<nsVector3> VertexPositions;
	nsTArray<nsColor> VertexColors;
	duDebugDrawPrimitives Type;

public:
	nsRenderer* Renderer;


public:
	nsRecastDebugDraw()
	{
		Renderer = nullptr;
	}


	~nsRecastDebugDraw()
	{

	}


	virtual void depthMask(bool state) override
	{
		NS_ValidateV(0, "Not implemented yet!");
	}


	virtual void texture(bool state) override
	{
		NS_ValidateV(0, "Not implemented yet!");
	}


	/// Begin drawing primitives.
	///  @param prim [in] primitive type to draw, one of rcDebugDrawPrimitives.
	///  @param size [in] size of a primitive, applies to point size and line width only.
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f) override
	{
		VertexPositions.Clear();
		VertexColors.Clear();
		Type = prim;
	}


	/// Submit a vertex
	///  @param pos [in] position of the verts.
	///  @param color [in] color of the verts.
	virtual void vertex(const float* pos, unsigned int color) override
	{
		VertexPositions.Add(nsVector3(pos[0], pos[1], pos[2]));
		VertexColors.Add(color);
	}


	/// Submit a vertex
	///  @param x,y,z [in] position of the verts.
	///  @param color [in] color of the verts.
	virtual void vertex(const float x, const float y, const float z, unsigned int color) override
	{
		VertexPositions.Add(nsVector3(x, y, z));
		VertexColors.Add(color);
	}


	/// Submit a vertex
	///  @param pos [in] position of the verts.
	///  @param color [in] color of the verts.
	virtual void vertex(const float* pos, unsigned int color, const float* uv) override
	{
		NS_ValidateV(0, "Not implemented yet!");
	}


	/// Submit a vertex
	///  @param x,y,z [in] position of the verts.
	///  @param color [in] color of the verts.
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v) override
	{
		NS_ValidateV(0, "Not implemented yet!");
	}


	/// End drawing primitives.
	virtual void end() override
	{
		NS_Assert(Renderer);
		NS_Assert(VertexPositions.GetCount() == VertexColors.GetCount());

		if (Type == DU_DRAW_POINTS)
		{
			for (int v = 0; v < VertexPositions.GetCount(); ++v)
			{
				Renderer->DebugDrawMeshAABB(VertexPositions[v] - 1.0f, VertexPositions[v] + 1.0f, VertexColors[v]);
			}
		}
		else if (Type == DU_DRAW_LINES)
		{
			NS_Assert(VertexPositions.GetCount() % 2 == 0);

			for (int v = 0; v < VertexPositions.GetCount(); v += 2)
			{
				Renderer->DebugDrawLine(VertexPositions[v], VertexPositions[v + 1], VertexColors[v]);
			}
		}
		else if (Type == DU_DRAW_TRIS)
		{
			NS_Assert(VertexPositions.GetCount() % 3 == 0);

			for (int v = 0; v < VertexPositions.GetCount(); v += 3)
			{
				Renderer->DebugDrawMeshTriangle(VertexPositions[v], VertexPositions[v + 1], VertexPositions[v + 2], nsColor(50, 100, 150));
			}
		}
		else
		{
			NS_ValidateV(0, "Not implemented yet!");
		}
	}

};




class nsNavMeshBuildTask : public nsIThreadTask
{
private:
	nsAtomic bDone;
	nsRecastContext Context;


public:
	nsNavigationInputGeometry InputGeometry;
	rcCompactHeightfield* CompactHeightField;
	rcContourSet* ContourSet;
	rcPolyMesh* PolyMesh;
	rcPolyMeshDetail* PolyMeshDetail;
	bool bBuilding;


public:
	nsNavMeshBuildTask()
	{
		Reset();
	}


	~nsNavMeshBuildTask() noexcept
	{

	}


	virtual void Reset() noexcept override
	{
		bDone.Set(0);
		Context.resetTimers();
		InputGeometry.Clear();
		bBuilding = false;
	}


	virtual void Execute() noexcept override
	{
		nsAABB bounding;
		bounding.Min = nsVector3(-2000.0f, -16.0f, -2000.0f);
		bounding.Max = nsVector3(2000.0f, 256.0f, 2000.0f);

		const nsVector2 cellSize = nsVector2(16.0f, 8.0f);
		int width, height;
		rcCalcGridSize((const float*)&bounding.Min, (const float*)(&bounding.Max), cellSize.X, &width, &height);

		rcHeightfield* heightField = rcAllocHeightfield();
		bool bSuccess = rcCreateHeightfield(&Context, *heightField, width, height, (const float*)&bounding.Min, (const float*)&bounding.Max, cellSize.X, cellSize.Y);
		NS_Assert(bSuccess);

		nsTArray<uint8> triAreas(InputGeometry.GetTriangleCount());

		const float walkableSlopeDegree = 45.0f;
		rcMarkWalkableTriangles(&Context, walkableSlopeDegree, (const float*)InputGeometry.GetVertexData(), InputGeometry.GetVertexCount(), (const int*)InputGeometry.GetIndexData(), InputGeometry.GetTriangleCount(), triAreas.GetData());
		bSuccess = rcRasterizeTriangles(&Context, (const float*)InputGeometry.GetVertexData(), triAreas.GetData(), InputGeometry.GetTriangleCount(), *heightField);
		NS_Assert(bSuccess);

		const int walkableHeight = nsMath::Ceil<int>(180.0f / cellSize.Y);
		const int walkableClimb = nsMath::Floor<int>(30.0f / cellSize.Y);
		rcFilterLowHangingWalkableObstacles(&Context, walkableClimb, *heightField);
		rcFilterLedgeSpans(&Context, walkableHeight, walkableClimb, *heightField);
		rcFilterWalkableLowHeightSpans(&Context, walkableHeight, *heightField);

		CompactHeightField = rcAllocCompactHeightfield();
		bSuccess = rcBuildCompactHeightfield(&Context, walkableHeight, walkableClimb, *heightField, *CompactHeightField);
		NS_Assert(bSuccess);

		const int walkableRadius = nsMath::Ceil<int>(36.0f / cellSize.X);
		bSuccess = rcErodeWalkableArea(&Context, walkableRadius, *CompactHeightField);
		NS_Assert(bSuccess);

		bSuccess = rcBuildDistanceField(&Context, *CompactHeightField);
		NS_Assert(bSuccess);

		const int minRegionArea = 16;
		const int mergeRegionArea = 400;
		bSuccess = rcBuildRegions(&Context, *CompactHeightField, 0, minRegionArea, mergeRegionArea);
		NS_Assert(bSuccess);

		const float maxError = 1.0f;
		const int maxEdgeLength = 64;
		ContourSet = rcAllocContourSet();
		bSuccess = rcBuildContours(&Context, *CompactHeightField, maxError, maxEdgeLength, *ContourSet);
		NS_Assert(bSuccess);

		const int maxVertPerPoly = 6;
		PolyMesh = rcAllocPolyMesh();
		bSuccess = rcBuildPolyMesh(&Context, *ContourSet, maxVertPerPoly, *PolyMesh);
		NS_Assert(bSuccess);

		const float sampleDist = cellSize.X * 4.0f;
		const float sampleMaxError = cellSize.Y;
		PolyMeshDetail = rcAllocPolyMeshDetail();
		bSuccess = rcBuildPolyMeshDetail(&Context, *PolyMesh, *CompactHeightField, sampleDist, sampleMaxError, *PolyMeshDetail);
		NS_Assert(bSuccess);


		bDone.Set(1);
	}


	virtual bool IsIdle() const noexcept override
	{
		return !bBuilding;
	}


	virtual bool IsRunning() const noexcept override
	{
		return bBuilding;
	}


	virtual bool IsDone() const noexcept override
	{
		return bDone.Get() == 1;
	}


#ifdef _DEBUG
	virtual nsString GetDebugName() const noexcept 
	{ 
		return "nsNavMeshBuildTask";
	}
#endif // _DEBUG

};


static nsNavMeshBuildTask NavMeshBuildTask;




nsNavigationManager::nsNavigationManager() noexcept
{
	bInitialized = false;
	bBuildingNavMesh = false;
}


void nsNavigationManager::Initialize()
{
	if (bInitialized)
	{
		return;
	}


	bInitialized = true;
}


void nsNavigationManager::BuildNavMesh(nsWorld* world)
{
	NavMeshBuildTask.Reset();

	const nsTArray<nsActor*>& allActors = world->GetAllActors();

	for (int act = 0; act < allActors.GetCount(); ++act)
	{
		nsActor* actor = allActors[act];

		if (!actor->IsStatic())
		{
			continue;
		}

		nsMeshComponent* meshComp = actor->GetComponent<nsMeshComponent>();

		if (meshComp == nullptr || !meshComp->bGenerateNavMesh)
		{
			continue;
		}

		const nsMatrix4 worldTransformMatrix = meshComp->GetWorldTransform().ToMatrix();
		const nsMeshID mesh = meshComp->GetModelAsset().GetMeshes()[0];
		const nsMeshVertexData& vertexData = nsMeshManager::Get().GetMeshVertexData(mesh, 0);

		for (int i = 0; i < vertexData.Indices.GetCount(); i += 3)
		{
			NavMeshBuildTask.InputGeometry.AddTriangle(
				vertexData.Positions[vertexData.Indices[i]] * worldTransformMatrix,
				vertexData.Positions[vertexData.Indices[i + 1]] * worldTransformMatrix,
				vertexData.Positions[vertexData.Indices[i + 2]] * worldTransformMatrix
			);
		}
	}
	
	NavMeshBuildTask.Execute();
}


void nsNavigationManager::BuildNavMesh(const nsNavigationInputGeometry& inputGeometry, const nsNavigationBuildSettings& buildSettings)
{
}



#ifdef NS_ENGINE_DEBUG_DRAW
static nsRecastDebugDraw RecastDebugDraw;


void nsNavigationManager::DebugDraw(nsRenderer* renderer)
{
	RecastDebugDraw.Renderer = renderer;
	duDebugDrawPolyMesh(&RecastDebugDraw, *NavMeshBuildTask.PolyMesh);
}

#endif // NS_ENGINE_DEBUG_DRAW