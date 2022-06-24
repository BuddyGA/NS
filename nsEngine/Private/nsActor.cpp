#include "nsActor.h"
#include "nsWorld.h"
#include "nsConsole.h"


#define NS_ACTOR_DEFAULT_ROOT_COMPONENT_NAME	"default_root_component"


static nsLogCategory ActorLog("ActorLog", nsELogVerbosity::LV_DEBUG);

nsMemory nsActor::ComponentMemory("actor_components", NS_MEMORY_SIZE_MiB(1));



NS_DEFINE_OBJECT(nsActor, nsObject);

nsActor::nsActor()
	: Level(nullptr)
	, Flags(nsEActorFlag::CallStartStopPlay)
	, Parent(nullptr)
	, RootComponent(nullptr)
{
	Components.Reserve(4);
	RootComponent = AddComponent<nsTransformComponent>(NS_ACTOR_DEFAULT_ROOT_COMPONENT_NAME);
}


void nsActor::OnInitialize()
{
	if (Flags & nsEActorFlag::Initialized)
	{
		return;
	}

	Flags |= nsEActorFlag::Initialized;
	//RootComponent = AddComponent<nsTransformComponent>("default_root_component");

	for (int i = 0; i < Components.GetCount(); ++i)
	{
		Components[i]->OnInitialize();
	}
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

	if (Flags & nsEActorFlag::StartedPlay)
	{
		for (int i = 0; i < Components.GetCount(); ++i)
		{
			Components[i]->OnStopPlay();
		}

		Flags &= ~nsEActorFlag::StartedPlay;
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

	Children.Clear();

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
	if (Flags & nsEActorFlag::AddedToLevel)
	{
		return;
	}

	Flags |= nsEActorFlag::AddedToLevel;

	for (int i = 0; i < Components.GetCount(); ++i)
	{
		Components[i]->OnAddedToLevel();
	}
}


void nsActor::OnRemovedFromLevel()
{
	if (Flags & nsEActorFlag::AddedToLevel)
	{
		for (int i = 0; i < Components.GetCount(); ++i)
		{
			Components[i]->OnRemovedFromLevel();
		}

		Flags &= ~nsEActorFlag::AddedToLevel;
	}
}


nsWorld* nsActor::GetWorld() const
{
	return Level ? Level->GetWorld() : nullptr;
}


void nsActor::SetAsStatic(bool bIsStatic)
{
	const bool bWasStatic = (Flags & nsEActorFlag::Static);

	if (bWasStatic == bIsStatic)
	{
		return;
	}

	if (bIsStatic)
	{
		Flags |= nsEActorFlag::Static;
	}
	else
	{
		Flags &= ~nsEActorFlag::Static;
	}

	for (int i = 0; i < Components.GetCount(); ++i)
	{
		Components[i]->OnStaticChanged();
	}
}


void nsActor::SetRootComponent(nsTransformComponent* newRootComponent)
{
	if (newRootComponent == nullptr || newRootComponent == RootComponent)
	{
		return;
	}

	const int newRootIndex = Components.Find(newRootComponent);

	if (newRootIndex == NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Warning(ActorLog, "Fail to set actor new root component. newRootComponent not found from component list! [actor: %s, newRootComponent: %s]", *Name, *newRootComponent->Name);
		return;
	}

	NS_Assert(Components[0] == RootComponent);
	nsTArrayInline<nsTransformComponent*, NS_ENGINE_TRANSFORM_MAX_CHILDREN> detachedFromRootComponent;

	for (int i = 1; i < Components.GetCount(); ++i)
	{
		nsTransformComponent* transformComponent = ns_Cast<nsTransformComponent>(Components[i]);

		if (transformComponent && transformComponent->GetParent() == RootComponent)
		{
			transformComponent->DetachFromParent();
			detachedFromRootComponent.Add(transformComponent);
		}
	}

	if (RootComponent->Name == NS_ACTOR_DEFAULT_ROOT_COMPONENT_NAME)
	{
		Components[0] = Components[newRootIndex];
		Components[newRootIndex] = RootComponent;
		RemoveComponent(RootComponent);
		RootComponent = nullptr;
	}

	NS_Assert(Components[0] == newRootComponent);
	RootComponent = newRootComponent;

	for (int i = 0; i < detachedFromRootComponent.GetCount(); ++i)
	{
		if (detachedFromRootComponent[i] == RootComponent)
		{
			continue;
		}

		detachedFromRootComponent[i]->AttachToParent(RootComponent, nsETransformAttachmentMode::KEEP_WORLD_TRANSFORM);
	}
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


bool nsActor::RemoveComponent(nsActorComponent* component)
{
	if (component == nullptr)
	{
		return false;
	}

	const int index = Components.Find(component);

	if (index == NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Warning(ActorLog, "Fail to remove component [%s] from actor [%s]. Actor does not own the component!", *component->Name, *Name);
		return false;
	}

	component->OnRemovedFromLevel();
	component->OnDestroy();
	Components.RemoveAt(index);
	ComponentMemory.Deallocate(component);

	return true;
}
