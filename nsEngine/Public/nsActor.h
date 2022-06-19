#pragma once

#include "nsActorComponents.h"


class nsLevel;
class nsWorld;



enum class nsEActorAttachmentMode : uint8
{
	RESET_TRANSFORM = 0,
	KEEP_LOCAL_TRANSFORM,
	KEEP_WORLD_TRANSFORM
};



namespace nsEActorFlag
{
	enum
	{
		NONE					= (0),
		Persistent				= (1 << 0),
		CallStartStopPlay		= (1 << 1),
		CallTickUpdate			= (1 << 2),
		CallPhysicsTickUpdate	= (1 << 3),
		CallPostTickUpdate		= (1 << 4),
		AddedToLevel			= (1 << 5),
		StartedPlay				= (1 << 6),
		PendingDestroy			= (1 << 7),
	};
};

typedef uint32 nsActorFlags;



class NS_ENGINE_API nsActor
{
public:
	nsName Name;

protected:
	nsLevel* Level;
	nsActorFlags Flags;

private:
	enum class EDirtyTransform : uint8
	{
		NONE = 0,
		LOCAL,
		WORLD
	};

	nsTransform LocalTransform;
	nsTransform WorldTransform;
	nsActor* Parent;
	nsTArray<nsActorComponent*> Components;
	nsTArray<nsActor*> Children;
	EDirtyTransform DirtyTransform;


	static nsMemory ComponentMemory;


public:
	nsActor();

protected:
	void OnInitialize();
	void OnStartPlay();
	void OnStopPlay();
	void OnTickUpdate(float deltaTime);
	void OnDestroy();
	void OnTransformUpdated();

public:
	void OnAddedToLevel();
	void OnRemovedFromLevel();

private:
	void UpdateTransform();

public:
	NS_NODISCARD nsWorld* GetWorld() const;
	void AttachToParent(nsActor* parent, nsEActorAttachmentMode attachmentMode);
	void DetachFromParent();
	void SetLocalTransform(nsTransform transform);
	void SetLocalPosition(nsVector3 position);
	void SetLocalRotation(nsQuaternion rotation);
	void SetLocalScale(nsVector3 scale);
	void SetWorldTransform(nsTransform transform);
	void SetWorldPosition(nsVector3 position);
	void SetWorldRotation(nsQuaternion rotation);
	void SetWorldScale(nsVector3 scale);

	NS_NODISCARD nsTransform GetLocalTransform();
	NS_NODISCARD nsVector3 GetLocalPosition();
	NS_NODISCARD nsQuaternion GetLocalRotation();
	NS_NODISCARD nsVector3 GetLocalScale();
	NS_NODISCARD nsTransform GetWorldTransform();
	NS_NODISCARD nsVector3 GetWorldPosition();
	NS_NODISCARD nsQuaternion GetWorldRotation();
	NS_NODISCARD nsVector3 GetWorldScale();


	NS_INLINE void AddLocalPosition(nsVector3 delta)
	{
		SetLocalPosition(delta + GetLocalPosition());
	}


	NS_INLINE void AddLocalRotation(nsQuaternion delta)
	{
		SetLocalRotation(delta * GetLocalRotation());
	}


	NS_INLINE void AddLocalScale(nsVector3 delta)
	{
		SetLocalScale(delta + GetLocalScale());
	}


	NS_INLINE void AddWorldPosition(nsVector3 delta)
	{
		SetWorldPosition(delta + GetWorldPosition());
	}


	NS_INLINE void AddWorldRotation(nsQuaternion delta)
	{
		SetWorldRotation(delta * GetWorldRotation());
	}


	NS_INLINE void AddWorldScale(nsVector3 delta)
	{
		SetWorldScale(delta + GetWorldScale());
	}


	NS_NODISCARD_INLINE bool IsPendingDestroy() const noexcept
	{
		return (Flags & nsEActorFlag::PendingDestroy);
	}



	nsActorComponent* FindComponent(const nsName& name) const;


	template<typename TComponent = nsActorComponent>
	NS_INLINE TComponent* AddComponent(nsName name)
	{
		static_assert(std::is_base_of<nsActorComponent, TComponent>::value, "AddComponent() type of <TComponent> must be derived from type <nsActorComponent>!");

		nsActorComponent* checkComponent = FindComponent(name);
		NS_ValidateV(checkComponent == nullptr, "Component with name [%s] already exists!", *name);

		TComponent* newComponent = ComponentMemory.AllocateConstruct<TComponent>();
		newComponent->Name = name;
		newComponent->Actor = this;
		newComponent->OnInitialize();

		if ((Flags & nsEActorFlag::CallStartStopPlay) && (Flags & nsEActorFlag::StartedPlay))
		{
			newComponent->OnStartPlay();
		}

		if (Flags & nsEActorFlag::AddedToLevel)
		{
			newComponent->OnActorAddedToLevel();
		}

		Components.Add(newComponent);

		return newComponent;
	}


	template<typename TComponent = nsActorComponent>
	NS_INLINE bool RemoveComponent()
	{
		static_assert(std::is_base_of<nsActorComponent, TComponent>::value, "RemoveComponent() type of <TComponent> must be derived from type <nsActorComponent>!");

		for (int i = 0; i < Components.GetCount(); ++i)
		{
			TComponent* check = dynamic_cast<TComponent*>(Components[i]);

			if (check)
			{
				check->OnDestroy();
				Components.RemoveAt(i);
				ComponentMemory.DeallocateDestruct<TComponent>(check);

				return true;
			}
		}

		return false;
	}


	template<typename TComponent = nsActorComponent>
	NS_INLINE TComponent* GetComponent() const
	{
		static_assert(std::is_base_of<nsActorComponent, TComponent>::value, "GetComponent() type of <TComponent> must be derived from type <nsActorComponent>!");

		for (int i = 0; i < Components.GetCount(); ++i)
		{
			TComponent* check = dynamic_cast<TComponent*>(Components[i]);

			if (check)
			{
				return check;
			}
		}

		return nullptr;
	}


	friend class nsWorld;

};
