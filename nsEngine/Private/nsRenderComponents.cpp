#include "nsRenderComponents.h"
#include "nsRenderManager.h"
#include "nsMaterial.h"
#include "nsAnimationManager.h"
#include "nsConsole.h"




// ================================================================================================================================== //
// RENDER COMPONENT
// ================================================================================================================================== //
NS_CLASS_BEGIN(nsRenderComponent, nsTransformComponent)
NS_CLASS_END(nsRenderComponent)

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
NS_CLASS_BEGIN(nsMeshComponent, nsRenderComponent)
NS_CLASS_END(nsMeshComponent)

nsMeshComponent::nsMeshComponent()
{
	Materials.Add();
	RenderMeshId = nsRenderMeshID::INVALID;
	bGenerateNavMesh = true;
}


void nsMeshComponent::OnDestroy()
{
	UnregisterMesh();

	ModelAsset = nsSharedModelAsset();
	Materials.Resize(1);
	RenderMeshId = nsRenderMeshID::INVALID;

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

	if (RenderMeshId == nsRenderMeshID::INVALID)
	{
		RenderMeshId = renderContext.AddRenderMesh(meshes[0], Materials[0], GetWorldTransform().ToMatrix(), nsAnimationInstanceID::INVALID);
	}
	else
	{
		renderContext.UpdateRenderMesh(RenderMeshId, meshes[0], Materials[0], GetWorldTransform().ToMatrix(), nsAnimationInstanceID::INVALID);
	}
}


void nsMeshComponent::UnregisterMesh()
{
	if (RenderMeshId == nsRenderMeshID::INVALID || !bAddedToLevel)
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




// ================================================================================================================================== //
// SKELETAL MESH COMPONENT
// ================================================================================================================================== //
NS_CLASS_BEGIN(nsSkeletalMeshComponent, nsMeshComponent)
NS_CLASS_END(nsSkeletalMeshComponent)

nsSkeletalMeshComponent::nsSkeletalMeshComponent()
{
	bGenerateNavMesh = false;
	bDebugDrawSkeleton = false;
}


void nsSkeletalMeshComponent::OnStartPlay()
{
	nsMeshComponent::OnStartPlay();

	if (AnimationInstance.IsValid())
	{
		nsAnimationManager::Get().SetInstanceUpdatePose(AnimationInstance, true);
	}
}


void nsSkeletalMeshComponent::OnStopPlay()
{
	if (AnimationInstance.IsValid())
	{
		nsAnimationManager::Get().SetInstanceUpdatePose(AnimationInstance, false);
	}

	nsMeshComponent::OnStopPlay();
}


void nsSkeletalMeshComponent::OnDestroy()
{
	nsAnimationManager::Get().DestroyInstance(AnimationInstance);
	SkeletonAsset = nsSharedSkeletonAsset();

	nsMeshComponent::OnDestroy();
}


void nsSkeletalMeshComponent::RegisterMesh()
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

	if (RenderMeshId == nsRenderMeshID::INVALID)
	{
		RenderMeshId = renderContext.AddRenderMesh(meshes[0], Materials[0], GetWorldTransform().ToMatrix(), AnimationInstance);
	}
	else
	{
		renderContext.UpdateRenderMesh(RenderMeshId, meshes[0], Materials[0], GetWorldTransform().ToMatrix(), AnimationInstance);
	}


#ifdef NS_ENGINE_DEBUG_DRAW
	if (AnimationInstance.IsValid())
	{
		nsAnimationManager::Get().SetInstanceDebugDraw(AnimationInstance, bDebugDrawSkeleton, GetWorldTransform());
	}
#endif // NS_ENGINE_DEBUG_DRAW

}


void nsSkeletalMeshComponent::UnregisterMesh()
{
	if (RenderMeshId == nsRenderMeshID::INVALID || !bAddedToLevel)
	{
		return;
	}

	nsRenderContextWorld& renderContext = nsRenderManager::Get().GetWorldRenderContext(GetWorld());
	renderContext.RemoveRenderMesh(RenderMeshId);
}


void nsSkeletalMeshComponent::SetSkeleton(nsSharedSkeletonAsset newSkeleton)
{
	if (SkeletonAsset == newSkeleton)
	{
		return;
	}

	SkeletonAsset = newSkeleton;
	nsAnimationManager& animationManager = nsAnimationManager::Get();
	animationManager.DestroyInstance(AnimationInstance);

	if (SkeletonAsset.IsValid())
	{
		AnimationInstance = animationManager.CreateInstance("anim_instance", SkeletonAsset.GetSkeleton());
	}
}


void nsSkeletalMeshComponent::PlayAnimation(nsSharedAnimationAsset animation, float playRate, bool bLoop)
{
	if (!SkeletonAsset.IsValid())
	{
		return;
	}

	if (!animation.IsValid())
	{
		return;
	}

	nsAnimationManager::Get().PlayAnimation(AnimationInstance, animation.GetClip(), playRate, bLoop);
}
