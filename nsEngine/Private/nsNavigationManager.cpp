#include "nsNavigationManager.h"
#include "nsWorld.h"
#include "nsMesh.h"
#include "nsRenderer.h"
#include "nsRenderComponents.h"
#include "nsNavigationComponents.h"
#include "nsConsole.h"

// Recast & Detour
#include "Recast.h"
#include "DebugDraw.h"
#include "RecastDebugDraw.h"
#include "DetourCrowd.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"



NS_ENGINE_DEFINE_HANDLE(nsNavigationAgentID);


nsLogCategory NavigationLog(TEXT("nsNavigationLog"), nsELogVerbosity::LV_DEBUG);



class nsRecastContext : public rcContext
{
public:
	virtual void doLog(const rcLogCategory category, const char* msg, const int len) override
	{
		if (category == RC_LOG_PROGRESS)
		{
			NS_CONSOLE_Debug(NavigationLog, TEXT("Recast: %s"), msg);
		}
		else if (category == RC_LOG_WARNING)
		{
			NS_CONSOLE_Warning(NavigationLog, TEXT("Recast: %s"), msg);
		}
		else if (category == RC_LOG_ERROR)
		{
			NS_CONSOLE_Error(NavigationLog, TEXT("Recast: %s"), msg);
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
		NS_ValidateV(0, TEXT("Not implemented yet!"));
	}


	virtual void texture(bool state) override
	{
		NS_ValidateV(0, TEXT("Not implemented yet!"));
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
		NS_ValidateV(0, TEXT("Not implemented yet!"));
	}


	/// Submit a vertex
	///  @param x,y,z [in] position of the verts.
	///  @param color [in] color of the verts.
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v) override
	{
		NS_ValidateV(0, TEXT("Not implemented yet!"));
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
			NS_ValidateV(0, TEXT("Not implemented yet!"));
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

	dtNavMesh* DetourNavMesh;
	dtNavMeshQuery* DetourNavMeshQuery;

	bool bBuilding;


public:
	nsNavMeshBuildTask()
	{
		CompactHeightField = nullptr;
		ContourSet = nullptr;
		PolyMesh = nullptr;
		PolyMeshDetail = nullptr;
		
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

		if (CompactHeightField)
		{
			rcFreeCompactHeightfield(CompactHeightField);
			CompactHeightField = nullptr;
		}

		if (ContourSet)
		{
			rcFreeContourSet(ContourSet);
			ContourSet = nullptr;
		}

		if (PolyMesh)
		{
			rcFreePolyMesh(PolyMesh);
			PolyMesh = nullptr;
		}

		if (PolyMeshDetail)
		{
			rcFreePolyMeshDetail(PolyMeshDetail);
			PolyMeshDetail = nullptr;
		}

		DetourNavMesh = nullptr;
		DetourNavMeshQuery = nullptr;

		bBuilding = false;
	}


	virtual void Execute() noexcept override
	{
		NS_CONSOLE_Log(NavigationLog, TEXT("Building navigation mesh"));

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

		const int walkableHeight = nsMath::Ceil<int>(200.0f / cellSize.Y);
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

		for (int i = 0; i < PolyMesh->npolys; ++i)
		{
			if (PolyMesh->areas[i] == RC_WALKABLE_AREA)
			{
				PolyMesh->flags[i] = 1;
			}
		}

		dtNavMeshCreateParams navMeshCreateParams{};
		nsPlatform::Memory_Copy(navMeshCreateParams.bmin, PolyMesh->bmin, sizeof(float) * 3);
		nsPlatform::Memory_Copy(navMeshCreateParams.bmax, PolyMesh->bmax, sizeof(float) * 3);
		navMeshCreateParams.buildBvTree = true;
		navMeshCreateParams.ch = PolyMesh->ch;
		navMeshCreateParams.cs = PolyMesh->cs;
		navMeshCreateParams.detailMeshes = PolyMeshDetail->meshes;
		navMeshCreateParams.detailTriCount = PolyMeshDetail->ntris;
		navMeshCreateParams.detailTris = PolyMeshDetail->tris;
		navMeshCreateParams.detailVerts = PolyMeshDetail->verts;
		navMeshCreateParams.detailVertsCount = PolyMeshDetail->nverts;
		navMeshCreateParams.nvp = PolyMesh->nvp;
		navMeshCreateParams.polyAreas = PolyMesh->areas;
		navMeshCreateParams.polyCount = PolyMesh->npolys;
		navMeshCreateParams.polyFlags = PolyMesh->flags;
		navMeshCreateParams.polys = PolyMesh->polys;
		navMeshCreateParams.vertCount = PolyMesh->nverts;
		navMeshCreateParams.verts = PolyMesh->verts;
		navMeshCreateParams.walkableClimb = 30.0f;
		navMeshCreateParams.walkableHeight = 200.0f;
		navMeshCreateParams.walkableRadius = 36.0f;
		
		uint8* navData = nullptr;
		int navDataSize = 0;
		bSuccess = dtCreateNavMeshData(&navMeshCreateParams, &navData, &navDataSize);
		NS_Assert(bSuccess);

		dtStatus detourStatus = 0;

		NS_Assert(DetourNavMesh);
		detourStatus = DetourNavMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		NS_Assert(dtStatusSucceed(detourStatus));

		NS_Assert(DetourNavMeshQuery);
		detourStatus = DetourNavMeshQuery->init(DetourNavMesh, 2048);
		NS_Assert(dtStatusSucceed(detourStatus));


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
	DetourCrowd = nullptr;
	DetourNavMesh = nullptr;
	DetourNavMeshQuery = nullptr;
}


void nsNavigationManager::Initialize()
{
	if (bInitialized)
	{
		return;
	}

	DetourCrowd = dtAllocCrowd();
	DetourCrowd->init(NS_ENGINE_NAVIGATION_MAX_AGENT, 40.0f, nullptr);

	bInitialized = true;
}


void nsNavigationManager::BuildNavMesh(nsWorld* world)
{
	if (DetourNavMesh)
	{
		dtFreeNavMesh(DetourNavMesh);
	}

	DetourNavMesh = dtAllocNavMesh();

	if (DetourNavMeshQuery)
	{
		dtFreeNavMeshQuery(DetourNavMeshQuery);
	}

	DetourNavMeshQuery = dtAllocNavMeshQuery();


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
	
	NavMeshBuildTask.DetourNavMesh = DetourNavMesh;
	NavMeshBuildTask.DetourNavMeshQuery = DetourNavMeshQuery;
	NavMeshBuildTask.Execute();

	const bool bSuccess = DetourCrowd->init(NS_ENGINE_NAVIGATION_MAX_AGENT, 40.0f, DetourNavMesh);
	NS_Assert(bSuccess);
}


void nsNavigationManager::BuildNavMesh(const nsNavigationInputGeometry& inputGeometry, const nsNavigationBuildSettings& buildSettings)
{
	NS_ValidateV(0, TEXT("Not implemented yet!"));
}


nsNavigationAgentID nsNavigationManager::CreateAgent(nsNavigationAgentComponent* component)
{
	NS_Assert(component);

	const int stateId = AgentStates.Add();

	NavAgentState& state = AgentStates[stateId];
	state.MoveTargetPosition = nsVector3::ZERO;

	dtCrowdAgentParams dtParams{};
	dtParams.radius = component->Radius;
	dtParams.height = component->Height;
	dtParams.maxAcceleration = component->MaxAcceleration;
	dtParams.maxSpeed = component->MaxSpeed;
	dtParams.collisionQueryRange = component->Radius * 8.0f;
	dtParams.pathOptimizationRange = component->Radius * 16.0f;
	dtParams.updateFlags = 0;
	dtParams.obstacleAvoidanceType = 3;
	dtParams.separationWeight = 2.0f;
	dtParams.userData = component;
	dtParams.updateFlags = DT_CROWD_OBSTACLE_AVOIDANCE;

	const nsVector3 initialPosition = component->GetWorldPosition();
	state.AgentIndex = DetourCrowd->addAgent((const float*)&initialPosition, &dtParams);

	return stateId;
}


void nsNavigationManager::DestroyAgent(nsNavigationAgentID& agent)
{
	if (IsAgentValid(agent))
	{
		NavAgentState& state = AgentStates[agent.Id];
		DetourCrowd->removeAgent(state.AgentIndex);
		AgentStates.RemoveAt(agent.Id);
	}

	agent = nsNavigationAgentID::INVALID;
}


void nsNavigationManager::UpdateAgentParams(nsNavigationAgentID agent)
{
	NS_Assert(IsAgentValid(agent));

	NavAgentState& state = AgentStates[agent.Id];

	if (state.AgentIndex != -1)
	{
		dtCrowdAgent* dtAgent = DetourCrowd->getEditableAgent(state.AgentIndex);

		nsNavigationAgentComponent* component = static_cast<nsNavigationAgentComponent*>(dtAgent->params.userData);
		NS_Assert(component);

		dtAgent->params.radius = component->Radius;
		dtAgent->params.height = component->Height;
		dtAgent->params.maxAcceleration = component->MaxAcceleration;
		dtAgent->params.maxSpeed = component->MaxSpeed;
	}
}


void nsNavigationManager::SetAgentMoveTarget(nsNavigationAgentID agent, const nsVector3& targetPosition)
{
	NS_Assert(IsAgentValid(agent));

	NavAgentState& state = AgentStates[agent.Id];
	state.MoveTargetPosition = targetPosition;
	
	if (state.AgentIndex != -1)
	{
		dtPolyRef nearestPoly = 0;
		float nearestPosition[3];
		bool bSuccess = dtStatusSucceed(DetourNavMeshQuery->findNearestPoly((const float*)&state.MoveTargetPosition, DetourCrowd->getQueryExtents(), DetourCrowd->getFilter(0), &nearestPoly, nearestPosition));

		if (bSuccess)
		{
			bSuccess = DetourCrowd->requestMoveTarget(state.AgentIndex, nearestPoly, nearestPosition);
		}

		if (!bSuccess)
		{
			NS_CONSOLE_Warning(NavigationLog, TEXT("Request move failed!"));
		}
	}
}


void nsNavigationManager::StopAgentMovement(nsNavigationAgentID agent)
{
	NS_Assert(IsAgentValid(agent));

	DetourCrowd->resetMoveTarget(AgentStates[agent.Id].AgentIndex);
}


void nsNavigationManager::MoveAgents(float deltaTime)
{
	DetourCrowd->update(deltaTime, nullptr);

	for (auto it = AgentStates.CreateConstIterator(); it; ++it)
	{
		const int index = it.GetIndex();
		const NavAgentState& state = it.GetValue();

		if (state.AgentIndex == -1)
		{
			continue;
		}

		const dtCrowdAgent* dtAgent = DetourCrowd->getAgent(state.AgentIndex);
		nsNavigationAgentComponent* component = static_cast<nsNavigationAgentComponent*>(dtAgent->params.userData);

		if (component)
		{
			component->Internal_SyncWithDetourCrowdAgent(
				deltaTime,
				nsVector3(dtAgent->npos[0], dtAgent->npos[1], dtAgent->npos[2]),
				nsVector3(dtAgent->vel[0], dtAgent->vel[1], dtAgent->vel[2]),
				nsVector3(dtAgent->dvel[0], dtAgent->dvel[1], dtAgent->dvel[2])
			);
		}
	}
}



#ifdef NS_ENGINE_DEBUG_DRAW
static nsRecastDebugDraw RecastDebugDraw;


void nsNavigationManager::DebugDraw(nsRenderer* renderer)
{
	RecastDebugDraw.Renderer = renderer;
	duDebugDrawPolyMesh(&RecastDebugDraw, *NavMeshBuildTask.PolyMesh);
}

#endif // NS_ENGINE_DEBUG_DRAW