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




nsPhysicsManager_PhysX::nsPhysicsManager_PhysX()
{
	bInitialized = false;
	Foundation = nullptr;
	DefaultCpuDispatcher = nullptr;
	Physics = nullptr;

	SceneNames.Reserve(8);
	SceneSettings.Reserve(8);
	SceneObjects.Reserve(8);

	ObjectNames.Reserve(128);
	ObjectSettings.Reserve(128);
}


void nsPhysicsManager_PhysX::Initialize()
{
	if (bInitialized)
	{
		return;
	}

	NS_CONSOLE_Log(PhysXLog, "Initialize physics manager [PhysX]");

	Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, AllocatorCallback, ErrorCallback);
	DefaultCpuDispatcher = PxDefaultCpuDispatcherCreate(2);

	PxTolerancesScale toleranceScale;
	toleranceScale.length = 100.0f;
	toleranceScale.speed = 100.0f;

	Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, toleranceScale, true);
	
	bInitialized = true;
}


void nsPhysicsManager_PhysX::Update(float deltaTime)
{
	const float fixedDeltaTime = 0.016667f;

	for (auto it = SceneObjects.CreateIterator(); it; ++it)
	{
		PxScene* scene = (*it);
		scene->simulate(fixedDeltaTime);
		scene->fetchResults(true);
	}
}


nsPhysicsSceneID nsPhysicsManager_PhysX::CreatePhysicsScene(nsName name, const nsPhysicsSceneSettings& settings)
{
	const int nameId = SceneNames.Add();
	const int settingsId = SceneSettings.Add();
	const int objectId = SceneObjects.Add();
	NS_Assert(nameId == settingsId && settingsId == objectId);

	SceneNames[nameId] = name;
	SceneSettings[settingsId] = settings;

	PxSceneDesc sceneDesc(Physics->getTolerancesScale());
	sceneDesc.gravity = NS_ToPxVec3(settings.Gravity);
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	sceneDesc.cpuDispatcher = DefaultCpuDispatcher;
	sceneDesc.flags = PxSceneFlag::eENABLE_ACTIVE_ACTORS | PxSceneFlag::eEXCLUDE_KINEMATICS_FROM_ACTIVE_ACTORS;
	sceneDesc.userData = nullptr;
	
	SceneObjects[objectId] = Physics->createScene(sceneDesc);

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
	return scene.IsValid() && SceneSettings.IsValid(scene.Id) && SceneObjects[scene.Id] != nullptr;
}


nsName nsPhysicsManager_PhysX::GetPhysicsSceneName(nsPhysicsSceneID scene) const
{
	NS_Assert(IsPhysicsSceneValid(scene));

	return SceneNames[scene.Id];
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


nsPhysicsObjectID nsPhysicsManager_PhysX::CreatePhysicsObjectRigidBody_Box(nsName name, nsActor* actor, const nsVector3& halfExtent, nsPhysicsCollisionLayers collisionLayers, bool bIsStatic)
{
	return nsPhysicsObjectID();
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


void nsPhysicsManager_PhysX::SetPhysicsObjectWorldTransform(nsPhysicsObjectID physicsObject, const nsVector3& worldPosition, const nsQuaternion& worldRotation)
{
}


bool nsPhysicsManager_PhysX::IsPhysicsObjectValid(nsPhysicsObjectID physicsObject) const
{
	return false;
}


nsName nsPhysicsManager_PhysX::GetPhysicsObjectName(nsPhysicsObjectID physicsObject) const
{
	return nsName();
}
