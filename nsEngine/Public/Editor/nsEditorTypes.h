#pragma once

#include "nsRenderer.h"
#include "nsGUIFramework.h"
#include "nsAssetTypes.h"
#include "nsActor.h"



extern nsLogCategory nsEditorLog;



enum class nsEEditorCoordinateSpace : uint8
{
	LOCAL = 0,
	WORLD
};



enum class nsEEditorViewMode : uint8
{
	PERSPECTIVE = 0,
	ORTHO_FRONT,
	ORTHO_BACK,
	ORTHO_LEFT,
	ORTHO_RIGHT,
	ORTHO_TOP,
	ORTHO_BOTTOM
};



enum class nsEEditorDragDropType : uint8
{
	NONE = 0,
	ASSET,
	NAVMESH_VOLUME
};



struct nsEditorAssetFolder
{
	nsString Path;
	nsName Name;
	nsTArray<nsEditorAssetFolder> Subfolders;
	nsTArray<nsAssetInfo> AssetInfos;
};
