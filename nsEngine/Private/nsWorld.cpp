#include "nsWorld.h"
#include "nsConsole.h"
#include "nsPhysics_PhysX.h"



static nsLogCategory WorldLog("nsWorldLog", nsELogVerbosity::LV_DEBUG);



NS_DEFINE_OBJECT(nsWorld, nsObject);

nsWorld::nsWorld(nsName name, bool bInitPhysics)
{
	NS_Assert(name.GetLength() > 0);

	Name = name;
	StartTimeSeconds = 0;
	DeltaTimeSeconds = 0.0f;
	bHasPhysics = bInitPhysics;
	bHasStartedPlay = false;

	PhysicsScene = nullptr;
	Levels.Reserve(8);

	ActorMemory.Initialize("world_actor", NS_MEMORY_SIZE_MiB(1));
	ActorList.Reserve(64);
	StartStopPlayActors.Reserve(64);
	TickUpdateActors.Reserve(64);

	bOnlyTickAfterStartedPlay = true;
}


void nsWorld::Initialize()
{
	if (bHasPhysics)
	{
		PhysicsScene = nsPhysicsManager::Get().CreateScene(nsName::Format("%s.physics_scene", *Name));
	}

	CreateLevel("Persistent");
}


void nsWorld::Destroy()
{
	NS_ValidateV(0, "Not implemented yet!");
}


void nsWorld::CleanupPendingDestroyLevelsAndActors()
{
	const int pendingDestroyCount = PendingDestroyActors.GetCount();

	for (int i = 0; i < pendingDestroyCount; ++i)
	{
		nsActor* actor = PendingDestroyActors[i];
		NS_Assert(actor && (actor->Flags & nsEActorFlag::PendingDestroy));

		nsLevel* level = actor->Level;
		NS_Assert(level);
		level->RemoveActor(actor);

		NS_CONSOLE_Debug(WorldLog, "Destroy actor [%s]", *actor->Name);
		actor->OnDestroy();

		ActorMemory.DeallocateDestruct(actor);
	}

	PendingDestroyActors.Clear();

	if (pendingDestroyCount > 0)
	{
		RefreshActorList();
	}
}


void nsWorld::DispatchStartPlay()
{
	if (bHasStartedPlay)
	{
		return;
	}

	NS_CONSOLE_Log(WorldLog, "Start play!");

	for (int i = 0; i < StartStopPlayActors.GetCount(); ++i)
	{
		StartStopPlayActors[i]->OnStartPlay();
	}

	StartTimeSeconds = static_cast<float>(nsPlatform::PerformanceQuery_Counter()) / static_cast<float>(nsPlatform::PerformanceQuery_Frequency());
	bHasStartedPlay = true;
}


void nsWorld::DispatchStopPlay()
{
	if (!bHasStartedPlay)
	{
		return;
	}

	NS_CONSOLE_Log(WorldLog, "Stop play!");

	for (int i = 0; i < StartStopPlayActors.GetCount(); ++i)
	{
		StartStopPlayActors[i]->OnStopPlay();
	}

	StartTimeSeconds = 0.0f;
	bHasStartedPlay = false;
}


void nsWorld::DispatchTickUpdate(float deltaTime)
{
	if (bOnlyTickAfterStartedPlay && !bHasStartedPlay)
	{
		return;
	}

	DeltaTimeSeconds = deltaTime;

	for (int i = 0; i < TickUpdateActors.GetCount(); ++i)
	{
		TickUpdateActors[i]->OnTickUpdate(deltaTime);
	}
}


void nsWorld::SyncActorTransformsWithPhysics()
{
	if (PhysicsScene == nullptr)
	{
		return;
	}

	PxU32 numActiveActors = 0;
	PxActor** activeActors = PhysicsScene->getActiveActors(numActiveActors);

	for (PxU32 i = 0; i < numActiveActors; ++i)
	{
		NS_Assert(activeActors[i]->is<PxRigidActor>());

		PxRigidActor* rigidActor = static_cast<PxRigidActor*>(activeActors[i]);

		nsTransformComponent* actorTransformComp = static_cast<nsTransformComponent*>(rigidActor->userData);
		const PxTransform globalPose = rigidActor->getGlobalPose();

		nsTransform newTransform;
		newTransform.Position = NS_FromPxVec3(globalPose.p);
		newTransform.Rotation = NS_FromPxQuat(globalPose.q);
		newTransform.Scale = actorTransformComp->GetWorldScale();

		actorTransformComp->SetWorldTransform(newTransform);
	}
}


bool nsWorld::PhysicsRayCast(nsPhysicsHitResult& hitResult, const nsVector3& origin, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	if (PhysicsScene == nullptr)
	{
		return false;
	}

	return nsPhysicsManager::Get().SceneQueryRayCast(PhysicsScene, hitResult, origin, direction, distance, params);
}


void nsWorld::RefreshActorList()
{
	ActorList.Clear();
	StartStopPlayActors.Clear();
	TickUpdateActors.Clear();

	for (int i = 0; i < Levels.GetCount(); ++i)
	{
		nsLevel* level = Levels[i];
		NS_Assert(level);

		ActorList.InsertAt(level->Actors);
	}

	for (int i = 0; i < ActorList.GetCount(); ++i)
	{
		nsActor* actor = ActorList[i];
		NS_Assert(actor);

		const uint16& flags = actor->Flags;
		if (flags & nsEActorFlag::CallStartStopPlay) StartStopPlayActors.Add(actor);
		if (flags & nsEActorFlag::CallTickUpdate) TickUpdateActors.Add(actor);
	}
}


nsLevel* nsWorld::FindLevel(const nsName& levelName) const
{
	if (levelName.GetLength() == 0)
	{
		return nullptr;
	}

	for (int i = 0; i < Levels.GetCount(); ++i)
	{
		if (Levels[i]->Name == levelName)
		{
			return Levels[i];
		}
	}

	return nullptr;
}


nsLevel* nsWorld::CreateLevel(nsName levelName)
{
	if (levelName.GetLength() == 0)
	{
		NS_CONSOLE_Warning(WorldLog, "Fail to add level. [levelName] is empty!");
		return nullptr;
	}

	if (FindLevel(levelName))
	{
		NS_CONSOLE_Warning(WorldLog, "Fail to add level. Level with name [%s] already exists!", *levelName);
		return nullptr;
	}

	NS_CONSOLE_Log(WorldLog, "Create new level [%s]", *levelName);

	nsLevel* newLevel = ns_CreateObject<nsLevel>(levelName);
	newLevel->World = this;
	Levels.Add(newLevel);

	return newLevel;
}


void nsWorld::DestroyLevel(nsName levelName)
{
	nsLevel* level = FindLevel(levelName);

	if (level == nullptr)
	{
		NS_CONSOLE_Warning(WorldLog, "Fail to remove level. Level with name [%s] not found!", *levelName);
		return;
	}

	NS_CONSOLE_Log(WorldLog, "Destroy level [%s]", *levelName);

	level->Destroy();
	Levels.Remove(level);
	RefreshActorList();
}


void nsWorld::InitActor(nsActor* actor, nsName name, bool bIsStatic, const nsTransform& optTransform, nsActor* optParent)
{
	NS_Validate_IsMainThread();

	NS_Assert(actor);

	ActorList.Add(actor);

	actor->Name = name;

	uint32& flags = actor->Flags;
	if (flags & nsEActorFlag::CallStartStopPlay) StartStopPlayActors.Add(actor);
	if (flags & nsEActorFlag::CallTickUpdate) TickUpdateActors.Add(actor);

	if (bIsStatic)
	{
		flags |= nsEActorFlag::Static;
	}

	actor->SetWorldTransform(optTransform);
	actor->OnInitialize();

}


void nsWorld::DestroyActor(nsActor*& actor)
{
	if (actor == nullptr)
	{
		return;
	}

	if (actor->Flags & nsEActorFlag::PendingDestroy)
	{
		NS_CONSOLE_Warning(WorldLog, "Ignoring destroy actor [%s] that has marked as pending destroy!", *actor->Name);
		return;
	}

	nsLevel* level = actor->Level;
	NS_Assert(level);

	nsWorld* world = level->World;
	NS_Assert(world);
	NS_AssertV(world == this, "Cannot destroy actor from different world!");

	for (int i = 0; i < actor->Children.GetCount(); ++i)
	{
		DestroyActor(actor->Children[i]);
	}

	NS_CONSOLE_Debug(WorldLog, "Mark actor [%s] as pending destroy", *actor->Name);

	PendingDestroyActors.Add(actor);
	actor->Flags |= nsEActorFlag::PendingDestroy;
	actor = nullptr;
}


void nsWorld::AddActorToLevel(nsActor* actor, nsLevel* level)
{
	if (actor == nullptr)
	{
		return;
	}

	NS_AssertV(actor->Level == nullptr, "Cannot add actor to level while inside another level. Call RemoveActorFromLevel() before add it to another level!");
	nsLevel* useLevel = level ? level : GetPersistentLevel();
	actor->Level = useLevel;

	if (useLevel->AddActor(actor) && bHasStartedPlay && (actor->Flags & nsEActorFlag::CallStartStopPlay))
	{
		actor->OnStartPlay();
	}
}


void nsWorld::RemoveActorFromLevel(nsActor* actor)
{
	if (actor == nullptr)
	{
		return;
	}

	nsLevel* level = actor->Level;
	NS_Assert(level);

	level->RemoveActor(actor);
}
