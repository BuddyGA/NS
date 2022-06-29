#include "nsActorComponents.h"
#include "nsWorld.h"
#include "nsConsole.h"



nsLogCategory nsComponentLog("nsComponentLog", nsELogVerbosity::LV_DEBUG);



// ================================================================================================================================== //
// ACTOR COMPONENT
// ================================================================================================================================== //
NS_CLASS_BEGIN(nsActorComponent, nsObject)
NS_CLASS_END(nsActorComponent)

nsActorComponent::nsActorComponent()
{
	Actor = nullptr;
	bAddedToLevel = false;
	bStartedPlay = false;
}


void nsActorComponent::OnDestroy()
{
	Name = "";
	Actor = nullptr;
	bAddedToLevel = false;
}


void nsActorComponent::OnAddedToLevel()
{
	bAddedToLevel = true;
}


void nsActorComponent::OnRemovedFromLevel()
{
	bAddedToLevel = false;
}


void nsActorComponent::OnStartPlay()
{
	bStartedPlay = true;
}


void nsActorComponent::OnStopPlay()
{
	bStartedPlay = false;
}


nsWorld* nsActorComponent::GetWorld() const
{
	NS_Assert(Actor);

	return Actor->GetWorld();
}




// ================================================================================================================================== //
// TRANSFORM COMPONENT
// ================================================================================================================================== //
NS_CLASS_BEGIN(nsTransformComponent, nsActorComponent)
NS_CLASS_END(nsTransformComponent)

nsTransformComponent::nsTransformComponent()
{
	Parent = nullptr;
	LocalTransform = nsTransform();
	WorldTransform = nsTransform();
	DirtyTransform = EDirtyTransform::NONE;
}


void nsTransformComponent::UpdateTransform()
{
	if (DirtyTransform == EDirtyTransform::NONE)
	{
		return;
	}

	if (DirtyTransform == EDirtyTransform::LOCAL)
	{
		LocalTransform.Position = Parent ? WorldTransform.Position - Parent->WorldTransform.Position : WorldTransform.Position;
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
	OnTransformChanged();

	// Propagate to children
	for (int i = 0; i < Children.GetCount(); ++i)
	{
		Children[i]->DirtyTransform = EDirtyTransform::WORLD;
		Children[i]->UpdateTransform();
	}
}


void nsTransformComponent::AttachToParent(nsTransformComponent* parent, nsETransformAttachmentMode attachmentMode, nsName socketName)
{
	if (parent == nullptr)
	{
		NS_CONSOLE_Warning(nsComponentLog, "Ignore attach transform component [%s] to parent. parent is NULL!", *Name);
		return;
	}

	if (this == parent)
	{
		NS_CONSOLE_Warning(nsComponentLog, "Fail attach actor [%s] to parent. Cannot attach to itself!", *Name);
		return;
	}

	if (this == parent->Parent)
	{
		parent->DetachFromParent();
	}

	DetachFromParent();
	Parent = parent;
	Parent->Children.Add(this);
	
	if (attachmentMode == nsETransformAttachmentMode::RESET_TRANSFORM)
	{
		LocalTransform = nsTransform();
		WorldTransform = Parent->WorldTransform;
	}
	else if (attachmentMode == nsETransformAttachmentMode::KEEP_LOCAL_TRANSFORM)
	{
		DirtyTransform = EDirtyTransform::WORLD;
	}
	else // nsETransformAttachmentMode::KEEP_WORLD_TRANSFORM
	{
		DirtyTransform = EDirtyTransform::LOCAL;
	}

	Parent->OnChildtAttached(this, attachmentMode, socketName);

	UpdateTransform();
}


void nsTransformComponent::DetachFromParent()
{
	if (Parent == nullptr)
	{
		return;
	}

	Parent->OnChildDetached(this);
	Parent->Children.Remove(this);
	Parent = nullptr;
	DirtyTransform = EDirtyTransform::LOCAL;
	UpdateTransform();
}
