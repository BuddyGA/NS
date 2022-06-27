#include "nsPhysics_PhysX.h"
#include "nsConsole.h"
#include "nsActor.h"
#include "nsViewport.h"
#include "nsPhysicsComponents.h"
#include "nsRenderContextWorld.h"
#include "physx/foundation/PxAllocatorCallback.h"
#include "physx/foundation//PxErrorCallback.h"


static nsLogCategory PhysXLog("nsPhysXLog", nsELogVerbosity::LV_DEBUG);



class nsPhysX_AllocatorCallback : public PxAllocatorCallback
{
private:
	nsCriticalSection CriticalSection;
	nsMemory Memory;

public:
	nsPhysX_AllocatorCallback()
	{
		Memory.Initialize("physx_allocator", NS_MEMORY_SIZE_MiB(16));
	}


	virtual void* allocate(size_t size, const char*, const char*, int) override
	{
		CriticalSection.Enter();
		void* data = Memory.Allocate(static_cast<int>(size));
		CriticalSection.Leave();

		return data;
	}


	virtual void deallocate(void* ptr) override
	{
		CriticalSection.Enter();
		Memory.Deallocate(ptr);
		CriticalSection.Leave();
	}

};



class nsPhysX_ErrorCallback : public PxErrorCallback
{
public:
	virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line) override
	{
		if (code == PxErrorCode::eABORT || code == PxErrorCode::eINTERNAL_ERROR || code == PxErrorCode::eOUT_OF_MEMORY)
		{
			NS_CONSOLE_Error(PhysXLog, "Error: %s", message);
			NS_Validate(0);
		}
		else if (code == PxErrorCode::eDEBUG_WARNING || code == PxErrorCode::ePERF_WARNING)
		{
			NS_CONSOLE_Warning(PhysXLog, "%s", message);
		}
		else if (code == PxErrorCode::eINVALID_OPERATION || code == PxErrorCode::eINVALID_PARAMETER)
		{
			NS_CONSOLE_Error(PhysXLog, "%s", message);
		}
	}

};



class nsPhysX_QueryFilterCallback : public PxQueryFilterCallback
{
public:
	nsPhysicsQueryIgnoredActors IgnoredActors;
	bool bIsMousePicking;


public:
	nsPhysX_QueryFilterCallback()
	{
		bIsMousePicking = false;
	}


	/**
	\brief This filter callback is executed before the exact intersection test if PxQueryFlag::ePREFILTER flag was set.

	\param[in] filterData custom filter data specified as the query's filterData.data parameter.
	\param[in] shape A shape that has not yet passed the exact intersection test.
	\param[in] actor The shape's actor.
	\param[in,out] queryFlags scene query flags from the query's function call (only flags from PxHitFlag::eMODIFIABLE_FLAGS bitmask can be modified)
	\return the updated type for this hit  (see #PxQueryHitType)
	*/
	virtual PxQueryHitType::Enum preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags) override
	{
		nsActor* actorToCheck = static_cast<nsCollisionComponent*>(actor->userData)->GetActor();
		NS_Assert(actorToCheck);

		if (IgnoredActors.Find(actorToCheck) != NS_ARRAY_INDEX_INVALID)
		{
			return PxQueryHitType::eNONE;
		}

		return bIsMousePicking ? PxQueryHitType::eBLOCK : PxQueryHitType::eTOUCH;
	}


	/**
	\brief This filter callback is executed if the exact intersection test returned true and PxQueryFlag::ePOSTFILTER flag was set.

	\param[in] filterData custom filter data of the query
	\param[in] hit Scene query hit information. faceIndex member is not valid for overlap queries. For sweep and raycast queries the hit information can be cast to #PxSweepHit and #PxRaycastHit respectively.
	\return the updated hit type for this hit  (see #PxQueryHitType)
	*/
	virtual PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit) override
	{
		return PxQueryHitType::eNONE;
	}

};



static PxFilterFlags ns_CollisionFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	// let triggers through
	if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlag::eDEFAULT;
	}

	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
	{
		pairFlags |= PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND;
		return PxFilterFlag::eDEFAULT;
	}

	return PxFilterFlag::eSUPPRESS;
}


static void ns_FillRayCastHitResult(nsPhysicsHitResult& outHitResult, const PxRaycastHit& rayCastHit, bool bIsBlock)
{
	outHitResult.Component = static_cast<nsCollisionComponent*>(rayCastHit.actor->userData);
	outHitResult.Actor = outHitResult.Component->GetActor();

	if (rayCastHit.flags & PxHitFlag::ePOSITION)
	{
		outHitResult.WorldPosition = NS_FromPxVec3(rayCastHit.position);
	}

	if (rayCastHit.flags & PxHitFlag::eNORMAL)
	{
		outHitResult.WorldNormal = NS_FromPxVec3(rayCastHit.normal);
	}

	outHitResult.Distance = rayCastHit.distance;
	outHitResult.bIsBlock = bIsBlock;
}


static void ns_FillSweepHitResult(nsPhysicsHitResult& outHitResult, const PxSweepHit& sweepHit, bool bIsBlock)
{
	outHitResult.Component = static_cast<nsCollisionComponent*>(sweepHit.actor->userData);
	outHitResult.Actor = outHitResult.Component->GetActor();

	if (sweepHit.flags & PxHitFlag::ePOSITION)
	{
		outHitResult.WorldPosition = NS_FromPxVec3(sweepHit.position);
	}

	if (sweepHit.flags & PxHitFlag::eNORMAL)
	{
		outHitResult.WorldNormal = NS_FromPxVec3(sweepHit.normal);
	}

	outHitResult.Distance = sweepHit.distance;
	outHitResult.bIsBlock = bIsBlock;
}



bool nsPhysX::SceneQueryRayCast(physx::PxScene* scene, nsPhysicsHitResult& outHitResult, const PxVec3& origin, const PxVec3& direction, float distance, const nsPhysicsQueryParams& params)
{
	NS_Assert(scene);

	PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;

	PxQueryFilterData queryFilterData{};
	queryFilterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
	queryFilterData.data.word1 = static_cast<PxU32>(params.Channel);

	nsPhysX_QueryFilterCallback queryFilterCallback;
	queryFilterCallback.IgnoredActors = params.IgnoredActors;

	PxRaycastBufferN<1> rayCastBuffer;
	bool bFoundHit = scene->raycast(origin, direction, distance, rayCastBuffer, hitFlags, queryFilterData, &queryFilterCallback);

	if (bFoundHit)
	{
		outHitResult = {};
		ns_FillRayCastHitResult(outHitResult, rayCastBuffer.hasBlock ? rayCastBuffer.block : rayCastBuffer.getTouch(0), rayCastBuffer.hasBlock);
	}

	return bFoundHit;
}


bool nsPhysX::SceneQueryRayCastMany(physx::PxScene* scene, nsPhysicsHitResultMany& outHitResultMany, const PxVec3& origin, const PxVec3& direction, float distance, const nsPhysicsQueryParams& params)
{
	NS_ValidateV(0, "Not implemented yet!");
	return false;
}


bool nsPhysX::SceneQuerySweep(physx::PxScene* scene, nsPhysicsHitResult& outHitResult, const PxGeometry& geometry, const PxTransform& transform, const PxVec3& direction, float distance, const nsPhysicsQueryParams& params)
{
	NS_Assert(scene);

	const PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;

	PxQueryFilterData queryFilterData{};
	queryFilterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
	queryFilterData.data.word1 = static_cast<PxU32>(params.Channel);

	nsPhysX_QueryFilterCallback queryFilterCallback;
	queryFilterCallback.IgnoredActors = params.IgnoredActors;

	PxSweepBufferN<1> sweepBuffer;
	const bool bFoundHit = scene->sweep(geometry, transform, direction, distance, sweepBuffer, hitFlags, queryFilterData, &queryFilterCallback);

	if (bFoundHit)
	{
		outHitResult = {};
		ns_FillSweepHitResult(outHitResult, sweepBuffer.hasBlock ? sweepBuffer.block : sweepBuffer.getTouch(0), sweepBuffer.hasBlock);
	}
	
	return bFoundHit;
}


bool nsPhysX::SceneQuerySweepMany(physx::PxScene* scene, nsPhysicsHitResultMany& outHitResultMany, const PxGeometry& geometry, const PxTransform& transform, const PxVec3& direction, float distance, const nsPhysicsQueryParams& params)
{
	NS_Assert(scene);

	const PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;

	PxQueryFilterData queryFilterData{};
	queryFilterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
	queryFilterData.data.word1 = static_cast<PxU32>(params.Channel);

	nsPhysX_QueryFilterCallback queryFilterCallback;
	queryFilterCallback.IgnoredActors = params.IgnoredActors;

	PxSweepBufferN<NS_ENGINE_PHYSICS_MAX_HIT_RESULT> sweepBuffer;
	scene->sweep(geometry, transform, direction, distance, sweepBuffer, hitFlags, queryFilterData, &queryFilterCallback);
	outHitResultMany.Clear();

	if (sweepBuffer.hasBlock)
	{
		ns_FillSweepHitResult(outHitResultMany.Add(), sweepBuffer.block, true);
	}

	for (PxU32 i = 0; i < sweepBuffer.getNbTouches(); ++i)
	{
		ns_FillSweepHitResult(outHitResultMany.Add(), sweepBuffer.getTouch(i), false);
	}

	return outHitResultMany.GetCount() > 0;
}



static nsPhysX_AllocatorCallback PhysXAllocatorCallback;
static nsPhysX_ErrorCallback PhysXErrorCallback;
static PxFoundation* Foundation;
static PxDefaultCpuDispatcher* CpuDispatcher;
static PxPvd* PVD;



nsPhysicsManager::nsPhysicsManager() noexcept
{
	bInitialized = false;
	Foundation = nullptr;
	CpuDispatcher = nullptr;
	Physics = nullptr;
	DefaultMaterial = nullptr;

#ifndef __NS_ENGINE_SHIPPING__
	Cooking = nullptr;
#endif // !__NS_ENGINE_SHIPPING__

	SceneNames.Reserve(8);
	SceneObjects.Reserve(8);

	bGlobalSimulate = false;
}


void nsPhysicsManager::Initialize()
{
	if (bInitialized)
	{
		return;
	}

	NS_CONSOLE_Log(PhysXLog, "Initialize physics manager [PhysX]");

	Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, PhysXAllocatorCallback, PhysXErrorCallback);

#if 0
	PVD = PxCreatePvd(*Foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	PVD->connect(*transport, PxPvdInstrumentationFlag::eALL);
#endif // 0

	CpuDispatcher = PxDefaultCpuDispatcherCreate(2);

	PxTolerancesScale toleranceScale;
	toleranceScale.length = 100.0f;
	toleranceScale.speed = 100.0f;

	Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, toleranceScale, true, PVD);

	DefaultMaterial = Physics->createMaterial(0.5f, 0.5f, 0.5f);

#ifndef __NS_ENGINE_SHIPPING__
	Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *Foundation, PxCookingParams(toleranceScale));
#endif // !__NS_ENGINE_SHIPPING__

	bInitialized = true;
}


void nsPhysicsManager::Simulate(float fixedTimeSteps)
{
	const float timeSteps = bGlobalSimulate ? fixedTimeSteps : NS_MATH_EPS;

	for (int i = 0; i < SceneObjects.GetCount(); ++i)
	{
		PxScene* scene = SceneObjects[i];
		scene->simulate(timeSteps);
		scene->fetchResults(true);
	}
}


physx::PxScene* nsPhysicsManager::CreateScene(nsName name)
{
	if (SceneNames.Find(name) != NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Debug(PhysXLog, "Fail to create scene. Scene with name [%s] already exists!", *name);
		return nullptr;
	}

	SceneNames.Add(name);

	PxSceneDesc sceneDesc(Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -980.0f, 0.0f);
	sceneDesc.filterShader = ns_CollisionFilterShader;
	sceneDesc.cpuDispatcher = CpuDispatcher;
	sceneDesc.flags = PxSceneFlag::eENABLE_ACTIVE_ACTORS | PxSceneFlag::eEXCLUDE_KINEMATICS_FROM_ACTIVE_ACTORS;
	sceneDesc.userData = nullptr;
	
	PxScene* scene = Physics->createScene(sceneDesc);
	
#ifdef __NS_ENGINE_DEBUG_DRAW__
	scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
#endif // __NS_ENGINE_DEBUG_DRAW__

	PxPvdSceneClient* pvdClient = scene->getScenePvdClient();

	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	SceneObjects.Add(scene);

	return scene;
}


void nsPhysicsManager::DestroyScene(physx::PxScene*& scene)
{
	NS_ValidateV(0, "Not implemented yet!");
}


bool nsPhysicsManager::SceneQueryRayCast(physx::PxScene* scene, nsPhysicsHitResult& hitResult, const nsVector3& origin, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	return nsPhysX::SceneQueryRayCast(scene, hitResult, NS_ToPxVec3(origin), NS_ToPxVec3(direction), distance, params);
}


bool nsPhysicsManager::SceneQuerySweepBox(physx::PxScene* scene, nsPhysicsHitResult& hitResult, const nsVector3& halfExtent, const nsTransform& worldTransform, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	return nsPhysX::SceneQuerySweep(scene, hitResult, PxBoxGeometry(NS_ToPxVec3(halfExtent)), NS_ToPxTransform(worldTransform), NS_ToPxVec3(direction), distance, params);
}


bool nsPhysicsManager::SceneQuerySweepSphere(physx::PxScene* scene, nsPhysicsHitResult& hitResult, float sphereRadius, const nsTransform& worldTransform, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	return nsPhysX::SceneQuerySweep(scene, hitResult, PxSphereGeometry(sphereRadius), NS_ToPxTransform(worldTransform), NS_ToPxVec3(direction), distance, params);
}


bool nsPhysicsManager::SceneQueryMousePicking(physx::PxScene* scene, nsPhysicsHitResult& outHitResult, const nsVector2& mousePosition, nsViewport* viewport)
{
	NS_Assert(viewport);

	bool bFoundHit = false;
	nsVector3 origin, direction;

	if (viewport->ProjectToWorld(mousePosition, origin, direction))
	{
		float nearClip, farClip;
		viewport->GetClip(nearClip, farClip);

		PxRaycastBuffer rayCastBuffer;
		bFoundHit = scene->raycast(NS_ToPxVec3(origin), NS_ToPxVec3(direction), farClip, rayCastBuffer, PxHitFlags(0));

		if (bFoundHit)
		{
			outHitResult = {};
			ns_FillRayCastHitResult(outHitResult, rayCastBuffer.block, true);
		}
	}

	return bFoundHit;
}



#ifdef __NS_ENGINE_DEBUG_DRAW__

void nsPhysicsManager::DebugDraw(nsRenderContextWorld& renderContextWorld)
{
	for (int i = 0; i < SceneObjects.GetCount(); ++i)
	{
		PxScene* scene = SceneObjects[i];
		const PxRenderBuffer& renderBuffer = scene->getRenderBuffer();
		const PxDebugLine* debugLines = renderBuffer.getLines();
		const PxU32 lineCount = renderBuffer.getNbLines();

		for (PxU32 i = 0; i < lineCount; ++i)
		{
			const PxDebugLine& line = debugLines[i];
			renderContextWorld.AddPrimitiveLine(NS_FromPxVec3(line.pos0), NS_FromPxVec3(line.pos1), nsColor(line.color0));
		}
	}
}

#endif // __NS_ENGINE_DEBUG_DRAW__
