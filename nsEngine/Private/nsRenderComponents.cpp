#include "nsRenderComponents.h"
#include "nsRenderManager.h"
#include "nsMaterial.h"




// ================================================================================================================================== //
// RENDER COMPONENT
// ================================================================================================================================== //
NS_DEFINE_OBJECT(nsRenderComponent, nsTransformComponent);

nsRenderComponent::nsRenderComponent()
{
	bIsVisible = true;
}


void nsRenderComponent::SetVisibility(bool bVisible)
{
	if (bIsVisible != bVisible)
	{
		bIsVisible = bVisible;
		OnVisibilityChanged();
	}
}




// ================================================================================================================================== //
// MESH COMPONENT
// ================================================================================================================================== //
NS_DEFINE_OBJECT(nsMeshComponent, nsRenderComponent);

nsMeshComponent::nsMeshComponent()
{
	Materials.Add();
	RenderMeshId = nsRenderContextMeshID::INVALID;
}


void nsMeshComponent::OnDestroy()
{
	UnregisterMesh();

	ModelAsset = nsSharedModelAsset();
	Materials.Resize(1);
	RenderMeshId = nsRenderContextMeshID::INVALID;

	nsActorComponent::OnDestroy();
}


void nsMeshComponent::OnAddedToLevel()
{
	nsRenderComponent::OnAddedToLevel();

	RegisterMesh();
}


void nsMeshComponent::OnRemovedFromLevel()
{
	UnregisterMesh();

	nsRenderComponent::OnRemovedFromLevel();
}


void nsMeshComponent::OnTransformChanged()
{
	RegisterMesh();
}


void nsMeshComponent::OnVisibilityChanged()
{
	if (!bAddedToLevel)
	{
		return;
	}

	if (IsVisible())
	{
		RegisterMesh();
	}
	else
	{
		UnregisterMesh();
	}
}


void nsMeshComponent::RegisterMesh()
{
	if (!IsVisible() || !bAddedToLevel || !ModelAsset.IsValid())
	{
		return;
	}

	nsRenderContextWorld& renderContext = nsRenderManager::Get().GetWorldRenderContext(GetWorld());

	if (Materials[0] == nsMaterialID::INVALID)
	{
		Materials[0] = nsMaterialManager::Get().GetDefaultMaterial_PhongChecker();
	}

	const nsAssetModelMeshes& meshes = ModelAsset.GetMeshes();

	if (RenderMeshId == nsRenderContextMeshID::INVALID)
	{
		RenderMeshId = renderContext.AddRenderMesh(meshes[0], Materials[0], GetWorldTransform().ToMatrix());
	}
	else
	{
		renderContext.UpdateRenderMesh(RenderMeshId, meshes[0], Materials[0], GetWorldTransform().ToMatrix());
	}
}


void nsMeshComponent::UnregisterMesh()
{
	if (RenderMeshId == nsRenderContextMeshID::INVALID || !bAddedToLevel)
	{
		return;
	}

	nsRenderContextWorld& renderContext = nsRenderManager::Get().GetWorldRenderContext(GetWorld());
	renderContext.RemoveRenderMesh(RenderMeshId);
}


void nsMeshComponent::SetMesh(nsSharedModelAsset newMesh)
{
	if (ModelAsset == newMesh)
	{
		return;
	}

	ModelAsset = newMesh;

	if (ModelAsset.IsValid())
	{
		RegisterMesh();
	}
	else if (IsVisible())
	{
		UnregisterMesh();
	}
}


void nsMeshComponent::SetMaterial(nsMaterialID newMaterial, int index)
{
	NS_Assert(newMaterial != nsMaterialID::INVALID);
	NS_Assert(index >= 0 && index < 8);

	if (Materials[index] == newMaterial)
	{
		return;
	}

	Materials[index] = newMaterial;
	RegisterMesh();
}
