#pragma once

#include "nsEditorTypes.h"



class nsEditorAssetExplorer
{
private:
	nsEditorAssetFolder EngineAssetFolder;
	nsEditorAssetFolder GameAssetFolder;
	const nsEditorAssetFolder* SelectedAssetFolder;
	const nsAssetInfo* SelectedAsset;


public:
	nsEditorAssetExplorer();

private:
	void ScanAssetsInFolder(nsEditorAssetFolder& assetFolder);

public:
	void ScanAssets();

private:
	nsGUIWindow Window;
	nsGUITable Table;


private:
	void DrawAssetFolder(nsGUIContext& context, float indent, const nsEditorAssetFolder& assetFolder);
	void DrawAssetInfo(nsGUIContext& context, const nsAssetInfo& assetInfo);

public:
	void DrawGUI(nsGUIContext& context);

};
