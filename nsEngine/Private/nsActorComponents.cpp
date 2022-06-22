#include "nsActorComponents.h"
#include "nsWorld.h"
#include "nsRenderManager.h"
#include "nsConsole.h"



static nsLogCategory ComponentLog("nsComponentLog", nsELogVerbosity::LV_INFO);



// ================================================================================================================================== //
// ACTOR COMPONENT
// ================================================================================================================================== //
NS_DEFINE_OBJECT(nsActorComponent, "ActorComponent", nsObject);

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


void nsActorComponent::OnAddedToLevel()
{
	bAddedToLevel = true;
}


void nsActorComponent::OnRemovedFromLevel()
{
	bAddedToLevel = false;
}




// ================================================================================================================================== //
// TRANSFORM COMPONENT
// ================================================================================================================================== //
NS_DEFINE_OBJECT(nsTransformComponent, "TransformComponent", nsActorComponent);

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
NS_DEFINE_OBJECT(nsCollisionComponent, "CollisionComponent", nsTransformComponent);

nsCollisionComponent::nsCollisionComponent()
{
	PhysicsObject = nsPhysicsObjectID::INVALID;
	ObjectChannel = nsEPhysicsCollisionChannel::Default;
	CollisionChannels = UINT32_MAX;
}


void nsCollisionComponent::OnDestroy()
{
	if (PhysicsObject != nsPhysicsObjectID::INVALID)
	{
		nsPhysicsManager::Get().DestroyPhysicsObject(PhysicsObject);
	}

	nsTransformComponent::OnDestroy();
}


void nsCollisionComponent::OnAddedToLevel()
{
	nsTransformComponent::OnAddedToLevel();

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

	nsTransformComponent::OnRemovedFromLevel();
}


void nsCollisionComponent::OnTransformChanged()
{
	if (PhysicsObject != nsPhysicsObjectID::INVALID)
	{
		const nsTransform worldTransform = GetWorldTransform();
		nsPhysicsManager::Get().SetPhysicsObjectWorldTransform(PhysicsObject, worldTransform.Position, worldTransform.Rotation);
	}
}


void nsCollisionComponent::SetObjectChannel(nsEPhysicsCollisionChannel::Type newObjectChannel)
{
	if (ObjectChannel != newObjectChannel)
	{
		ObjectChannel = newObjectChannel;

		if (PhysicsObject != nsPhysicsObjectID::INVALID)
		{
			nsPhysicsManager::Get().SetPhysicsObjectChannel(PhysicsObject, ObjectChannel);
		}
	}
}


void nsCollisionComponent::SetCollisionChannels(nsPhysicsCollisionChannels newCollisionChannels)
{
	if (CollisionChannels != newCollisionChannels)
	{
		CollisionChannels = newCollisionChannels;

		if (PhysicsObject != nsPhysicsObjectID::INVALID)
		{
			nsPhysicsManager::Get().SetPhysicsObjectCollisionChannels(PhysicsObject, CollisionChannels);
		}
	}
}




// ================================================================================================================================== //
// BOX COLLISION COMPONENT
// ================================================================================================================================== //
NS_DEFINE_OBJECT(nsBoxCollisionComponent, "BoxCollisionComponent", nsCollisionComponent);

nsBoxCollisionComponent::nsBoxCollisionComponent()
{
	HalfExtent = nsVector3(50.0f);
}


void nsBoxCollisionComponent::OnInitialize()
{
	nsActorComponent::OnInitialize();

	PhysicsObject = nsPhysicsManager::Get().CreatePhysicsObject_Box(Name, HalfExtent, true, false, this);
}


void nsBoxCollisionComponent::UpdateCollisionVolume()
{
	if (PhysicsObject != nsPhysicsObjectID::INVALID)
	{
		nsPhysicsManager& physicsManager = nsPhysicsManager::Get();
		physicsManager.UpdatePhysicsObjectShape_Box(PhysicsObject, HalfExtent);
		physicsManager.SetPhysicsObjectChannel(PhysicsObject, ObjectChannel);
		physicsManager.SetPhysicsObjectCollisionChannels(PhysicsObject, CollisionChannels);
	}
}


bool nsBoxCollisionComponent::SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	if (PhysicsObject == nsPhysicsObjectID::INVALID)
	{
		return false;
	}

	return nsPhysicsManager::Get().SceneQuerySweepBox(Actor->GetWorld()->GetPhysicsScene(), hitResult, HalfExtent, GetWorldTransform(), direction, distance, params);
}




// ================================================================================================================================== //
// CONVEX MESH COLLISION COMPONENT
// ================================================================================================================================== //
NS_DEFINE_OBJECT(nsConvexMeshCollisionComponent, "ConvexMeshCollisionComponent", nsCollisionComponent);

nsConvexMeshCollisionComponent::nsConvexMeshCollisionComponent()
{
	Mesh = nsMeshID::INVALID;
}


void nsConvexMeshCollisionComponent::UpdateCollisionVolume()
{
	if (Mesh == nsMeshID::INVALID)
	{
		return;
	}

	const nsMeshVertexData& vertexData = nsMeshManager::Get().GetMeshVertexData(Mesh, 0);
	nsPhysicsManager& physicsManager = nsPhysicsManager::Get();

	if (PhysicsObject == nsPhysicsObjectID::INVALID)
	{
		PhysicsObject = physicsManager.CreatePhysicsObject_ConvexMesh(Name, vertexData.Positions, true, this);
	}

	physicsManager.SetPhysicsObjectChannel(PhysicsObject, ObjectChannel);
	physicsManager.SetPhysicsObjectCollisionChannels(PhysicsObject, CollisionChannels);
}


void nsConvexMeshCollisionComponent::SetMesh(nsMeshID newMesh)
{
	if (Mesh != newMesh)
	{
		Mesh = newMesh;
		UpdateCollisionVolume();
	}
}


bool nsConvexMeshCollisionComponent::SweepTest(nsPhysicsHitResult& hitResult, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params)
{
	return false;
}




// ================================================================================================================================== //
// RENDER COMPONENT
// ================================================================================================================================== //
NS_DEFINE_OBJECT(nsRenderComponent, "RenderComponent", nsTransformComponent);

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
NS_DEFINE_OBJECT(nsMeshComponent, "MeshComponent", nsRenderComponent);

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
