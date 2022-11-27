#pragma once

#include "nsEditorAssetExplorer.h"
#include "nsEditorAssetImporter.h"
#include "nsEditorActorInspector.h"
#include "nsEditorTransformGizmo.h"
#include "nsEditorWorldOutliner.h"



class nsEditorContextMenu
{
private:
	nsGUIWindow Window;
	nsGUIInputFloat InputSnapTranslation;
	nsGUIInputFloat InputSnapRotation;
	nsGUIInputFloat InputSnapScale;


public:
	nsEditorContextMenu();
	bool DrawGUI(nsGUIContext& context, const nsPointFloat& screenCoord);


	NS_NODISCARD_INLINE float GetSnapTranslationValue() const
	{
		return InputSnapTranslation.FloatValue;
	}

	NS_NODISCARD_INLINE float GetSnapRotationValue() const
	{
		return InputSnapRotation.FloatValue;
	}

};




class NS_ENGINE_API nsEditor
{
private:
	class nsGameApplication* Game;
	nsEditorAssetExplorer AssetExplorer;
	nsEditorAssetImporter AssetImporter;
	nsEditorActorInspector ActorInspector;
	nsEditorWorldOutliner WorldOutliner;
	nsEditorGizmoTransform ActorGizmo;
	nsEditorContextMenu ContextMenu;

	nsEEditorViewMode ViewMode;
	nsPointFloat MouseCoord;
	nsPointFloat ContextMenuCoord;
	nsActor* FocusActor;
	nsAssetInfo DragDropAssetInfo;
	bool bShowAssetExplorer;
	bool bShowActorInspector;
	bool bShowWorldOutliner;
	bool bShowContextMenu;
	bool bIsLocalCoordSpace;
	bool bSceneViewportHovered;
	bool bIsFocusingOnGUI;
	bool bIsDraggingAsset;
	bool bIsDraggingAssetSpawned;
	bool bKeyPressed_LeftShift;

	nsTransform CameraTransform;
	nsVector3 CameraMoveAxis;
	nsVector3 CameraRotation;
	float CameraScrollValue;
	bool bMovingCamera;

public:
	nsWorld* MainWorld;
	nsViewport* MainViewport;
	nsRenderer* MainRenderer;


public:
	nsEditor(nsGameApplication* game);
	void OnMouseMove(const nsMouseMoveEventArgs& e);
	void OnMouseButton(const nsMouseButtonEventArgs& e);
	void OnMouseWheel(const nsMouseWheelEventArgs& e);
	void OnKeyboardButton(const nsKeyboardButtonEventArgs& e);

	void SelectFocusActor(nsActor* newActor);
	void BeginDragDropAsset(const nsAssetInfo& assetInfo);
	void AddMousePickingForActor(nsActor* actor);
	void MoveFocusActorDownToFloor();
	void ImportingAsset(const nsString& sourceFile);

	void TickUpdate(float deltaTime);
	void PreRender(nsRenderContextWorld& context);
	void DrawGUI(nsGUIContext& context);

};


extern NS_ENGINE_API nsEditor* g_Editor;
