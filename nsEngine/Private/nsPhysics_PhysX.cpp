#include "nsPhysics_PhysX.h"
#include "nsConsole.h"
#include "nsActor.h"



static nsLogCategory PhysXLog("nsPhysXLog", nsELogVerbosity::LV_DEBUG);



nsPhysX_AllocatorCallback::nsPhysX_AllocatorCallback()
{
	Memory.Initialize("physx_allocator", NS_MEMORY_SIZE_MiB(16));
}


void* nsPhysX_AllocatorCallback::allocate(size_t size, const char*, const char*, int)
{
	CriticalSection.Enter();
	void* data = Memory.Allocate(static_cast<int>(size));
	CriticalSection.Leave();

	return data;
}


void nsPhysX_AllocatorCallback::deallocate(void* ptr)
{
	CriticalSection.Enter();
	Memory.Deallocate(ptr);
	CriticalSection.Leave();
}




void nsPhysX_ErrorCallback::reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
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



#define NS_ToPxVec3(nsVec3) PxVec3(nsVec3.X, nsVec3.Y, nsVec3.Z)
#define NS_FromPxVec3(pxVec3) nsVector3(pxVec3.x, pxVec3.y, pxVec3.z)
#define NS_ToPxQuat(nsQuat) PxQuat(nsQuat.X, nsQuat.Y, nsQuat.Z, nsQuat.W)
#define NS_FromPxQuat(pxQuat) nsQuaternion(pxQuat.x, pxQuat.y, pxQuat.z, pxQuat.w)
#define NS_ToPxTransform(nsTrans) PxTransform(NS_ToPxVec3(nsTrans.Position), NS_ToPxQuat(nsTrans.Rotation))
#define NS_FromPxTransform(pxTrans) nsTransform(NS_FromPxVec3(pxTrans.p), NS_FromPxQuat(pxTrans.q))




namespace nsPxHelper
{
	NS_INLINE PxFilterFlags PxDefaultSimulationFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
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

	Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, AllocatorCallback, ErrorCallback);
	CpuDispatcher = PxDefaultCpuDispatcherCreate(2);

	PxTolerancesScale toleranceScale;
	toleranceScale.length = 100.0f;
	toleranceScale.speed = 100.0f;

	Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, toleranceScale, true);

	DefaultMaterial = Physics->createMaterial(0.5f, 0.5f, 0.5f);
	
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
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	sceneDesc.cpuDispatcher = CpuDispatcher;
	sceneDesc.flags = PxSceneFlag::eENABLE_ACTIVE_ACTORS | PxSceneFlag::eEXCLUDE_KINEMATICS_FROM_ACTIVE_ACTORS;
	sceneDesc.userData = nullptr;
	
	PxScene* scene = Physics->createScene(sceneDesc);

#ifdef __NS_ENGINE_DEBUG_DRAW__
	scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
#endif // __NS_ENGINE_DEBUG_DRAW__

	SceneObjects[sceneId] = scene;

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


nsPhysicsObjectID nsPhysicsManager_PhysX::CreatePhysicsObject_Box(nsName name, const nsVector3& halfExtent, nsEPhysicsCollisionChannel::Type collisionChannel, bool bIsStatic, bool bIsTrigger, void* transformComponent)
{
	NS_Assert(transformComponent);
	nsTransformComponent* transform = static_cast<nsTransformComponent*>(transformComponent);

	const int id = AllocatePhysicsObject();

	ObjectNames[id] = name;

	PxRigidActor* rigidActor = nullptr;
	
	if (bIsStatic)
	{
		PxRigidStatic* rigidStatic = Physics->createRigidStatic(NS_ToPxTransform(transform->GetWorldTransform()));
		rigidActor = rigidStatic;
	}
	else
	{
		PxRigidDynamic* rigidDynamic = Physics->createRigidDynamic(NS_ToPxTransform(transform->GetWorldTransform()));
		PxRigidBodyExt::updateMassAndInertia(*rigidDynamic, 100.0f);
		rigidActor = rigidDynamic;
	}

	PxShape* shape = PxRigidActorExt::createExclusiveShape(*rigidActor, PxBoxGeometry(NS_ToPxVec3(halfExtent)), *DefaultMaterial);

	if (bIsTrigger)
	{
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
	}

	NS_Assert(rigidActor);
	rigidActor->userData = transformComponent;

	ObjectRigidActors[id] = rigidActor;
	
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

	PxFilterData filterData = shape->getSimulationFilterData();
	filterData.word0 = static_cast<PxU32>(objectChannel);

	shape->setSimulationFilterData(filterData);
}


void nsPhysicsManager_PhysX::SetPhysicsObjectCollisionChannels(nsPhysicsObjectID physicsObject, nsPhysicsCollisionChannels collisionChannels)
{
	NS_Assert(IsPhysicsObjectValid(physicsObject));

	PxShape* shape = nsPxHelper::GetActorShape(ObjectRigidActors[physicsObject.Id]);

	PxFilterData filterData = shape->getSimulationFilterData();
	filterData.word1 = static_cast<PxU32>(collisionChannels);

	shape->setSimulationFilterData(filterData);
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
