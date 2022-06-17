#pragma once

#include "nsViewport.h"
#include "nsRenderContextWorld.h"
#include "nsGUIFramework.h"
#include "nsConsole.h"
#include "nsWorld.h"
#include "nsAssetManager.h"



extern nsLogCategory EditorLog;



enum class cstEEditorCoordinateSpace : uint8
{
	LOCAL = 0,
	WORLD
};



enum class cstEEditorViewMode : uint8
{
	PERSPECTIVE = 0,
	ORTHO_FRONT,
	ORTHO_BACK,
	ORTHO_LEFT,
	ORTHO_RIGHT,
	ORTHO_TOP,
	ORTHO_BOTTOM
};



struct cstEditorAssetFolder
{
	nsString Path;
	nsName Name;
	nsTArray<cstEditorAssetFolder> Subfolders;
	nsTArray<nsAssetInfo> AssetInfos;
};



class cstGame;
