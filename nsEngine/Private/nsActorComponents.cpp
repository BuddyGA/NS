#include "nsActorComponents.h"
#include "nsActor.h"
#include "nsRenderManager.h"



// ================================================================================================================================== //
// ACTOR COMPONENT
// ================================================================================================================================== //
nsActorComponent::nsActorComponent()
{
	Actor = nullptr;
	bAddedToLevel = false;
	bIsVisible = false;
}


void nsActorComponent::OnDestroy()
{
	Name = "";
	Actor = nullptr;
	bAddedToLevel = false;
	bIsVisible = false;
}


void nsActorComponent::SetVisibility(bool bVisible)
{
	if (bIsVisible == bVisible)
	{
		return;
	}

	bIsVisible = bVisible;
	OnVisibilityChanged();
}




// ================================================================================================================================== //
// COLLISION COMPONENT
// ================================================================================================================================== //
nsCollisionComponent::nsCollisionComponent()
{
	PhysicsObject = nsPhysicsObjectID::INVALID;
	Shape = nsEPhysicsShape::NONE;
	CollisionLayers = nsEPhysicsCollisionLayer::Default;
}


void nsCollisionComponent::OnActorAddedToLevel()
{
	UpdateCollisionVolume();
}


void nsCollisionComponent::OnActorRemovedFromLevel()
{
}




// ================================================================================================================================== //
// BOX COLLISION COMPONENT
// ================================================================================================================================== //
nsBoxCollisionComponent::nsBoxCollisionComponent()
{
	HalfExtent = nsVector3(50.0f);
}


void nsBoxCollisionComponent::OnInitialize()
{

}


void nsBoxCollisionComponent::UpdateCollisionVolume()
{
}




// ================================================================================================================================== //
// MESH COMPONENT
// ================================================================================================================================== //
nsMeshComponent::nsMeshComponent()
{
	bIsVisible = true;

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


void nsMeshComponent::OnActorAddedToLevel()
{
	bAddedToLevel = true;
	RegisterMesh();
}


void nsMeshComponent::OnActorRemovedFromLevel()
{
	UnregisterMesh();
	bAddedToLevel = false;
}


void nsMeshComponent::OnActorTransformUpdated()
{
	RegisterMesh();
}


void nsMeshComponent::OnVisibilityChanged()
{
	if (!bAddedToLevel)
	{
		return;
	}

	if (bIsVisible)
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
	if (!bIsVisible || !bAddedToLevel || !ModelAsset.IsValid())
	{
		return;
	}

	nsRenderContextWorld& renderContext = nsRenderManager::Get().GetWorldRenderContext(Actor->GetWorld());

	if (Materials[0] == nsMaterialID::INVALID)
	{
		Materials[0] = nsMaterialManager::Get().GetDefaultMaterial_PhongChecker();
	}

	const nsAssetModelMeshes& meshes = ModelAsset.GetMeshes();

	if (RenderMeshId == nsRenderContextMeshID::INVALID)
	{
		RenderMeshId = renderContext.AddRenderMesh(meshes[0], Materials[0], Actor->GetWorldTransform().ToMatrix());
	}
	else
	{
		renderContext.UpdateRenderMesh(RenderMeshId, meshes[0], Materials[0], Actor->GetWorldTransform().ToMatrix());
	}
}


void nsMeshComponent::UnregisterMesh()
{
	if (RenderMeshId == nsRenderContextMeshID::INVALID || !bAddedToLevel)
	{
		return;
	}

	nsRenderContextWorld& renderContext = nsRenderManager::Get().GetWorldRenderContext(Actor->GetWorld());
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
