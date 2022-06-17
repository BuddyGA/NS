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
	DirtyTransform = EDirtyTransform::NONE;
}


void nsActor::OnInitialize()
{
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


void nsActor::OnTransformUpdated()
{
	for (int i = 0; i < Components.GetCount(); ++i)
	{
		Components[i]->OnActorTransformUpdated();
	}
}


void nsActor::OnAddedToLevel()
{
	Flags |= nsEActorFlag::AddedToLevel;

	for (int i = 0; i < Components.GetCount(); ++i)
	{
		Components[i]->OnActorAddedToLevel();
	}
}


void nsActor::OnRemovedFromLevel()
{
	for (int i = 0; i < Components.GetCount(); ++i)
	{
		Components[i]->OnActorRemovedFromLevel();
	}

	Flags &= ~nsEActorFlag::AddedToLevel;
}


void nsActor::UpdateTransform()
{
	if (DirtyTransform == EDirtyTransform::NONE)
	{
		return;
	}

	if (DirtyTransform == EDirtyTransform::LOCAL)
	{
		LocalTransform.Position = Parent ? Parent->WorldTransform.Position - WorldTransform.Position : WorldTransform.Position;
		LocalTransform.Rotation = Parent ? LocalTransform.Rotation * Parent->WorldTransform.Rotation.GetConjugate() : WorldTransform.Rotation;
		LocalTransform.Scale = Parent ? WorldTransform.Scale / Parent->WorldTransform.Scale : WorldTransform.Scale;
	}
	else if (DirtyTransform == EDirtyTransform::WORLD)
	{
		WorldTransform.Position = Parent ? LocalTransform.Position + Parent->WorldTransform.Position : LocalTransform.Position;
		WorldTransform.Rotation = Parent ? LocalTransform.Rotation * Parent->WorldTransform.Rotation : LocalTransform.Rotation;
		WorldTransform.Scale = Parent ? LocalTransform.Scale * Parent->WorldTransform.Scale : LocalTransform.Scale;
	}

	DirtyTransform = EDirtyTransform::NONE;
	OnTransformUpdated();

	// Propagate to children
	for (int i = 0; i < Children.GetCount(); ++i)
	{
		Children[i]->DirtyTransform = EDirtyTransform::WORLD;
		Children[i]->UpdateTransform();
	}
}


nsWorld* nsActor::GetWorld() const
{
	NS_Assert(Level);
	return Level->GetWorld();
}


void nsActor::AttachToParent(nsActor* parent, nsEActorAttachmentMode attachmentMode)
{
	if (parent == nullptr)
	{
		NS_CONSOLE_Warning(ActorLog, "Ignore attach actor [%s] to parent. parent is NULL!", *Name);
		return;
	}

	if (this == parent)
	{
		NS_CONSOLE_Warning(ActorLog, "Fail attach actor [%s] to parent. Cannot attach to itself!", *Name);
		return;
	}

	const nsWorld* world = GetWorld();
	const nsWorld* parentWorld = parent->GetWorld();
	NS_Assert(world);
	NS_Assert(parentWorld);

	if (world != parentWorld)
	{
		NS_CONSOLE_Warning(ActorLog, "Fail attach actor [%s] to parent [%s]. Cannot attach actor to parent from different world! [ActorWorld: %s, ParentWorld: %s]", *Name, *parent->Name, *world->GetName(), *parentWorld->GetName());
		return;
	}

	if (this == parent->Parent)
	{
		parent->DetachFromParent();
	}

	DetachFromParent();
	Parent = parent;
	Parent->Children.Add(this);

	if (attachmentMode == nsEActorAttachmentMode::RESET_TRANSFORM)
	{
		LocalTransform = nsTransform();
		WorldTransform = Parent->WorldTransform;
	}
	else if (attachmentMode == nsEActorAttachmentMode::KEEP_LOCAL_TRANSFORM)
	{
		DirtyTransform = EDirtyTransform::WORLD;
	}
	else // ecEActorAttachmentMode::KEEP_WORLD_TRANSFORM
	{
		DirtyTransform = EDirtyTransform::LOCAL;
	}

	UpdateTransform();
}


void nsActor::DetachFromParent()
{
	if (Parent == nullptr)
	{
		return;
	}

	Parent->Children.Remove(this);
	Parent = nullptr;
	DirtyTransform = EDirtyTransform::LOCAL;
	UpdateTransform();
}


void nsActor::SetLocalTransform(nsTransform transform)
{
	LocalTransform = transform;
	DirtyTransform = EDirtyTransform::WORLD;
	UpdateTransform();
}


void nsActor::SetLocalPosition(nsVector3 position)
{
	if (DirtyTransform == EDirtyTransform::LOCAL)
	{
		UpdateTransform();
	}

	LocalTransform.Position = position;
	DirtyTransform = EDirtyTransform::WORLD;
}


void nsActor::SetLocalRotation(nsQuaternion rotation)
{
	if (DirtyTransform == EDirtyTransform::LOCAL)
	{
		UpdateTransform();
	}

	LocalTransform.Rotation = rotation;
	DirtyTransform = EDirtyTransform::WORLD;
}


void nsActor::SetLocalScale(nsVector3 scale)
{
	if (DirtyTransform == EDirtyTransform::LOCAL)
	{
		UpdateTransform();
	}

	LocalTransform.Scale = scale;
	DirtyTransform = EDirtyTransform::WORLD;
}


void nsActor::SetWorldTransform(nsTransform transform)
{
	WorldTransform = transform;
	DirtyTransform = EDirtyTransform::LOCAL;
	UpdateTransform();
}


void nsActor::SetWorldPosition(nsVector3 position)
{
	if (DirtyTransform == EDirtyTransform::WORLD)
	{
		UpdateTransform();
	}

	WorldTransform.Position = position;
	DirtyTransform = EDirtyTransform::LOCAL;
}


void nsActor::SetWorldRotation(nsQuaternion rotation)
{
	if (DirtyTransform == EDirtyTransform::WORLD)
	{
		UpdateTransform();
	}

	WorldTransform.Rotation = rotation;
	DirtyTransform = EDirtyTransform::LOCAL;
}


void nsActor::SetWorldScale(nsVector3 scale)
{
	if (DirtyTransform == EDirtyTransform::WORLD)
	{
		UpdateTransform();
	}

	WorldTransform.Scale = scale;
	DirtyTransform = EDirtyTransform::LOCAL;
}


nsTransform nsActor::GetLocalTransform()
{
	UpdateTransform();
	return LocalTransform;
}


nsVector3 nsActor::GetLocalPosition()
{
	UpdateTransform();
	return LocalTransform.Position;
}


nsQuaternion nsActor::GetLocalRotation()
{
	UpdateTransform();
	return LocalTransform.Rotation;
}


nsVector3 nsActor::GetLocalScale()
{
	UpdateTransform();
	return LocalTransform.Scale;
}


nsTransform nsActor::GetWorldTransform()
{
	UpdateTransform();
	return WorldTransform;
}


nsVector3 nsActor::GetWorldPosition()
{
	UpdateTransform();
	return WorldTransform.Position;
}


nsQuaternion nsActor::GetWorldRotation()
{
	UpdateTransform();
	return WorldTransform.Rotation;
}


nsVector3 nsActor::GetWorldScale()
{
	UpdateTransform();
	return WorldTransform.Scale;
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
