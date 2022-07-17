#include "nsWorld.h"
#include "nsConsole.h"
#include "nsPhysicsManager.h"



static nsLogCategory WorldLog(TEXT("nsWorldLog"), nsELogVerbosity::LV_DEBUG);



NS_CLASS_BEGIN(nsWorld, nsObject)
NS_CLASS_END(nsWorld)

nsWorld::nsWorld(nsString name, bool bInitPhysics)
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
	PrePhysicsTickUpdateActors.Reserve(64);
	PhysicsTickUpdateActors.Reserve(64);
	PostPhysicsTickUpdateActors.Reserve(64);
	PendingDestroyActors.Reserve(64);
}


void nsWorld::Initialize()
{
	if (bHasPhysics)
	{
		PhysicsScene = nsPhysicsManager::Get().CreateScene(nsName::Format("%s.physics_ctx", *Name));
	}

	CreateLevel("Persistent");
}


void nsWorld::Destroy()
{
	NS_ValidateV(0, TEXT("Not implemented yet!"));
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

		NS_CONSOLE_Debug(WorldLog, TEXT("Destroy actor [%s]"), *actor->Name);
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

	NS_CONSOLE_Log(WorldLog, TEXT("%s: Start play!"), *Name);

	nsPhysicsManager::Get().bGlobalSimulate = true;

	for (int i = 0; i < StartStopPlayActors.GetCount(); ++i)
	{
		StartStopPlayActors[i]->StartPlay();
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

	NS_CONSOLE_Log(WorldLog, TEXT("%s: Stop play!"), *Name);

	nsPhysicsManager::Get().bGlobalSimulate = false;

	for (int i = 0; i < StartStopPlayActors.GetCount(); ++i)
	{
		StartStopPlayActors[i]->StopPlay();
	}

	StartTimeSeconds = 0.0f;
	bHasStartedPlay = false;
}


void nsWorld::DispatchPrePhysicsTickUpdate(float deltaTime)
{
	if (!bHasStartedPlay)
	{
		return;
	}

	DeltaTimeSeconds = deltaTime;

	for (int i = 0; i < PrePhysicsTickUpdateActors.GetCount(); ++i)
	{
		PrePhysicsTickUpdateActors[i]->TickUpdate(deltaTime);
	}
}


void nsWorld::DispatchPhysicsTickUpdate(float deltaTime)
{
	if (!bHasStartedPlay)
	{
		return;
	}

	for (int i = 0; i < PhysicsTickUpdateActors.GetCount(); ++i)
	{
		PhysicsTickUpdateActors[i]->PhysicsTickUpdate(deltaTime);
	}
}


void nsWorld::DispatchPostPhysicsTickUpdate()
{
	if (!bHasStartedPlay)
	{
		return;
	}

	for (int i = 0; i < PostPhysicsTickUpdateActors.GetCount(); ++i)
	{
		PostPhysicsTickUpdateActors[i]->PostPhysicsTickUpdate();
	}
}


void nsWorld::SyncActorTransformsWithPhysics()
{
	if (PhysicsScene == nullptr)
	{
		return;
	}

	nsPhysicsManager::Get().SceneSyncTransforms(PhysicsScene);
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
	PrePhysicsTickUpdateActors.Clear();
	PhysicsTickUpdateActors.Clear();
	PostPhysicsTickUpdateActors.Clear();

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
		if (flags & nsEActorFlag::CallPrePhysicsTickUpdate) PrePhysicsTickUpdateActors.Add(actor);
		if (flags & nsEActorFlag::CallPhysicsTickUpdate) PhysicsTickUpdateActors.Add(actor);
		if (flags & nsEActorFlag::CallPostPhysicsTickUpdate) PostPhysicsTickUpdateActors.Add(actor);
	}
}


nsLevel* nsWorld::FindLevel(const nsString& levelName) const
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


nsLevel* nsWorld::CreateLevel(nsString levelName)
{
	if (levelName.GetLength() == 0)
	{
		NS_CONSOLE_Warning(WorldLog, TEXT("Fail to add level. [levelName] is empty!"));
		return nullptr;
	}

	NS_Validate_IsMainThread();

	if (FindLevel(levelName))
	{
		NS_CONSOLE_Warning(WorldLog, TEXT("Fail to add level. Level with name [%s] already exists!"), *levelName);
		return nullptr;
	}

	NS_CONSOLE_Log(WorldLog, TEXT("Create new level [%s]"), *levelName);

	nsLevel* newLevel = ns_CreateObject<nsLevel>(levelName);
	newLevel->World = this;
	Levels.Add(newLevel);

	return newLevel;
}


void nsWorld::DestroyLevel(nsString levelName)
{
	NS_Validate_IsMainThread();

	nsLevel* level = FindLevel(levelName);

	if (level == nullptr)
	{
		NS_CONSOLE_Warning(WorldLog, TEXT("Fail to remove level. Level with name [%s] not found!"), *levelName);
		return;
	}

	NS_CONSOLE_Log(WorldLog, TEXT("Destroy level [%s]"), *levelName);

	level->Destroy();
	Levels.Remove(level);
	RefreshActorList();
}


void nsWorld::InitActor(nsActor* actor, nsString name, bool bIsStatic, const nsTransform& optTransform, nsActor* optParent)
{
	NS_Validate_IsMainThread();

	NS_Assert(actor);

	ActorList.Add(actor);

	actor->Name = name;

	uint32& flags = actor->Flags;
	if (flags & nsEActorFlag::CallStartStopPlay) StartStopPlayActors.Add(actor);
	if (flags & nsEActorFlag::CallPrePhysicsTickUpdate) PrePhysicsTickUpdateActors.Add(actor);
	if (flags & nsEActorFlag::CallPhysicsTickUpdate) PhysicsTickUpdateActors.Add(actor);
	if (flags & nsEActorFlag::CallPostPhysicsTickUpdate) PostPhysicsTickUpdateActors.Add(actor);

	if (bIsStatic)
	{
		flags |= nsEActorFlag::Static;
	}

	actor->SetWorldTransform(optTransform);
	actor->Initialize();
}


void nsWorld::DestroyActor(nsActor*& actor)
{
	if (actor == nullptr)
	{
		return;
	}

	NS_Validate_IsMainThread();

	if (actor->Flags & nsEActorFlag::PendingDestroy)
	{
		NS_CONSOLE_Warning(WorldLog, TEXT("Ignoring destroy actor [%s] that has marked as pending destroy!"), *actor->Name);
		return;
	}

	nsLevel* level = actor->Level;
	NS_Assert(level);

	nsWorld* world = level->World;
	NS_Assert(world);
	NS_AssertV(world == this, TEXT("Cannot destroy actor from different world!"));

	for (int i = 0; i < actor->Children.GetCount(); ++i)
	{
		DestroyActor(actor->Children[i]);
	}

	NS_CONSOLE_Debug(WorldLog, TEXT("Mark actor [%s] as pending destroy"), *actor->Name);

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

	NS_AssertV(actor->Level == nullptr, TEXT("Cannot add actor to level while inside another level. Call RemoveActorFromLevel() before add it to another level!"));
	nsLevel* useLevel = level ? level : GetPersistentLevel();
	actor->Level = useLevel;
	const bool bAddedToLevel = useLevel->AddActor(actor);

	if (bAddedToLevel && bHasStartedPlay && (actor->Flags & nsEActorFlag::CallStartStopPlay))
	{
		actor->StartPlay();
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
