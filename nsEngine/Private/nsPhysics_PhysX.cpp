#include "nsPhysics_PhysX.h"
#include "nsConsole.h"
#include "nsActor.h"
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
	nsTArrayInline<nsActor*, 8> IgnoredActors;


public:
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
		nsActor* actorToCheck = static_cast<nsTransformComponent*>(actor->userData)->GetActor();

		if (IgnoredActors.Find(actorToCheck) != NS_ARRAY_INDEX_INVALID)
		{
			return PxQueryHitType::eNONE;
		}

		return PxQueryHitType::eBLOCK;
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



PxFilterFlags nsPhysXHelper::DefaultSimulationFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	// let triggers through
	if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlag::eDEFAULT;
	}

	pairFlags = PxPairFlag::eCONTACT_DEFAULT;

	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
	{
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
	}

	return PxFilterFlag::eDEFAULT;
}


bool nsPhysXHelper::SceneQuerySweep(physx::PxScene* scene, nsPhysicsHitResult& hitResult, const PxGeometry& geometry, const nsTransform& transform, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	NS_Assert(scene);

	PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;

	PxQueryFilterData queryFilterData{};
	queryFilterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
	queryFilterData.data.word1 = static_cast<PxU32>(params.Channel);

	nsPhysX_QueryFilterCallback queryFilterCallback;
	queryFilterCallback.IgnoredActors = params.IgnoredActors;

	PxSweepBufferN<1> buffer;

	bool bFoundHit = scene->sweep(geometry, NS_ToPxTransform(transform), NS_ToPxVec3(direction), distance, buffer, hitFlags, queryFilterData, &queryFilterCallback);

	if (bFoundHit)
	{
		const PxSweepHit sweepHit = buffer.block;
		hitResult.Component = static_cast<nsTransformComponent*>(sweepHit.actor->userData);
		hitResult.Actor = hitResult.Component->GetActor();

		if (sweepHit.flags & PxHitFlag::ePOSITION)
		{
			hitResult.WorldPosition = NS_FromPxVec3(sweepHit.position);
		}

		if (sweepHit.flags & PxHitFlag::eNORMAL)
		{
			hitResult.WorldNormal = NS_FromPxVec3(sweepHit.normal);
		}

		hitResult.Distance = sweepHit.distance;
	}

	return bFoundHit;
}


PxShape* nsPhysXHelper::GetActorShape(PxRigidActor* rigidActor)
{
	NS_Assert(rigidActor);

	PxShape* shape = nullptr;
	const PxU32 shapeCount = rigidActor->getShapes(&shape, 1, 0);
	NS_Assert(shape);
	NS_Assert(shapeCount == 1);

	return shape;
}



static nsPhysX_AllocatorCallback PhysXAllocatorCallback;
static nsPhysX_ErrorCallback PhysXErrorCallback;
static PxFoundation* Foundation;
static PxDefaultCpuDispatcher* CpuDispatcher;



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
	CpuDispatcher = PxDefaultCpuDispatcherCreate(2);

	PxTolerancesScale toleranceScale;
	toleranceScale.length = 100.0f;
	toleranceScale.speed = 100.0f;

	Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, toleranceScale, true);

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
	sceneDesc.filterShader = nsPhysXHelper::DefaultSimulationFilterShader;
	sceneDesc.cpuDispatcher = CpuDispatcher;
	sceneDesc.flags = PxSceneFlag::eENABLE_ACTIVE_ACTORS | PxSceneFlag::eEXCLUDE_KINEMATICS_FROM_ACTIVE_ACTORS;
	sceneDesc.userData = nullptr;
	
	PxScene* scene = Physics->createScene(sceneDesc);
	
#ifdef __NS_ENGINE_DEBUG_DRAW__
	scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
#endif // __NS_ENGINE_DEBUG_DRAW__

	SceneObjects.Add(scene);

	return scene;
}


void nsPhysicsManager::DestroyScene(physx::PxScene*& scene)
{
	NS_ValidateV(0, "Not implemented yet!");
}


bool nsPhysicsManager::SceneQueryRayCast(physx::PxScene* scene, nsPhysicsHitResult& hitResult, const nsVector3& origin, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	NS_Assert(scene);

	PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;

	PxQueryFilterData queryFilterData{};
	queryFilterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
	queryFilterData.data.word1 = static_cast<PxU32>(params.Channel);

	nsPhysX_QueryFilterCallback queryFilterCallback;
	queryFilterCallback.IgnoredActors = params.IgnoredActors;

	PxRaycastBufferN<1> buffer;
	bool bFoundHit = scene->raycast(NS_ToPxVec3(origin), NS_ToPxVec3(direction), distance, buffer, hitFlags, queryFilterData, &queryFilterCallback);

	if (bFoundHit)
	{
		const PxRaycastHit rayCastHit = buffer.block;

		hitResult.Component = static_cast<nsTransformComponent*>(rayCastHit.actor->userData);
		hitResult.Actor = hitResult.Component->GetActor();

		if (rayCastHit.flags & PxHitFlag::ePOSITION)
		{
			hitResult.WorldPosition = NS_FromPxVec3(rayCastHit.position);
		}

		if (rayCastHit.flags & PxHitFlag::eNORMAL)
		{
			hitResult.WorldNormal = NS_FromPxVec3(rayCastHit.normal);
		}

		hitResult.Distance = rayCastHit.distance;
	}

	return bFoundHit;
}


bool nsPhysicsManager::SceneQuerySweepBox(physx::PxScene* scene, nsPhysicsHitResult& hitResult, const nsVector3& halfExtent, const nsTransform& worldTransform, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	return nsPhysXHelper::SceneQuerySweep(scene, hitResult, PxBoxGeometry(NS_ToPxVec3(halfExtent)), worldTransform, direction, distance, params);
}


bool nsPhysicsManager::SceneQuerySweepSphere(physx::PxScene* scene, nsPhysicsHitResult& hitResult, float sphereRadius, const nsTransform& worldTransform, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	return nsPhysXHelper::SceneQuerySweep(scene, hitResult, PxSphereGeometry(sphereRadius), worldTransform, direction, distance, params);
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
