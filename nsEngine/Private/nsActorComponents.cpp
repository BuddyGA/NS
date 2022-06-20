#include "nsActorComponents.h"
#include "nsWorld.h"
#include "nsRenderManager.h"
#include "nsConsole.h"



static nsLogCategory ComponentLog("nsComponentLog", nsELogVerbosity::LV_INFO);



// ================================================================================================================================== //
// ACTOR COMPONENT
// ================================================================================================================================== //
nsActorComponent::nsActorComponent()
{
	Actor = nullptr;
	bAddedToLevel = false;
}


void nsActorComponent::OnDestroy()
{
	Name = "";
	Actor = nullptr;
	bAddedToLevel = false;
}




// ================================================================================================================================== //
// TRANSFORM COMPONENT
// ================================================================================================================================== //
nsTransformComponent::nsTransformComponent()
{
	Parent = nullptr;
	LocalTransform = nsTransform();
	WorldTransform = nsTransform();
	DirtyTransform = EDirtyTransform::NONE;
}


void nsTransformComponent::UpdateTransform()
{
	if (DirtyTransform == EDirtyTransform::NONE)
	{
		return;
	}

	if (DirtyTransform == EDirtyTransform::LOCAL)
	{
		LocalTransform.Position = Parent ? Parent->WorldTransform.Position - WorldTransform.Position : WorldTransform.Position;
		LocalTransform.Rotation = Parent ? LocalTransform.Rotation * Parent->WorldTransform.Rotation.GetConjugate() : WorldTransform.Rotation;
		LocalTransform.Scale = Parent ? WorldTransform.Scale / Parent->WorldTransform.Scale : WorldTransform.Scale;
	}
	else if (DirtyTransform == EDirtyTransform::WORLD)
	{
		WorldTransform.Position = Parent ? LocalTransform.Position + Parent->WorldTransform.Position : LocalTransform.Position;
		WorldTransform.Rotation = Parent ? LocalTransform.Rotation * Parent->WorldTransform.Rotation : LocalTransform.Rotation;
		WorldTransform.Scale = Parent ? LocalTransform.Scale * Parent->WorldTransform.Scale : LocalTransform.Scale;
	}

	DirtyTransform = EDirtyTransform::NONE;
	OnTransformChanged();

	// Propagate to children
	for (int i = 0; i < Children.GetCount(); ++i)
	{
		Children[i]->DirtyTransform = EDirtyTransform::WORLD;
		Children[i]->UpdateTransform();
	}
}


void nsTransformComponent::AttachToParent(nsTransformComponent* parent, nsETransformAttachmentMode attachmentMode)
{
	if (parent == nullptr)
	{
		NS_CONSOLE_Warning(ComponentLog, "Ignore attach transform component [%s] to parent. parent is NULL!", *Name);
		return;
	}

	if (this == parent)
	{
		NS_CONSOLE_Warning(ComponentLog, "Fail attach actor [%s] to parent. Cannot attach to itself!", *Name);
		return;
	}

	if (this == parent->Parent)
	{
		parent->DetachFromParent();
	}

	DetachFromParent();
	Parent = parent;
	Parent->Children.Add(this);

	if (attachmentMode == nsETransformAttachmentMode::RESET_TRANSFORM)
	{
		LocalTransform = nsTransform();
		WorldTransform = Parent->WorldTransform;
	}
	else if (attachmentMode == nsETransformAttachmentMode::KEEP_LOCAL_TRANSFORM)
	{
		DirtyTransform = EDirtyTransform::WORLD;
	}
	else // nsETransformAttachmentMode::KEEP_WORLD_TRANSFORM
	{
		DirtyTransform = EDirtyTransform::LOCAL;
	}

	UpdateTransform();
}


void nsTransformComponent::DetachFromParent()
{
	if (Parent == nullptr)
	{
		return;
	}

	Parent->Children.Remove(this);
	Parent = nullptr;
	DirtyTransform = EDirtyTransform::LOCAL;
	UpdateTransform();
}




// ================================================================================================================================== //
// COLLISION COMPONENT
// ================================================================================================================================== //
nsCollisionComponent::nsCollisionComponent()
{
	PhysicsObject = nsPhysicsObjectID::INVALID;
	Shape = nsEPhysicsShape::NONE;
	CollisionChannel = nsEPhysicsCollisionChannel::DEFAULT;
}


void nsCollisionComponent::OnAddedToLevel()
{
	UpdateCollisionVolume();

	if (PhysicsObject != nsPhysicsObjectID::INVALID)
	{
		nsPhysicsManager::Get().AddPhysicsObjectToScene(PhysicsObject, Actor->GetWorld()->GetPhysicsScene());
	}
}


void nsCollisionComponent::OnRemovedFromLevel()
{
	if (PhysicsObject != nsPhysicsObjectID::INVALID)
	{
		nsPhysicsManager::Get().RemovePhysicsObjectFromScene(PhysicsObject, Actor->GetWorld()->GetPhysicsScene());
	}
}


void nsCollisionComponent::SetCollisionChannel(nsEPhysicsCollisionChannel channel)
{
	if (CollisionChannel != channel)
	{
		CollisionChannel = channel;

		if (PhysicsObject != nsPhysicsObjectID::INVALID)
		{
			nsPhysicsManager::Get().SetPhysicsObjectCollisionChannel(PhysicsObject, CollisionChannel);
		}
	}
}




// ================================================================================================================================== //
// BOX COLLISION COMPONENT
// ================================================================================================================================== //
nsBoxCollisionComponent::nsBoxCollisionComponent()
{
	Shape = nsEPhysicsShape::BOX;
	HalfExtent = nsVector3(50.0f);
}


void nsBoxCollisionComponent::OnInitialize()
{
	nsActorComponent::OnInitialize();

	PhysicsObject = nsPhysicsManager::Get().CreatePhysicsObjectRigidBody_Box(Name, Actor, HalfExtent, CollisionChannel, true);
}


void nsBoxCollisionComponent::UpdateCollisionVolume()
{
	if (PhysicsObject != nsPhysicsObjectID::INVALID)
	{
		nsPhysicsManager& physicsManager = nsPhysicsManager::Get();
		physicsManager.UpdatePhysicsObjectShape_Box(PhysicsObject, HalfExtent);
		physicsManager.SetPhysicsObjectCollisionChannel(PhysicsObject, CollisionChannel);
	}
}




// ================================================================================================================================== //
// RENDER COMPONENT
// ================================================================================================================================== //

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
	bAddedToLevel = true;
	RegisterMesh();
}


void nsMeshComponent::OnRemovedFromLevel()
{
	UnregisterMesh();
	bAddedToLevel = false;
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

	nsRenderContextWorld& renderContext = nsRenderManager::Get().GetWorldRenderContext(Actor->GetWorld());

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
