#pragma once

#include "nsObject.h"
#include "nsPhysics.h"
#include "nsRenderer.h"
#include "nsAssetTypes.h"



class NS_ENGINE_API nsActorComponent : public nsObject
{
	NS_DECLARE_OBJECT()

protected:
	nsActor* Actor;
	bool bAddedToLevel;


public:
	nsActorComponent();
	virtual void OnInitialize() {}
	virtual void OnStartPlay() {}
	virtual void OnStopPlay() {}
	virtual void OnTickUpdate(float deltaTime) {}
	virtual void OnDestroy();
	virtual void OnAddedToLevel();
	virtual void OnRemovedFromLevel();
	virtual bool IsFullyLoaded() { return true; }


	NS_NODISCARD_INLINE nsActor* GetActor() const
	{
		return Actor;
	}


	friend class nsActor;

};



enum class nsETransformAttachmentMode : uint8
{
	RESET_TRANSFORM = 0,
	KEEP_LOCAL_TRANSFORM,
	KEEP_WORLD_TRANSFORM
};


class NS_ENGINE_API nsTransformComponent : public nsActorComponent
{
	NS_DECLARE_OBJECT()

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
	void UpdateTransform();

protected:
	virtual void OnTransformChanged() {}

public:
	void AttachToParent(nsTransformComponent* parent, nsETransformAttachmentMode attachmentMode);
	void DetachFromParent();


	NS_NODISCARD_INLINE nsTransformComponent* GetParent() const
	{
		return Parent;
	}


	NS_INLINE void SetLocalTransform(nsTransform transform)
	{
		LocalTransform = transform;
		DirtyTransform = EDirtyTransform::WORLD;
		UpdateTransform();
	}


	NS_INLINE void SetLocalPosition(nsVector3 position)
	{
		if (DirtyTransform == EDirtyTransform::LOCAL)
		{
			UpdateTransform();
		}

		LocalTransform.Position = position;
		DirtyTransform = EDirtyTransform::WORLD;
	}


	NS_INLINE void SetLocalRotation(nsQuaternion rotation)
	{
		if (DirtyTransform == EDirtyTransform::LOCAL)
		{
			UpdateTransform();
		}

		LocalTransform.Rotation = rotation;
		DirtyTransform = EDirtyTransform::WORLD;
	}


	NS_INLINE void SetLocalScale(nsVector3 scale)
	{
		if (DirtyTransform == EDirtyTransform::LOCAL)
		{
			UpdateTransform();
		}

		LocalTransform.Scale = scale;
		DirtyTransform = EDirtyTransform::WORLD;
	}


	NS_INLINE void SetWorldTransform(nsTransform transform)
	{
		WorldTransform = transform;
		DirtyTransform = EDirtyTransform::LOCAL;
		UpdateTransform();
	}


	NS_INLINE void SetWorldPosition(nsVector3 position)
	{
		if (DirtyTransform == EDirtyTransform::WORLD)
		{
			UpdateTransform();
		}

		WorldTransform.Position = position;
		DirtyTransform = EDirtyTransform::LOCAL;
	}


	NS_INLINE void SetWorldRotation(nsQuaternion rotation)
	{
		if (DirtyTransform == EDirtyTransform::WORLD)
		{
			UpdateTransform();
		}

		WorldTransform.Rotation = rotation;
		DirtyTransform = EDirtyTransform::LOCAL;
	}


	NS_INLINE void SetWorldScale(nsVector3 scale)
	{
		if (DirtyTransform == EDirtyTransform::WORLD)
		{
			UpdateTransform();
		}

		WorldTransform.Scale = scale;
		DirtyTransform = EDirtyTransform::LOCAL;
	}


	NS_NODISCARD_INLINE nsTransform GetLocalTransform()
	{
		UpdateTransform();
		return LocalTransform;
	}


	NS_NODISCARD_INLINE nsVector3 GetLocalPosition()
	{
		UpdateTransform();
		return LocalTransform.Position;
	}


	NS_NODISCARD_INLINE nsQuaternion GetLocalRotation()
	{
		UpdateTransform();
		return LocalTransform.Rotation;
	}


	NS_NODISCARD_INLINE nsVector3 GetLocalScale()
	{
		UpdateTransform();
		return LocalTransform.Scale;
	}


	NS_NODISCARD_INLINE nsTransform GetWorldTransform()
	{
		UpdateTransform();
		return WorldTransform;
	}


	NS_NODISCARD_INLINE nsVector3 GetWorldPosition()
	{
		UpdateTransform();
		return WorldTransform.Position;
	}


	NS_NODISCARD_INLINE nsQuaternion GetWorldRotation()
	{
		UpdateTransform();
		return WorldTransform.Rotation;
	}


	NS_NODISCARD_INLINE nsVector3 GetWorldScale()
	{
		UpdateTransform();
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

};



class NS_ENGINE_API nsCollisionComponent : public nsTransformComponent
{
	NS_DECLARE_OBJECT()

protected:
	nsPhysicsObjectID PhysicsObject;
	nsEPhysicsCollisionChannel::Type ObjectChannel;
	nsPhysicsCollisionChannels CollisionChannels;


public:
	nsCollisionComponent();
	virtual void OnDestroy() override;
	virtual void OnAddedToLevel() override;
	virtual void OnRemovedFromLevel() override;
	virtual void OnTransformChanged() override;

	void SetObjectChannel(nsEPhysicsCollisionChannel::Type newObjectChannel);
	void SetCollisionChannels(nsPhysicsCollisionChannels newCollisionChannels);
	virtual void UpdateCollisionVolume() = 0;
	virtual bool SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams()) = 0;


	NS_NODISCARD_INLINE nsEPhysicsCollisionChannel::Type GetObjectChannel() const
	{
		return ObjectChannel;
	}


	NS_NODISCARD_INLINE nsPhysicsCollisionChannels GetCollisionChannels() const
	{
		return CollisionChannels;
	}

};



class NS_ENGINE_API nsBoxCollisionComponent : public nsCollisionComponent
{
	NS_DECLARE_OBJECT()

public:
	nsVector3 HalfExtent;


public:
	nsBoxCollisionComponent();
	virtual void OnInitialize() override;
	virtual void UpdateCollisionVolume() override;
	virtual bool SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams()) override;

};




class NS_ENGINE_API nsConvexMeshCollisionComponent : public nsCollisionComponent
{
	NS_DECLARE_OBJECT()

private:
	nsMeshID Mesh;


public:
	nsConvexMeshCollisionComponent();
	virtual void UpdateCollisionVolume() override;
	virtual bool SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams()) override;

	void SetMesh(nsMeshID newMesh);

};




class NS_ENGINE_API nsRenderComponent : public nsTransformComponent
{
	NS_DECLARE_OBJECT()

private:
	bool bIsVisible;


public:
	nsRenderComponent();

protected:
	virtual void OnVisibilityChanged() {}

public:
	void SetVisibility(bool bVisible);


	NS_NODISCARD_INLINE bool IsVisible() const
	{
		return bIsVisible;
	}

};




class NS_ENGINE_API nsMeshComponent : public nsRenderComponent
{
	NS_DECLARE_OBJECT()

private:
	nsSharedModelAsset ModelAsset;
	nsTArrayInline<nsMaterialID, NS_ENGINE_ASSET_MODEL_MAX_MESHES> Materials;
	nsRenderContextMeshID RenderMeshId;


public:
	nsMeshComponent();
	virtual void OnDestroy() override;
	virtual void OnAddedToLevel() override;
	virtual void OnRemovedFromLevel() override;

protected:
	virtual void OnTransformChanged() override;
	virtual void OnVisibilityChanged() override;

private:
	void RegisterMesh();
	void UnregisterMesh();

public:
	void SetMesh(nsSharedModelAsset newMesh);
	void SetMaterial(nsMaterialID newMaterial, int index);


	NS_NODISCARD_INLINE const nsSharedModelAsset& GetModelAsset() const
	{
		return ModelAsset;
	}

};
