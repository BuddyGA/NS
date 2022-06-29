#pragma once

#include "nsActorComponents.h"
#include "nsAssetTypes.h"



// ================================================================================================================================== //
// RENDER COMPONENT
// ================================================================================================================================== //
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




// ================================================================================================================================== //
// MESH COMPONENT
// ================================================================================================================================== //
class NS_ENGINE_API nsMeshComponent : public nsRenderComponent
{
	NS_DECLARE_OBJECT()

protected:
	nsSharedModelAsset ModelAsset;
	nsTArrayInline<nsMaterialID, NS_ENGINE_ASSET_MODEL_MAX_MESH> Materials;
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




// ================================================================================================================================== //
// SKELETAL MESH COMPONENT
// ================================================================================================================================== //
class NS_ENGINE_API nsSkeletalMeshComponent : public nsMeshComponent
{
	NS_DECLARE_OBJECT()

private:
	nsSharedSkeletonAsset SkeletonAsset;
	nsAnimationInstanceID AnimationInstance;


public:
	nsSkeletalMeshComponent();
	void SetSkeleton(nsSharedSkeletonAsset newSkeleton);

};
