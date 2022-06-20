#include "nsActor.h"
#include "nsWorld.h"
#include "nsConsole.h"


static nsLogCategory ActorLog("ActorLog", nsELogVerbosity::LV_DEBUG);

nsMemory nsActor::ComponentMemory("actor_components", NS_MEMORY_SIZE_MiB(1));



nsActor::nsActor()
{
	Level = nullptr;
	Flags = 0;
	Parent = nullptr;
	RootComponent = nullptr;
}


void nsActor::OnInitialize()
{
	RootComponent = AddComponent<nsTransformComponent>("root_component");
}


void nsActor::OnStartPlay()
{
	NS_Assert(Flags & nsEActorFlag::CallStartStopPlay);

	if (Flags & nsEActorFlag::StartedPlay)
	{
		return;
	}

	Flags |= nsEActorFlag::StartedPlay;

	for (int i = 0; i < Components.GetCount(); ++i)
	{
		Components[i]->OnStartPlay();
	}
}


void nsActor::OnStopPlay()
{
	NS_Assert(Flags & nsEActorFlag::CallStartStopPlay);

	if (!(Flags & nsEActorFlag::StartedPlay))
	{
		return;
	}

	Flags &= ~nsEActorFlag::StartedPlay;

	for (int i = 0; i < Components.GetCount(); ++i)
	{
		Components[i]->OnStopPlay();
	}
}


void nsActor::OnTickUpdate(float deltaTime)
{
	NS_Assert(Flags & nsEActorFlag::CallTickUpdate);

	for (int i = 0; i < Components.GetCount(); ++i)
	{
		Components[i]->OnTickUpdate(deltaTime);
	}
}


void nsActor::OnDestroy()
{
	NS_Assert(Flags & nsEActorFlag::PendingDestroy);

	if (Parent && !(Parent->Flags & nsEActorFlag::PendingDestroy))
	{
		Parent->Children.Remove(this);
		Parent = nullptr;
	}

	for (int i = 0; i < Children.GetCount(); ++i)
	{
		Children[i]->OnDestroy();
	}

	Children.Clear(true);

	for (int i = 0; i < Components.GetCount(); ++i)
	{
		nsActorComponent* comp = Components[i];
		NS_Assert(comp);
		comp->OnDestroy();
		ComponentMemory.DeallocateDestruct(comp);
	}

	Components.Clear(true);
}


void nsActor::OnAddedToLevel()
{
	Flags |= nsEActorFlag::AddedToLevel;

	for (int i = 0; i < Components.GetCount(); ++i)
	{
		Components[i]->OnAddedToLevel();
	}
}


void nsActor::OnRemovedFromLevel()
{
	for (int i = 0; i < Components.GetCount(); ++i)
	{
		Components[i]->OnRemovedFromLevel();
	}

	Flags &= ~nsEActorFlag::AddedToLevel;
}


nsWorld* nsActor::GetWorld() const
{
	NS_Assert(Level);
	return Level->GetWorld();
}


void nsActor::AttachToParent(nsActor* parent, nsETransformAttachmentMode attachmentMode)
{
}


void nsActor::DetachFromParent()
{
}


nsActorComponent* nsActor::FindComponent(const nsName& name) const
{
	if (name.GetLength() == 0)
	{
		return nullptr;
	}

	for (int i = 0; i < Components.GetCount(); ++i)
	{
		if (Components[i]->Name == name)
		{
			return Components[i];
		}
	}

	return nullptr;
}
