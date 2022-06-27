#pragma once

#include "nsActorComponents.h"



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
		Static					= (1 << 5),
		Initialized				= (1 << 6),
		AddedToLevel			= (1 << 7),
		StartedPlay				= (1 << 8),
		PendingDestroy			= (1 << 9),
	};
};

typedef uint32 nsActorFlags;



class NS_ENGINE_API nsActor : public nsObject
{
	NS_DECLARE_OBJECT()

protected:
	nsLevel* Level;
	nsActorFlags Flags;
	nsTransformComponent* RootComponent;

private:
	nsActor* Parent;
	nsTArray<nsActorComponent*> Components;
	nsTArrayInline<nsActor*, NS_ENGINE_TRANSFORM_MAX_CHILDREN> Children;


	static nsMemory ComponentMemory;


public:
	nsActor();
	virtual void OnInitialize();
	virtual void OnStartPlay();
	virtual void OnStopPlay();
	virtual void OnTickUpdate(float deltaTime);
	virtual void OnPhysicsTickUpdate(float fixedDeltaTime);
	virtual void OnDestroy();
	void OnAddedToLevel();
	void OnRemovedFromLevel();

	NS_NODISCARD nsWorld* GetWorld() const;
	void SetAsStatic(bool bIsStatic);
	void SetRootComponent(nsTransformComponent* newRootComponent);
	void AttachToParent(nsActor* parent, nsETransformAttachmentMode attachmentMode);
	void DetachFromParent();


	NS_INLINE void SetLocalTransform(nsTransform transform)
	{
		RootComponent->SetLocalTransform(transform);
	}


	NS_INLINE void SetLocalPosition(nsVector3 position)
	{
		RootComponent->SetLocalPosition(position);
	}


	NS_INLINE void SetLocalRotation(nsQuaternion rotation)
	{
		RootComponent->SetLocalRotation(rotation);
	}


	NS_INLINE void SetLocalScale(nsVector3 scale)
	{
		RootComponent->SetLocalScale(scale);
	}


	NS_INLINE void SetWorldTransform(nsTransform transform)
	{
		RootComponent->SetWorldTransform(transform);
	}


	NS_INLINE void SetWorldPosition(nsVector3 position)
	{
		RootComponent->SetWorldPosition(position);
	}


	NS_INLINE void SetWorldRotation(nsQuaternion rotation)
	{
		RootComponent->SetWorldRotation(rotation);
	}


	NS_INLINE void SetWorldScale(nsVector3 scale)
	{
		RootComponent->SetWorldScale(scale);
	}


	NS_NODISCARD_INLINE nsTransform GetLocalTransform()
	{
		return RootComponent->GetLocalTransform();
	}


	NS_NODISCARD_INLINE nsVector3 GetLocalPosition()
	{
		return RootComponent->GetLocalPosition();
	}


	NS_NODISCARD_INLINE nsQuaternion GetLocalRotation()
	{
		return RootComponent->GetLocalRotation();
	}


	NS_NODISCARD_INLINE nsVector3 GetLocalScale()
	{
		return RootComponent->GetLocalScale();
	}


	NS_NODISCARD_INLINE nsTransform GetWorldTransform()
	{
		return RootComponent->GetWorldTransform();
	}


	NS_NODISCARD_INLINE nsVector3 GetWorldPosition()
	{
		return RootComponent->GetWorldPosition();
	}


	NS_NODISCARD_INLINE nsQuaternion GetWorldRotation()
	{
		return RootComponent->GetWorldRotation();
	}


	NS_NODISCARD_INLINE nsVector3 GetWorldScale()
	{
		return RootComponent->GetWorldScale();
	}


	NS_INLINE void AddLocalPosition(nsVector3 delta)
	{
		RootComponent->AddLocalPosition(delta);
	}


	NS_INLINE void AddLocalRotation(nsQuaternion delta)
	{
		RootComponent->AddLocalRotation(delta);
	}


	NS_INLINE void AddLocalScale(nsVector3 delta)
	{
		RootComponent->AddLocalScale(delta);
	}


	NS_INLINE void AddWorldPosition(nsVector3 delta)
	{
		RootComponent->AddWorldPosition(delta);
	}


	NS_INLINE void AddWorldRotation(nsQuaternion delta)
	{
		RootComponent->AddWorldRotation(delta);
	}


	NS_INLINE void AddWorldScale(nsVector3 delta)
	{
		RootComponent->AddWorldScale(delta);
	}


	NS_NODISCARD_INLINE bool IsStatic() const
	{
		return (Flags & nsEActorFlag::Static);
	}


	NS_NODISCARD_INLINE bool IsPendingDestroy() const
	{
		return (Flags & nsEActorFlag::PendingDestroy);
	}



	nsActorComponent* FindComponent(const nsName& name) const;
	bool RemoveComponent(nsActorComponent* component);


	NS_NODISCARD_INLINE nsTransformComponent* GetRootComponent() const
	{
		return RootComponent;
	}


	template<typename TComponent = nsActorComponent>
	NS_INLINE TComponent* AddComponent(nsName name)
	{
		static_assert(std::is_base_of<nsActorComponent, TComponent>::value, "AddComponent() type of <TComponent> must be derived from type <nsActorComponent>!");

		NS_Validate_IsMainThread();

		nsActorComponent* checkComponent = FindComponent(name);
		NS_ValidateV(checkComponent == nullptr, "Actor [%s] already had component with name [%s]!", *Name, *name);

		TComponent* newComponent = ComponentMemory.AllocateConstruct<TComponent>();
		newComponent->Name = name;
		newComponent->Actor = this;
		//newComponent->OnInitialize();

		Components.Add(newComponent);

		if constexpr (std::is_base_of<nsTransformComponent, TComponent>::value)
		{
			if (RootComponent)
			{
				newComponent->AttachToParent(RootComponent, nsETransformAttachmentMode::RESET_TRANSFORM);
			}
		}

		if (Flags & nsEActorFlag::Initialized)
		{
			newComponent->OnInitialize();
		}

		if (Flags & nsEActorFlag::AddedToLevel)
		{
			newComponent->OnAddedToLevel();
		}

		if ((Flags & nsEActorFlag::CallStartStopPlay) && (Flags & nsEActorFlag::StartedPlay))
		{
			newComponent->OnStartPlay();
		}

		return newComponent;
	}


	/*
	template<typename TComponent = nsActorComponent>
	NS_INLINE bool RemoveComponent()
	{
		static_assert(std::is_base_of<nsActorComponent, TComponent>::value, "RemoveComponent() type of <TComponent> must be derived from type <nsActorComponent>!");

		for (int i = 0; i < Components.GetCount(); ++i)
		{
			TComponent* check = ns_Cast<TComponent>(Components[i]);

			if (check)
			{
				check->OnRemovedFromLevel();
				check->OnDestroy();
				Components.RemoveAt(i);
				ComponentMemory.DeallocateDestruct<TComponent>(check);

				return true;
			}
		}

		return false;
	}
	*/


	template<typename TComponent = nsActorComponent>
	NS_INLINE TComponent* GetComponent() const
	{
		static_assert(std::is_base_of<nsActorComponent, TComponent>::value, "GetComponent() type of <TComponent> must be derived from type <nsActorComponent>!");

		for (int i = 0; i < Components.GetCount(); ++i)
		{
			if (TComponent* comp = ns_Cast<TComponent>(Components[i]))
			{
				return comp;
			}
		}

		return nullptr;
	}


	friend class nsWorld;

};
