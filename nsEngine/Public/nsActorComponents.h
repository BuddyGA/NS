#pragma once

#include "nsRenderer.h"
#include "nsAssetTypes.h"


class nsActor;



class NS_ENGINE_API nsActorComponent
{
protected:
	nsName Name;
	nsActor* Actor;
	bool bAddedToLevel;
	bool bIsVisible;


public:
	nsActorComponent();
	virtual void OnInitialize() {}
	virtual void OnStartPlay() {}
	virtual void OnStopPlay() {}
	virtual void OnTickUpdate(float deltaTime) {}
	virtual void OnDestroy();
	virtual void OnActorAddedToLevel() {}
	virtual void OnActorRemovedFromLevel() {}
	virtual void OnActorTransformUpdated() {}
	virtual void OnVisibilityChanged() {}
	void SetVisibility(bool bVisible);


public:
	NS_NODISCARD_INLINE const nsName& GetName() const noexcept
	{
		return Name;
	}


	NS_NODISCARD_INLINE bool IsVisible() const
	{
		return bIsVisible;
	}


	friend class nsActor;

};



class NS_ENGINE_API nsMeshComponent : public nsActorComponent
{
private:
	nsSharedModelAsset ModelAsset;
	nsTArrayInline<nsMaterialID, NS_ENGINE_ASSET_MODEL_MAX_MESHES> Materials;
	nsRenderContextMeshID RenderMeshId;


public:
	nsMeshComponent();
	virtual void OnDestroy() override;
	virtual void OnActorAddedToLevel() override;
	virtual void OnActorRemovedFromLevel() override;
	virtual void OnActorTransformUpdated() override;
	virtual void OnVisibilityChanged() override;

private:
	void RegisterMesh();
	void UnregisterMesh();

public:
	void SetMesh(nsSharedModelAsset newMesh);
	void SetMaterial(nsMaterialID newMaterial, int index);

};
