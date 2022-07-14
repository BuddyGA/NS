#include "cstEditor.h"
#include "cstGame.h"
#include "nsRenderManager.h"
#include "nsPhysicsManager.h"
#include "nsRenderComponents.h"
#include "nsPhysicsComponents.h"



nsLogCategory EditorLog(TEXT("cstEditorLog"), nsELogVerbosity::LV_DEBUG);



// ================================================================================================================================================================================== //
// EDITOR - CONTEXT OPTIONS
// ================================================================================================================================================================================== //
cstEditorContextMenu::cstEditorContextMenu()
{
	Window.Name = "editor_context_menu";
	Window.BorderWidth = 1.0f;
	Window.bTitleBar = false;
	Window.bMoveable = false;
	Window.bResizable = false;

	InputSnapTranslation.Name = "context_snap_translation";
	InputSnapTranslation.Size = nsPointFloat(0.0f, 22.0f);
	InputSnapTranslation.MinValue = 0.0f;
	InputSnapTranslation.MaxValue = 1000.0f;
	InputSnapTranslation.DragSpeed = 1.0f;
	InputSnapTranslation.BackgroundColor = nsColor(50, 50, 50, 255);

	InputSnapRotation.Name = "context_snap_rotation";
	InputSnapRotation.Size = nsPointFloat(0.0f, 22.0f);
	InputSnapRotation.MinValue = 0.0f;
	InputSnapRotation.MaxValue = 90.0f;
	InputSnapRotation.DragSpeed = 1.0f;
	InputSnapRotation.BackgroundColor = nsColor(50, 50, 50, 255);

	InputSnapScale.Name = "context_snap_scale";
	InputSnapScale.Size = nsPointFloat(0.0f, 22.0f);
	InputSnapScale.MinValue = 0.0f;
	InputSnapScale.MaxValue = 100.0f;
	InputSnapScale.DragSpeed = 0.125f;
	InputSnapScale.BackgroundColor = nsColor(50, 50, 50, 255);
}


bool cstEditorContextMenu::DrawGUI(nsGUIContext& context, const nsPointFloat& screenCoord)
{
	int commitCount = 0;

	nsGUIRect rect;
	rect.Left = screenCoord.X;
	rect.Top = screenCoord.Y;
	rect.Right = rect.Left + 128.0f;
	rect.Bottom = rect.Top + 256.0f;

	Window.SetRect(rect);

	Window.BeginDraw(context);
	{
		const nsGUIRect contentRect = Window.GetContentRect();
		context.BeginRegion(nullptr, contentRect, nsPointFloat(2.0f), nsEGUIElementLayout::VERTICAL, nsEGUIScrollOption::None, false, "editor_context_menu");
		{
			context.AddControlText(TEXT("Snap Position:"));
			if (InputSnapTranslation.Draw(context))
			{
				commitCount++;
			}

			context.AddControlText(TEXT("Snap Rotation:"));
			if (InputSnapRotation.Draw(context))
			{
				commitCount++;
			}

			context.AddControlText(TEXT("Snap Scale:"));
			if (InputSnapScale.Draw(context))
			{
				commitCount++;
			}
		}
		context.EndRegion();
	}
	Window.EndDraw(context);

	return commitCount > 0;
}





// ================================================================================================================================================================================== //
// EDITOR - MAIN
// ================================================================================================================================================================================== //
cstEditor::cstEditor(cstGame* game)
{
	Game = game;

	AssetExplorer.ScanAssets();

	ViewMode = cstEEditorViewMode::PERSPECTIVE;
	FocusActor = nullptr;
	bShowAssetExplorer = true;
	bShowActorInspector = true;
	bShowWorldOutliner = true;
	bShowContextMenu = false;
	bIsLocalCoordSpace = false;
	bSceneViewportHovered = false;
	bIsAnyControlFocused = false;
	bIsDraggingAsset = false;
	bIsDraggingAssetSpawned = false;
	bKeyPressed_LeftShift = false;

	CameraTransform.Position = nsVector3(0.0f, 1000.0f, -500.0f);
	CameraTransform.Rotation = nsQuaternion::FromRotation(30.0f, -45.0f, 0.0f);
	CameraRotation = nsVector2(30.0f, -45.0f);
	CameraScrollValue = 0.0f;
	bMovingCamera = false;

	MainWorld = nullptr;
	MainViewport = nullptr;
	MainRenderer = nullptr;
}


void cstEditor::OnMouseMove(const nsMouseMoveEventArgs& e)
{
	MouseCoord = nsPointFloat(static_cast<float>(e.Position.X), static_cast<float>(e.Position.Y));

	if (bSceneViewportHovered)
	{
		if (bIsDraggingAsset && DragDropAssetInfo.Type == nsEAssetType::MODEL)
		{
			NS_Assert(MainViewport);
			NS_Assert(MainWorld);

			const nsVector2 mousePosition(static_cast<float>(e.Position.X), static_cast<float>(e.Position.Y));

			nsVector3 worldPosition;
			nsVector3 worldDirection;
			const bool bValidProjection = MainViewport->ProjectToWorld(mousePosition, worldPosition, worldDirection);
			const nsVector3 projectedPosition = worldPosition + worldDirection * 800.0f;

			if (!bIsDraggingAssetSpawned && bValidProjection)
			{
				NS_CONSOLE_Log(EditorLog, TEXT("Spawn actor from asset [%s]"), *DragDropAssetInfo.Name.ToString());
				nsSharedModelAsset ModelAsset = nsAssetManager::Get().LoadModelAsset(DragDropAssetInfo.Name);

				nsActor* newActor = MainWorld->CreateActor(DragDropAssetInfo.Name.ToString(), true, projectedPosition);
				{
					nsMeshComponent* meshComp = newActor->AddComponent<nsMeshComponent>("mesh");
					meshComp->SetMesh(ModelAsset);
					AddMousePickingForActor(newActor);
					newActor->SetRootComponent(meshComp);
					MainWorld->AddActorToLevel(newActor);
				}

				bIsDraggingAssetSpawned = true;
				SelectFocusActor(newActor);
			}
			else
			{
				NS_Assert(FocusActor);
				FocusActor->SetWorldPosition(projectedPosition);
			}
		}
		else if (bMovingCamera)
		{
			CameraRotation.X += 0.25f * e.DeltaPosition.Y;
			CameraRotation.Y += 0.25f * e.DeltaPosition.X;
		}
	}

	if (MainViewport && FocusActor)
	{
		const nsVector2 mousePosition(static_cast<float>(e.Position.X), static_cast<float>(e.Position.Y));
		nsTransform actorTransform = bIsLocalCoordSpace ? FocusActor->GetLocalTransform() : FocusActor->GetWorldTransform();

		if (ActorGizmo.UpdateTransform(MainViewport, mousePosition, ViewMode, actorTransform, bIsLocalCoordSpace))
		{
			if (bIsLocalCoordSpace)
			{
				FocusActor->SetLocalTransform(actorTransform);
			}
			else
			{
				FocusActor->SetWorldTransform(actorTransform);
			}

			ActorInspector.UpdateCacheTransform();
		}
	}
	else
	{
		ActorGizmo.RemoveSelected();
	}
}


void cstEditor::OnMouseButton(const nsMouseButtonEventArgs& e)
{
	NS_Assert(MainViewport);
	NS_Assert(MainWorld);

	if (e.ButtonState == nsEButtonState::PRESSED)
	{
		if (bSceneViewportHovered)
		{
			if (e.Key == nsEInputKey::MOUSE_LEFT)
			{
				if (FocusActor)
				{
					const nsTransform transform = bIsLocalCoordSpace ? FocusActor->GetLocalTransform() : FocusActor->GetWorldTransform();
					ActorGizmo.BeginTransform(MainViewport, nsVector2(static_cast<float>(e.Position.X), static_cast<float>(e.Position.Y)), transform, bIsLocalCoordSpace);
				}
			}

			if (e.Key == nsEInputKey::MOUSE_RIGHT && !bMovingCamera)
			{
				bMovingCamera = true;
				Game->SetMouseRelativeMode(true);
				Game->ShowMouseCursor(false);
			}
		}
	}
	else if (e.ButtonState == nsEButtonState::RELEASED)
	{
		if (e.Key == nsEInputKey::MOUSE_LEFT)
		{
			if (bIsDraggingAsset)
			{
				NS_CONSOLE_Debug(EditorLog, TEXT("End drag-drop asset"));
				bIsDraggingAsset = false;
			}
			else if (!ActorGizmo.IsUpdating() && bSceneViewportHovered)
			{
				nsPhysicsHitResult hitResult;
				const bool bFoundHit = nsPhysicsManager::Get().SceneQueryMousePicking(MainWorld->GetPhysicsScene(), hitResult, nsVector2(static_cast<float>(e.Position.X), static_cast<float>(e.Position.Y)), MainViewport);
				SelectFocusActor(bFoundHit ? hitResult.Actor : nullptr);
			}

			ActorGizmo.EndTransform();
		}

		if (e.Key == nsEInputKey::MOUSE_RIGHT && bMovingCamera)
		{
			bMovingCamera = false;
			Game->SetMouseRelativeMode(false);
			Game->ShowMouseCursor(true);
		}
	}
}


void cstEditor::OnMouseWheel(const nsMouseWheelEventArgs& e)
{
	if (bSceneViewportHovered)
	{
		CameraScrollValue += e.ScrollValue.Y;
	}
}


void cstEditor::OnKeyboardButton(const nsKeyboardButtonEventArgs& e)
{
	if (e.ButtonState == nsEButtonState::PRESSED)
	{
		if (e.Key == nsEInputKey::KEYBOARD_F1)
		{
			bShowAssetExplorer = !bShowAssetExplorer;
		}
		else if (e.Key == nsEInputKey::KEYBOARD_F2)
		{
			bShowActorInspector = !bShowActorInspector;
		}
		else if (e.Key == nsEInputKey::KEYBOARD_F3)
		{
			bShowWorldOutliner = !bShowWorldOutliner;
		}
		else if (e.Key == nsEInputKey::KEYBOARD_F4)
		{
			MainRenderer->DebugDrawFlags ^= nsERenderDebugDraw::Wireframe;
			NS_CONSOLE_Log(EditorLog, TEXT("Debug draw wireframe [%s]"), (MainRenderer->DebugDrawFlags & nsERenderDebugDraw::Wireframe) ? TEXT("ON") : TEXT("OFF"));
		}
		else if (e.Key == nsEInputKey::KEYBOARD_F5)
		{
			MainRenderer->DebugDrawFlags ^= nsERenderDebugDraw::Collision;
			NS_CONSOLE_Log(EditorLog, TEXT("Debug draw collision [%s]"), (MainRenderer->DebugDrawFlags & nsERenderDebugDraw::Collision) ? TEXT("ON") : TEXT("OFF"));
		}
		else if (e.Key == nsEInputKey::KEYBOARD_F6)
		{
			MainRenderer->DebugDrawFlags ^= nsERenderDebugDraw::Skeleton;
			NS_CONSOLE_Log(EditorLog, TEXT("Debug draw skeleton [%s]"), (MainRenderer->DebugDrawFlags & nsERenderDebugDraw::Skeleton) ? TEXT("ON") : TEXT("OFF"));
		}
		else if (e.Key == nsEInputKey::KEYBOARD_F7)
		{
			MainRenderer->DebugDrawFlags ^= nsERenderDebugDraw::NavMesh;
			NS_CONSOLE_Log(EditorLog, TEXT("Debug draw NavMesh [%s]"), (MainRenderer->DebugDrawFlags & nsERenderDebugDraw::NavMesh) ? TEXT("ON") : TEXT("OFF"));
		}
		else if (e.Key == nsEInputKey::KEYBOARD_F8)
		{
			if (MainWorld->HasStartedPlay())
			{
				MainWorld->DispatchStopPlay();
			}
			else
			{
				MainWorld->DispatchStartPlay();
			}
		}

		if (bSceneViewportHovered)
		{
			if (bMovingCamera)
			{
				if (e.Key == nsEInputKey::KEYBOARD_A)
				{
					CameraMoveAxis.X = -1.0f;
				}
				else if (e.Key == nsEInputKey::KEYBOARD_D)
				{
					CameraMoveAxis.X = 1.0f;
				}

				if (e.Key == nsEInputKey::KEYBOARD_W)
				{
					CameraMoveAxis.Z = 1.0f;
				}
				else if (e.Key == nsEInputKey::KEYBOARD_S)
				{
					CameraMoveAxis.Z = -1.0f;
				}
			}
			else if (e.Key == nsEInputKey::KEYBOARD_Q)
			{
				bIsLocalCoordSpace = !bIsLocalCoordSpace;
			}
			else if (e.Key == nsEInputKey::KEYBOARD_W)
			{
				ActorGizmo.Mode = cstEEditorGizmoTransformMode::TRANSLATE;
			}
			else if (e.Key == nsEInputKey::KEYBOARD_E)
			{
				ActorGizmo.Mode = cstEEditorGizmoTransformMode::ROTATE;
			}
			else if (e.Key == nsEInputKey::KEYBOARD_R)
			{
				ActorGizmo.Mode = cstEEditorGizmoTransformMode::SCALE;
			}

			if (e.Key == nsEInputKey::KEYBOARD_SHIFT_LEFT && !bKeyPressed_LeftShift)
			{
				bKeyPressed_LeftShift = true;
			}

			if (bKeyPressed_LeftShift)
			{
				if (e.Key == nsEInputKey::KEYBOARD_SPACEBAR)
				{
					NS_CONSOLE_Debug(EditorLog, TEXT("Toggle gizmo options!"));
					ContextMenuCoord = MouseCoord;
					bShowContextMenu = !bShowContextMenu;
				}
			}
		}

		if (!bIsAnyControlFocused)
		{
			if (e.Key == nsEInputKey::KEYBOARD_ESCAPE)
			{
				SelectFocusActor(nullptr);
			}
			else if (e.Key == nsEInputKey::KEYBOARD_DELETE && FocusActor)
			{
				MainWorld->DestroyActor(FocusActor);
				SelectFocusActor(nullptr);
			}
			else if (e.Key == nsEInputKey::KEYBOARD_Z)
			{
				MoveFocusActorDownToFloor();
			}
			else if (e.Key == nsEInputKey::KEYBOARD_NUMPAD_0)
			{
				if (ViewMode != cstEEditorViewMode::PERSPECTIVE)
				{
					NS_CONSOLE_Log(EditorLog, TEXT("Set camera view mode perspective"));
					ViewMode = cstEEditorViewMode::PERSPECTIVE;
					MainViewport->SetProjectionMode(false);
				}
			}
			else if (e.Key == nsEInputKey::KEYBOARD_NUMPAD_1)
			{
				if (ViewMode != cstEEditorViewMode::ORTHO_FRONT)
				{
					NS_CONSOLE_Log(EditorLog, TEXT("Set camera view mode orthographic-front"));
					ViewMode = cstEEditorViewMode::ORTHO_FRONT;
					CameraTransform.Rotation = nsQuaternion::IDENTITY;
					MainViewport->SetProjectionMode(true);
				}
			}
			else if (e.Key == nsEInputKey::KEYBOARD_NUMPAD_2)
			{
				if (ViewMode != cstEEditorViewMode::ORTHO_BACK)
				{
					NS_CONSOLE_Log(EditorLog, TEXT("Set camera view mode orthographic-back"));
					ViewMode = cstEEditorViewMode::ORTHO_BACK;
				}
			}
			else if (e.Key == nsEInputKey::KEYBOARD_NUMPAD_3)
			{
				if (ViewMode != cstEEditorViewMode::ORTHO_LEFT)
				{
					NS_CONSOLE_Log(EditorLog, TEXT("Set camera view mode orthographic-left"));
					ViewMode = cstEEditorViewMode::ORTHO_LEFT;
				}
			}
			else if (e.Key == nsEInputKey::KEYBOARD_NUMPAD_4)
			{
				if (ViewMode != cstEEditorViewMode::ORTHO_RIGHT)
				{
					NS_CONSOLE_Log(EditorLog, TEXT("Set camera view mode orthographic-right"));
					ViewMode = cstEEditorViewMode::ORTHO_RIGHT;
				}
			}
			else if (e.Key == nsEInputKey::KEYBOARD_NUMPAD_5)
			{
				if (ViewMode != cstEEditorViewMode::ORTHO_TOP)
				{
					NS_CONSOLE_Log(EditorLog, TEXT("Set camera view mode orthographic-top"));
					ViewMode = cstEEditorViewMode::ORTHO_TOP;
				}
			}
			else if (e.Key == nsEInputKey::KEYBOARD_NUMPAD_6)
			{
				if (ViewMode != cstEEditorViewMode::ORTHO_BOTTOM)
				{
					NS_CONSOLE_Log(EditorLog, TEXT("Set camera view mode orthographic-bottom"));
					ViewMode = cstEEditorViewMode::ORTHO_BOTTOM;
				}
			}
		}
	}
	else
	{
		if (e.Key == nsEInputKey::KEYBOARD_A || e.Key == nsEInputKey::KEYBOARD_D)
		{
			CameraMoveAxis.X = 0.0f;
		}

		if (e.Key == nsEInputKey::KEYBOARD_W || e.Key == nsEInputKey::KEYBOARD_S)
		{
			CameraMoveAxis.Z = 0.0f;
		}

		if (e.Key == nsEInputKey::KEYBOARD_SHIFT_LEFT && bKeyPressed_LeftShift)
		{
			bKeyPressed_LeftShift = false;
		}
	}
}


void cstEditor::SelectFocusActor(nsActor* newActor)
{
	if (newActor && newActor != FocusActor)
	{
		NS_CONSOLE_Debug(EditorLog, TEXT("Select actor [%s]"), *newActor->Name);
	}

	FocusActor = newActor;
	ActorInspector.SetInspectActor(FocusActor);
}


void cstEditor::BeginDragDropAsset(const nsAssetInfo& assetInfo)
{
	if (bIsDraggingAsset)
	{
		return;
	}

	DragDropAssetInfo = assetInfo;
	bIsDraggingAsset = true;
	bIsDraggingAssetSpawned = false;

	NS_CONSOLE_Debug(EditorLog, TEXT("Begin drag-drop asset [%s]"), *assetInfo.Name);
}


void cstEditor::AddMousePickingForActor(nsActor* actor)
{
	if (actor == nullptr)
	{
		return;
	}

	nsCollisionComponent* collisionComp = actor->GetComponent<nsCollisionComponent>();

	if (collisionComp == nullptr)
	{
		if (nsMeshComponent* meshComp = actor->GetComponent<nsMeshComponent>())
		{
			const nsSharedModelAsset& modelAsset = meshComp->GetModelAsset();

			if (modelAsset.IsValid())
			{
				const nsMeshID mesh0 = modelAsset.GetMeshes()[0];
				NS_Assert(mesh0 != nsMeshID::INVALID);

				nsConvexMeshCollisionComponent* convexMeshPicking = actor->AddComponent<nsConvexMeshCollisionComponent>("editor_mouse_picking");
				convexMeshPicking->SetMesh(mesh0);
				convexMeshPicking->SetSimulatePhysics(false);
				convexMeshPicking->SetCollisionTest(nsEPhysicsCollisionTest::QUERY_ONLY);
				convexMeshPicking->SetObjectChannel(nsEPhysicsCollisionChannel::MousePicking);
				convexMeshPicking->SetCollisionChannels(nsEPhysicsCollisionChannel::NONE);
			}
		}
	}
}


void cstEditor::MoveFocusActorDownToFloor()
{
	if (FocusActor == nullptr)
	{
		return;
	}

	nsCollisionComponent* collisionComp = FocusActor->GetComponent<nsCollisionComponent>();
	if (collisionComp == nullptr)
	{
		NS_CONSOLE_Warning(EditorLog, TEXT("Cannot move actor [%s] down to floor. No collision component!"), *FocusActor->Name);
		return;
	}

	nsTransform focusActorTransform = FocusActor->GetWorldTransform();

	nsPhysicsQueryParams queryParams;
	queryParams.IgnoredActors.Add(FocusActor);

	bool bFoundFloor = false;
	bool bAlreadyOnFloor = false;
	nsPhysicsHitResult hitResult;
	float distance = 100.0f;

	for (int i = 0; i < 10; ++i)
	{
		if (collisionComp->SweepTest(hitResult, -nsVector3::UP, distance, queryParams))
		{
			if (hitResult.Distance == 0.0f)
			{
				bAlreadyOnFloor = true;
				continue;
			}

			bFoundFloor = true;
			bAlreadyOnFloor = false;
			break;
		}

		distance += 100.0f;
	}
	
	if (bFoundFloor)
	{
		NS_CONSOLE_Log(EditorLog, TEXT("Move actor [%s] down to floor. [HitActor: %s, HitPosition: (%f, %f, %f), HitDistance: %f]"),
			*FocusActor->Name,
			*hitResult.Actor->Name,
			hitResult.WorldPosition.X, hitResult.WorldPosition.Y, hitResult.WorldPosition.Z,
			hitResult.Distance
		);

		focusActorTransform.Position.Y = hitResult.WorldPosition.Y;
		FocusActor->SetWorldTransform(focusActorTransform);
		collisionComp->AdjustPositionIfOverlappedWith(hitResult.Actor);
	}
	else if (!bAlreadyOnFloor)
	{
		NS_CONSOLE_Warning(EditorLog, TEXT("Cannot move actor [%s] down to floor. No hit found below!"), *FocusActor->Name);
	}
}


void cstEditor::TickUpdate(float deltaTime)
{
	NS_Assert(Game);
	NS_Assert(MainViewport);

	nsVector3 moveDirection;
	moveDirection += CameraTransform.GetAxisRight() * CameraMoveAxis.X;

	if (ViewMode == cstEEditorViewMode::PERSPECTIVE)
	{
		const nsPointInt windowDimension = Game->GetDimension();
		MainViewport->SetDimension(static_cast<float>(windowDimension.X), static_cast<float>(windowDimension.Y));
		
		CameraRotation.X = nsMath::Clamp(CameraRotation.X, -80.0f, 80.0f);
		CameraTransform.Rotation = nsQuaternion::FromRotation(CameraRotation.X, CameraRotation.Y, 0.0f);

		const nsVector3 forward = CameraTransform.GetAxisForward();
		moveDirection += (forward * CameraMoveAxis.Z);
		CameraTransform.Position += forward * CameraScrollValue * 5000.0f * deltaTime;
	}
	else
	{
		nsPointFloat viewportDimension = MainViewport->GetDimension();
		viewportDimension -= CameraScrollValue * 20.0f;
		MainViewport->SetDimension(viewportDimension.X, viewportDimension.Y);
	}

	moveDirection.Normalize();
	moveDirection *= 1000.0f * deltaTime;
	CameraTransform.Position += moveDirection;

	MainViewport->SetViewTransform(CameraTransform);

	CameraScrollValue = 0.0f;
}




void cstEditor::PreRender(nsRenderContextWorld& context)
{
	NS_Assert(Game);
	NS_Assert(MainViewport);

	MainRenderer->Viewport = *MainViewport;
	MainRenderer->RenderTargetDimension = Game->GetDimension();
	MainRenderer->RenderFinalTexture = nsERenderFinalTexture::SCENE_RENDER_TARGET;
	MainRenderer->RenderContextWorld = &nsRenderManager::Get().GetWorldRenderContext(MainWorld);
	MainRenderer->World = MainWorld;

	if (FocusActor)
	{
		const nsTransform transform = bIsLocalCoordSpace ? FocusActor->GetLocalTransform() : FocusActor->GetWorldTransform();
		ActorGizmo.Render(MainRenderer, MainViewport, transform, bIsLocalCoordSpace, true);
	}
}


void cstEditor::DrawGUI(nsGUIContext& context)
{
	const nsGUIRect canvasRect = context.GetCanvasRect();

	context.BeginRegion("editor_scene_viewport", nsGUIRect(0.0f, 0.0f, canvasRect.GetWidth(), canvasRect.GetHeight()), nsPointFloat(), nsEGUIElementLayout::NONE, nsEGUIScrollOption::None, false, "editor_scene_viewport");
	{
		bSceneViewportHovered = context.IsCurrentRegionHovered();
		bIsAnyControlFocused = context.IsAnyControlFocused();

		if (bShowAssetExplorer)
		{
			AssetExplorer.DrawGUI(context);
		}

		if (bShowActorInspector)
		{
			ActorInspector.DrawGUI(context);
		}

		if (bShowWorldOutliner)
		{
			nsActor* selectedActor = WorldOutliner.DrawGUI(context, MainWorld, FocusActor);
			SelectFocusActor(selectedActor);
		}

		if (bShowContextMenu && ContextMenu.DrawGUI(context, ContextMenuCoord))
		{
			ActorGizmo.SnapTranslationValue = ContextMenu.GetSnapTranslationValue();
			ActorGizmo.SnapRotationValue = ContextMenu.GetSnapRotationValue();
		}
	}
	context.EndRegion();

	/*
	static nsString _viewportInfoText;

	const nsPointFloat viewportDimension = MainViewport->GetDimension();
	_viewportInfoText = nsString::Format("ViewportDimension: %.3f, %.3f", viewportDimension.X, viewportDimension.Y);

	context.AddDrawText(*_viewportInfoText, _viewportInfoText.GetLength(), nsPointFloat(16.0f, canvasRect.GetHeight() - 100.0f));
	*/
}


cstEditor* g_Editor = nullptr;
