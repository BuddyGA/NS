#include "nsWorld.h"
#include "nsConsole.h"



static nsLogCategory WorldLog("nsWorldLog", nsELogVerbosity::LV_DEBUG);



nsWorld::nsWorld(nsName name, bool bHasPhysics)
{
	NS_Assert(name.GetLength() > 0);

	Name = name;
	StartTimeSeconds = 0;
	DeltaTimeSeconds = 0.0f;
	bHasStartedPlay = false;

	Levels.Reserve(8);

	ActorMemory.Initialize("world_actor", NS_MEMORY_SIZE_MiB(1));
	ActorList.Reserve(64);
	StartStopPlayActors.Reserve(64);
	TickUpdateActors.Reserve(64);

	bOnlyTickAfterStartedPlay = true;
}


void nsWorld::Initialize()
{
	CreateLevel(nsName::Format("%s.level_persistent", *Name));
}


void nsWorld::Destroy()
{
	NS_ValidateV(0, "Not implemented yet!");

}


void nsWorld::CleanupActors()
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


void nsWorld::DestroyActor(nsActor* actor)
{
	if (actor == nullptr)
	{
		return;
	}

	if (actor->Flags & nsEActorFlag::PendingDestroy)
	{
		NS_CONSOLE_Warning(WorldLog, "Ignoring destroy actor that has marked as pending destroy!");
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

	actor->Flags |= nsEActorFlag::PendingDestroy;
	PendingDestroyActors.Add(actor);
}