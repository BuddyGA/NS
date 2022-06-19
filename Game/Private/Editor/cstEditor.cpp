#include "cstEditor.h"
#include "cstGame.h"
#include "nsRenderManager.h"



nsLogCategory EditorLog("cstEditorLog", nsELogVerbosity::LV_DEBUG);




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


void cstEditorContextMenu::DrawGUI(nsGUIContext& context, const nsPointFloat& screenCoord)
{
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
			context.AddControlText("Snap Position:");
			InputSnapTranslation.Draw(context);

			context.AddControlText("Snap Rotation:");
			InputSnapRotation.Draw(context);

			context.AddControlText("Snap Scale:");
			InputSnapScale.Draw(context);
		}
		context.EndRegion();
	}
	Window.EndDraw(context);
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
				NS_CONSOLE_Log(EditorLog, "Spawn actor from asset [%s]", *DragDropAssetInfo.Name);

				nsActor* newActor = MainWorld->CreateActor(DragDropAssetInfo.Name, projectedPosition);
				{
					nsMeshComponent* meshComp = newActor->AddComponent<nsMeshComponent>("mesh");
					meshComp->SetMesh(nsAssetManager::Get().LoadModelAsset(DragDropAssetInfo.Name));

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
	if (e.ButtonState == nsEButtonState::PRESSED)
	{
		if (bSceneViewportHovered)
		{
			if (e.Key == nsEInputKey::MOUSE_LEFT)
			{
				if (MainViewport && FocusActor)
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
				NS_CONSOLE_Debug(EditorLog, "End drag-drop asset");
				bIsDraggingAsset = false;
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
		if (e.Key == nsEInputKey::KEYBOARD_ESCAPE)
		{
			if (!bIsAnyControlFocused)
			{
				SelectFocusActor(nullptr);
			}
		}
		else if (e.Key == nsEInputKey::KEYBOARD_F1)
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
		else if (e.Key == nsEInputKey::KEYBOARD_F5)
		{
			if (MainRenderer)
			{
				MainRenderer->bIsWireframe = !MainRenderer->bIsWireframe;
			}
		}

		if (bSceneViewportHovered)
		{
			if (bMovingCamera)
			{
				if (e.Key == nsEInputKey::KEYBOARD_A) CameraMoveAxis.X = -1.0f;
				else if (e.Key == nsEInputKey::KEYBOARD_D) CameraMoveAxis.X = 1.0f;

				if (e.Key == nsEInputKey::KEYBOARD_W) CameraMoveAxis.Z = 1.0f;
				else if (e.Key == nsEInputKey::KEYBOARD_S) CameraMoveAxis.Z = -1.0f;
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
					NS_CONSOLE_Debug(EditorLog, "Toggle gizmo options!");
					ContextMenuCoord = MouseCoord;
					bShowContextMenu = !bShowContextMenu;
				}
			}
		}

		if (!bIsAnyControlFocused)
		{
			if (e.Key == nsEInputKey::KEYBOARD_DELETE && MainWorld && FocusActor)
			{
				MainWorld->DestroyActor(FocusActor);
				SelectFocusActor(nullptr);
			}
			else if (e.Key == nsEInputKey::KEYBOARD_NUMPAD_0)
			{
				if (ViewMode != cstEEditorViewMode::PERSPECTIVE)
				{
					NS_CONSOLE_Log(EditorLog, "Set camera view mode perspective");
					ViewMode = cstEEditorViewMode::PERSPECTIVE;
					MainViewport->SetProjectionMode(false);
				}
			}
			else if (e.Key == nsEInputKey::KEYBOARD_NUMPAD_1)
			{
				if (ViewMode != cstEEditorViewMode::ORTHO_FRONT)
				{
					NS_CONSOLE_Log(EditorLog, "Set camera view mode orthographic-front");
					ViewMode = cstEEditorViewMode::ORTHO_FRONT;
					CameraTransform.Rotation = nsQuaternion::IDENTITY;
					MainViewport->SetProjectionMode(true);
				}
			}
			else if (e.Key == nsEInputKey::KEYBOARD_NUMPAD_2)
			{
				if (ViewMode != cstEEditorViewMode::ORTHO_BACK)
				{
					NS_CONSOLE_Log(EditorLog, "Set camera view mode orthographic-back");
					ViewMode = cstEEditorViewMode::ORTHO_BACK;
				}
			}
			else if (e.Key == nsEInputKey::KEYBOARD_NUMPAD_3)
			{
				if (ViewMode != cstEEditorViewMode::ORTHO_LEFT)
				{
					NS_CONSOLE_Log(EditorLog, "Set camera view mode orthographic-left");
					ViewMode = cstEEditorViewMode::ORTHO_LEFT;
				}
			}
			else if (e.Key == nsEInputKey::KEYBOARD_NUMPAD_4)
			{
				if (ViewMode != cstEEditorViewMode::ORTHO_RIGHT)
				{
					NS_CONSOLE_Log(EditorLog, "Set camera view mode orthographic-right");
					ViewMode = cstEEditorViewMode::ORTHO_RIGHT;
				}
			}
			else if (e.Key == nsEInputKey::KEYBOARD_NUMPAD_5)
			{
				if (ViewMode != cstEEditorViewMode::ORTHO_TOP)
				{
					NS_CONSOLE_Log(EditorLog, "Set camera view mode orthographic-top");
					ViewMode = cstEEditorViewMode::ORTHO_TOP;
				}
			}
			else if (e.Key == nsEInputKey::KEYBOARD_NUMPAD_6)
			{
				if (ViewMode != cstEEditorViewMode::ORTHO_BOTTOM)
				{
					NS_CONSOLE_Log(EditorLog, "Set camera view mode orthographic-bottom");
					ViewMode = cstEEditorViewMode::ORTHO_BOTTOM;
				}
			}
		}
	}
	else
	{
		if (e.Key == nsEInputKey::KEYBOARD_A || e.Key == nsEInputKey::KEYBOARD_D) CameraMoveAxis.X = 0.0f;
		if (e.Key == nsEInputKey::KEYBOARD_W || e.Key == nsEInputKey::KEYBOARD_S) CameraMoveAxis.Z = 0.0f;

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
		NS_CONSOLE_Debug(EditorLog, "Select actor [%s]", *newActor->Name);
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

	NS_CONSOLE_Debug(EditorLog, "Begin drag-drop asset [%s]", *assetInfo.Name);
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

	if (FocusActor)
	{
		const nsTransform transform = bIsLocalCoordSpace ? FocusActor->GetLocalTransform() : FocusActor->GetWorldTransform();
		ActorGizmo.Render(context, MainViewport, transform, bIsLocalCoordSpace, true);
	}

	MainRenderer->Viewport = *MainViewport;
	MainRenderer->RenderTargetDimension = Game->GetDimension();
	MainRenderer->RenderFinalTexture = nsERenderFinalTexture::SCENE_RENDER_TARGET;
	MainRenderer->RenderContextWorld = &nsRenderManager::Get().GetWorldRenderContext(MainWorld);
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

		if (bShowContextMenu)
		{
			ContextMenu.DrawGUI(context, ContextMenuCoord);

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
