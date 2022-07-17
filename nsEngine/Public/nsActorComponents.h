#pragma once

#include "nsEngineTypes.h"



extern nsLogCategory nsComponentLog;



// ================================================================================================================================== //
// ACTOR COMPONENT
// ================================================================================================================================== //
class NS_ENGINE_API nsActorComponent : public nsObject
{
	NS_DECLARE_OBJECT(nsActorComponent)

protected:
	nsActor* Actor;
	bool bAddedToLevel;
	bool bStartedPlay;


public:
	nsActorComponent();
	virtual void OnInitialize() {}
	virtual void OnDestroy();
	virtual void OnAddedToLevel();
	virtual void OnRemovedFromLevel();
	virtual void OnStartPlay();
	virtual void OnStopPlay();
	virtual void OnTickUpdate(float deltaTime) {}
	virtual void OnPhysicsTickUpdate(float deltaTime) {}
	virtual void OnPostPhysicsTickUpdate() {}
	virtual void OnStaticChanged() {}
	virtual bool IsFullyLoaded() { return true; }
	NS_NODISCARD nsWorld* GetWorld() const;


	NS_NODISCARD_INLINE nsActor* GetActor() const
	{
		return Actor;
	}


	friend class nsActor;

};




// ================================================================================================================================== //
// TRANSFORM COMPONENT
// ================================================================================================================================== //
enum class nsETransformAttachmentMode : uint8
{
	RESET_TRANSFORM = 0,
	KEEP_LOCAL_TRANSFORM,
	KEEP_WORLD_TRANSFORM
};


class NS_ENGINE_API nsTransformComponent : public nsActorComponent
{
	NS_DECLARE_OBJECT(nsTransformComponent)

private:
	enum class EDirtyTransform : uint8
	{
		NONE = 0,
		LOCAL,
		WORLD
	};


	nsTransformComponent* Parent;
	nsTransform LocalTransform;
	nsTransform WorldTransform;
	nsTArrayInline<nsTransformComponent*, NS_ENGINE_TRANSFORM_MAX_CHILDREN> Children;
	EDirtyTransform DirtyTransform;


public:
	nsTransformComponent();

private:
	void UpdateTransform(bool bPhysicsSync);

protected:
	virtual void OnTransformChanged(bool bPhysicsSync) {}
	virtual void OnChildtAttached(nsTransformComponent* child, nsETransformAttachmentMode attachmentMode, nsName socketName) {}
	virtual void OnChildDetached(nsTransformComponent* child) {}

public:
	void AttachToParent(nsTransformComponent* parent, nsETransformAttachmentMode attachmentMode, nsName socketName = nsName::NONE);
	void DetachFromParent();


	NS_NODISCARD_INLINE nsTransformComponent* GetParent() const
	{
		return Parent;
	}


	NS_INLINE void SetLocalTransform(nsTransform transform)
	{
		LocalTransform = transform;
		DirtyTransform = EDirtyTransform::WORLD;
		UpdateTransform(false);
	}


	NS_INLINE void SetLocalPosition(nsVector3 position)
	{
		LocalTransform.Position = position;
		DirtyTransform = EDirtyTransform::WORLD;
		UpdateTransform(false);
	}


	NS_INLINE void SetLocalRotation(nsQuaternion rotation)
	{
		LocalTransform.Rotation = rotation;
		DirtyTransform = EDirtyTransform::WORLD;
		UpdateTransform(false);
	}


	NS_INLINE void SetLocalScale(nsVector3 scale)
	{
		LocalTransform.Scale = scale;
		DirtyTransform = EDirtyTransform::WORLD;
		UpdateTransform(false);
	}


	NS_INLINE void SetWorldTransform(nsTransform transform)
	{
		WorldTransform = transform;
		DirtyTransform = EDirtyTransform::LOCAL;
		UpdateTransform(false);
	}


	NS_INLINE void SetWorldPosition(nsVector3 position)
	{
		WorldTransform.Position = position;
		DirtyTransform = EDirtyTransform::LOCAL;
		UpdateTransform(false);
	}


	NS_INLINE void SetWorldRotation(nsQuaternion rotation)
	{
		WorldTransform.Rotation = rotation;
		DirtyTransform = EDirtyTransform::LOCAL;
		UpdateTransform(false);
	}


	NS_INLINE void SetWorldScale(nsVector3 scale)
	{
		WorldTransform.Scale = scale;
		DirtyTransform = EDirtyTransform::LOCAL;
		UpdateTransform(false);
	}


	NS_NODISCARD_INLINE nsTransform GetLocalTransform()
	{
		UpdateTransform(false);
		return LocalTransform;
	}


	NS_NODISCARD_INLINE nsVector3 GetLocalPosition()
	{
		UpdateTransform(false);
		return LocalTransform.Position;
	}


	NS_NODISCARD_INLINE nsQuaternion GetLocalRotation()
	{
		UpdateTransform(false);
		return LocalTransform.Rotation;
	}


	NS_NODISCARD_INLINE nsVector3 GetLocalScale()
	{
		UpdateTransform(false);
		return LocalTransform.Scale;
	}


	NS_NODISCARD_INLINE nsTransform GetWorldTransform()
	{
		UpdateTransform(false);
		return WorldTransform;
	}


	NS_NODISCARD_INLINE nsVector3 GetWorldPosition()
	{
		UpdateTransform(false);
		return WorldTransform.Position;
	}


	NS_NODISCARD_INLINE nsQuaternion GetWorldRotation()
	{
		UpdateTransform(false);
		return WorldTransform.Rotation;
	}


	NS_NODISCARD_INLINE nsVector3 GetWorldScale()
	{
		UpdateTransform(false);
		return WorldTransform.Scale;
	}


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


	NS_INLINE void Internal_SyncWithPhysicsTransform(nsTransform physicsTransformPose)
	{
		WorldTransform = physicsTransformPose;
		DirtyTransform = EDirtyTransform::LOCAL;
		UpdateTransform(true);
	}

};
