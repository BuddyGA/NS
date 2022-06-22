#include "nsPhysics_PhysX.h"
#include "nsConsole.h"
#include "nsActor.h"



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



namespace nsPxHelper
{
	NS_INLINE PxFilterFlags DefaultSimulationFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		// let triggers through
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlag::eDEFAULT;
		}

		pairFlags = PxPairFlag::eCONTACT_DEFAULT;

		if ( (filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1) )
		{
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
		}
		
		return PxFilterFlag::eDEFAULT;
	}


	NS_NODISCARD_INLINE PxShape* GetActorShape(PxRigidActor* rigidActor)
	{
		NS_Assert(rigidActor);

		PxShape* shape = nullptr;
		const PxU32 shapeCount = rigidActor->getShapes(&shape, 1, 0);
		NS_Assert(shape);
		NS_Assert(shapeCount == 1);

		return shape;
	}

};




static nsPhysX_AllocatorCallback PhysXAllocatorCallback;
static nsPhysX_ErrorCallback PhysXErrorCallback;



nsPhysicsManager_PhysX::nsPhysicsManager_PhysX()
{
	bInitialized = false;
	Foundation = nullptr;
	CpuDispatcher = nullptr;
	Physics = nullptr;
	DefaultMaterial = nullptr;

	SceneNames.Reserve(8);
	SceneObjects.Reserve(8);

	ObjectNames.Reserve(128);
	ObjectRigidActors.Reserve(128);
}


void nsPhysicsManager_PhysX::Initialize()
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


void nsPhysicsManager_PhysX::Update(float fixedDeltaTime)
{
	for (auto it = SceneObjects.CreateIterator(); it; ++it)
	{
		PxScene* scene = (*it);
		scene->simulate(fixedDeltaTime);
		scene->fetchResults(true);
	}
}


nsPhysicsSceneID nsPhysicsManager_PhysX::CreatePhysicsScene(nsName name)
{
	const int nameId = SceneNames.Add();
	const int sceneId = SceneObjects.Add();
	NS_Assert(nameId == sceneId);

	SceneNames[nameId] = name;

	PxSceneDesc sceneDesc(Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -980.0f, 0.0f);
	sceneDesc.filterShader = nsPxHelper::DefaultSimulationFilterShader;
	sceneDesc.cpuDispatcher = CpuDispatcher;
	sceneDesc.flags = PxSceneFlag::eENABLE_ACTIVE_ACTORS | PxSceneFlag::eEXCLUDE_KINEMATICS_FROM_ACTIVE_ACTORS;
	sceneDesc.userData = nullptr;
	
	PxScene* scene = Physics->createScene(sceneDesc);
	SceneObjects[sceneId] = scene;

#ifdef __NS_ENGINE_DEBUG_DRAW__
	scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
#endif // __NS_ENGINE_DEBUG_DRAW__

	return nameId;
}


void nsPhysicsManager_PhysX::DestroyPhysicsScene(nsPhysicsSceneID& scene)
{
	if (!IsPhysicsSceneValid(scene))
	{
		scene = nsPhysicsSceneID::INVALID;
		return;
	}

	NS_ValidateV(0, "Not implemented yet!");
}


bool nsPhysicsManager_PhysX::IsPhysicsSceneValid(nsPhysicsSceneID scene) const
{
	return scene.IsValid() && SceneObjects[scene.Id] != nullptr;
}


void nsPhysicsManager_PhysX::SyncPhysicsSceneTransforms(nsPhysicsSceneID scene)
{
	NS_Assert(IsPhysicsSceneValid(scene));

	PxScene* pxScene = SceneObjects[scene.Id];
	PxU32 numActiveActors = 0;
	PxActor** pxActiveActors = pxScene->getActiveActors(numActiveActors);

	for (PxU32 i = 0; i < numActiveActors; ++i)
	{
		NS_Assert(pxActiveActors[i]->is<PxRigidActor>());

		PxRigidActor* pxRigidActor = static_cast<PxRigidActor*>(pxActiveActors[i]);

		nsActor* actor = static_cast<nsActor*>(pxRigidActor->userData);
		NS_Assert(actor);

		const PxTransform globalPose = pxRigidActor->getGlobalPose();

		nsTransform newTransform;
		newTransform.Position = NS_FromPxVec3(globalPose.p);
		newTransform.Rotation = NS_FromPxQuat(globalPose.q);
		newTransform.Scale = actor->GetWorldScale();

		actor->SetWorldTransform(newTransform);
	}
}


nsName nsPhysicsManager_PhysX::GetPhysicsSceneName(nsPhysicsSceneID scene) const
{
	NS_Assert(IsPhysicsSceneValid(scene));

	return SceneNames[scene.Id];
}



int nsPhysicsManager_PhysX::CreatePhysicsObject(nsName name, bool bIsStatic, nsTransformComponent* transformComponent)
{
	NS_Assert(transformComponent);

	const int id = AllocatePhysicsObject();

	ObjectNames[id] = name;

	PxRigidActor* rigidActor = nullptr;

	if (bIsStatic)
	{
		PxRigidStatic* rigidStatic = Physics->createRigidStatic(NS_ToPxTransform(transformComponent->GetWorldTransform()));
		rigidActor = rigidStatic;
	}
	else
	{
		PxRigidDynamic* rigidDynamic = Physics->createRigidDynamic(NS_ToPxTransform(transformComponent->GetWorldTransform()));
		PxRigidBodyExt::updateMassAndInertia(*rigidDynamic, 100.0f);
		rigidActor = rigidDynamic;
	}

	rigidActor->userData = transformComponent;

	ObjectRigidActors[id] = rigidActor;

	return id;
}


nsPhysicsObjectID nsPhysicsManager_PhysX::CreatePhysicsObject_Box(nsName name, const nsVector3& halfExtent, bool bIsStatic, bool bIsTrigger, nsTransformComponent* transformComponent)
{
	const int id = CreatePhysicsObject(name, bIsStatic, transformComponent);
	PxRigidActor* pxRigidActor = ObjectRigidActors[id];
	NS_Assert(pxRigidActor);

	PxShape* shape = PxRigidActorExt::createExclusiveShape(*pxRigidActor, PxBoxGeometry(NS_ToPxVec3(halfExtent)), *DefaultMaterial);

	if (bIsTrigger)
	{
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
	}
	
	return id;
}


nsPhysicsObjectID nsPhysicsManager_PhysX::CreatePhysicsObject_ConvexMesh(nsName name, const nsTArray<nsVertexMeshPosition>& vertices, bool bIsStatic, nsTransformComponent* transformComponent)
{
	NS_Assert(!vertices.IsEmpty());

	const int id = CreatePhysicsObject(name, bIsStatic, transformComponent);
	PxRigidActor* pxRigidActor = ObjectRigidActors[id];
	NS_Assert(pxRigidActor);

	PxConvexMeshDesc convexMeshDesc{};
	convexMeshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
	convexMeshDesc.points.stride = sizeof(nsVertexMeshPosition);
	convexMeshDesc.points.data = vertices.GetData();
	convexMeshDesc.points.count = static_cast<PxU32>(vertices.GetCount());
	convexMeshDesc.vertexLimit = 16;

	PxDefaultMemoryOutputStream buffer;
	PxConvexMeshCookingResult::Enum result;
	const bool bSuccess = Cooking->cookConvexMesh(convexMeshDesc, buffer, &result);
	NS_ValidateV(bSuccess, "Cooking convex mesh failed!");

	PxDefaultMemoryInputData input(buffer.getData(), buffer.getSize());
	PxConvexMesh* convexMesh = Physics->createConvexMesh(input);

	PxShape* shape = PxRigidActorExt::createExclusiveShape(*pxRigidActor, PxConvexMeshGeometry(convexMesh), *DefaultMaterial);
	convexMesh->release();

	return id;
}


void nsPhysicsManager_PhysX::DestroyPhysicsObject(nsPhysicsObjectID& physicsObject)
{
	if (!IsPhysicsObjectValid(physicsObject))
	{
		physicsObject = nsPhysicsObjectID::INVALID;
		return;
	}

	NS_ValidateV(0, "Not implemented yet!");
}


bool nsPhysicsManager_PhysX::IsPhysicsObjectValid(nsPhysicsObjectID physicsObject) const
{
	return physicsObject.IsValid() && ObjectRigidActors[physicsObject.Id] != nullptr;
}


void nsPhysicsManager_PhysX::UpdatePhysicsObjectShape_Box(nsPhysicsObjectID physicsObject, const nsVector3& halfExtent)
{
	NS_Assert(IsPhysicsObjectValid(physicsObject));

	PxShape* boxShape = nsPxHelper::GetActorShape(ObjectRigidActors[physicsObject.Id]);
	
	PxBoxGeometry boxGeometry;
	const bool bValid = boxShape->getBoxGeometry(boxGeometry);
	NS_Assert(bValid);

	boxGeometry.halfExtents = NS_ToPxVec3(halfExtent);
	boxShape->setGeometry(boxGeometry);
}


void nsPhysicsManager_PhysX::SetPhysicsObjectChannel(nsPhysicsObjectID physicsObject, nsEPhysicsCollisionChannel::Type objectChannel)
{
	NS_Assert(IsPhysicsObjectValid(physicsObject));

	PxShape* shape = nsPxHelper::GetActorShape(ObjectRigidActors[physicsObject.Id]);

	PxFilterData simulationFilterData = shape->getSimulationFilterData();
	simulationFilterData.word0 = static_cast<PxU32>(objectChannel);
	shape->setSimulationFilterData(simulationFilterData);

	PxFilterData queryFilterData = shape->getQueryFilterData();
	queryFilterData.word0 = static_cast<PxU32>(objectChannel);
	shape->setQueryFilterData(queryFilterData);
}


void nsPhysicsManager_PhysX::SetPhysicsObjectCollisionChannels(nsPhysicsObjectID physicsObject, nsPhysicsCollisionChannels collisionChannels)
{
	NS_Assert(IsPhysicsObjectValid(physicsObject));

	PxShape* shape = nsPxHelper::GetActorShape(ObjectRigidActors[physicsObject.Id]);

	PxFilterData simulationFilterData = shape->getSimulationFilterData();
	simulationFilterData.word1 = static_cast<PxU32>(collisionChannels);
	shape->setSimulationFilterData(simulationFilterData);

	PxFilterData queryFilterData = shape->getQueryFilterData();
	queryFilterData.word1 = static_cast<PxU32>(collisionChannels);
	shape->setQueryFilterData(queryFilterData);
}


void nsPhysicsManager_PhysX::SetPhysicsObjectWorldTransform(nsPhysicsObjectID physicsObject, const nsVector3& worldPosition, const nsQuaternion& worldRotation)
{
	NS_Assert(IsPhysicsObjectValid(physicsObject));

	PxRigidActor* rigidActor = ObjectRigidActors[physicsObject.Id];
	rigidActor->setGlobalPose(NS_ToPxTransform(nsTransform(worldPosition, worldRotation)));
}


nsName nsPhysicsManager_PhysX::GetPhysicsObjectName(nsPhysicsObjectID physicsObject) const
{
	NS_Assert(IsPhysicsObjectValid(physicsObject));

	return ObjectNames[physicsObject.Id];
}


void nsPhysicsManager_PhysX::AddPhysicsObjectToScene(nsPhysicsObjectID physicsObject, nsPhysicsSceneID scene)
{
	NS_Assert(IsPhysicsObjectValid(physicsObject));
	NS_Assert(IsPhysicsSceneValid(scene));

	PxScene* pxScene = SceneObjects[scene.Id];
	NS_Assert(pxScene);

	PxRigidActor* pxActor = ObjectRigidActors[physicsObject.Id];
	NS_Assert(pxActor);

	pxScene->addActor(*pxActor);
}


void nsPhysicsManager_PhysX::RemovePhysicsObjectFromScene(nsPhysicsObjectID physicsObject, nsPhysicsSceneID scene)
{
	NS_Assert(IsPhysicsObjectValid(physicsObject));
	NS_Assert(IsPhysicsSceneValid(scene));

	PxScene* pxScene = SceneObjects[scene.Id];
	NS_Assert(pxScene);

	PxRigidActor* pxActor = ObjectRigidActors[physicsObject.Id];
	NS_Assert(pxActor);

	pxScene->removeActor(*pxActor);
}


bool nsPhysicsManager_PhysX::SceneQueryRayCast(nsPhysicsSceneID scene, nsPhysicsHitResult& hitResult, const nsVector3& origin, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	NS_Assert(IsPhysicsSceneValid(scene));

	PxScene* pxScene = SceneObjects[scene.Id];
	NS_Assert(pxScene);

	PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;

	PxQueryFilterData queryFilterData{};
	queryFilterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
	queryFilterData.data.word1 = static_cast<PxU32>(params.Channel);

	nsPhysX_QueryFilterCallback queryFilterCallback; 
	queryFilterCallback.IgnoredActors = params.IgnoredActors;

	PxRaycastBufferN<1> buffer;
	bool bFoundHit = pxScene->raycast(NS_ToPxVec3(origin), NS_ToPxVec3(direction), distance, buffer, hitFlags, queryFilterData, &queryFilterCallback);

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
	}

	return bFoundHit;
}


bool nsPhysicsManager_PhysX::SceneQuerySweep(nsPhysicsSceneID scene, nsPhysicsHitResult& hitResult, const PxGeometry& geometry, const nsTransform& transform, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	NS_Assert(IsPhysicsSceneValid(scene));

	PxScene* pxScene = SceneObjects[scene.Id];
	NS_Assert(pxScene);

	PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;

	PxQueryFilterData queryFilterData{};
	queryFilterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
	queryFilterData.data.word1 = static_cast<PxU32>(params.Channel);

	nsPhysX_QueryFilterCallback queryFilterCallback;
	queryFilterCallback.IgnoredActors = params.IgnoredActors;

	PxSweepBufferN<1> buffer;

	bool bFoundHit = pxScene->sweep(geometry, NS_ToPxTransform(transform), NS_ToPxVec3(direction), distance, buffer, hitFlags, queryFilterData, &queryFilterCallback);

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
	}

	return false;
}




#ifdef __NS_ENGINE_DEBUG_DRAW__

void nsPhysicsManager_PhysX::DEBUG_Draw(nsRenderContextWorld& renderContextWorld)
{
	for (auto it = SceneObjects.CreateConstIterator(); it; ++it)
	{
		PxScene* scene = (*it);
		NS_Assert(scene);

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
