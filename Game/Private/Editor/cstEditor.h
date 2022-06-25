#pragma once

#include "cstEditorAssetExplorer.h"
#include "cstEditorActorInspector.h"
#include "cstEditorWorldOutliner.h"
#include "cstEditorGizmo.h"


class nsRenderer;



class cstEditorContextMenu
{
private:
	nsGUIWindow Window;
	nsGUIInputFloat InputSnapTranslation;
	nsGUIInputFloat InputSnapRotation;
	nsGUIInputFloat InputSnapScale;


public:
	cstEditorContextMenu();
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




class cstEditor
{
private:
	cstGame* Game;
	cstEditorAssetExplorer AssetExplorer;
	cstEditorActorInspector ActorInspector;
	cstEditorWorldOutliner WorldOutliner;
	cstEditorGizmoTransform ActorGizmo;
	cstEditorContextMenu ContextMenu;

	cstEEditorViewMode ViewMode;
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
	bool bIsAnyControlFocused;
	bool bIsDraggingAsset;
	bool bIsDraggingAssetSpawned;
	bool bKeyPressed_LeftShift;

	nsTransform CameraTransform;
	nsVector3 CameraMoveAxis;
	nsVector2 CameraRotation;
	float CameraScrollValue;
	bool bMovingCamera;

public:
	nsWorld* MainWorld;
	nsViewport* MainViewport;
	nsRenderer* MainRenderer;


public:
	cstEditor(cstGame* game);
	void OnMouseMove(const nsMouseMoveEventArgs& e);
	void OnMouseButton(const nsMouseButtonEventArgs& e);
	void OnMouseWheel(const nsMouseWheelEventArgs& e);
	void OnKeyboardButton(const nsKeyboardButtonEventArgs& e);

	void SelectFocusActor(nsActor* newActor);
	void BeginDragDropAsset(const nsAssetInfo& assetInfo);
	void AddMousePickingForActor(nsActor* actor);
	void MoveFocusActorDownToFloor();

	void TickUpdate(float deltaTime);
	void PreRender(nsRenderContextWorld& context);
	void DrawGUI(nsGUIContext& context);

};


extern cstEditor* g_Editor;
