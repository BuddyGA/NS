#pragma once

#include "cstEditorTypes.h"



class cstEditorAssetExplorer
{
private:
	cstEditorAssetFolder EngineAssetFolder;
	cstEditorAssetFolder GameAssetFolder;
	const cstEditorAssetFolder* SelectedAssetFolder;
	const nsAssetInfo* SelectedAsset;
	nsString ImportAssetSourceFile;


public:
	cstEditorAssetExplorer();
	
private:
	void ScanAssetsInFolder(cstEditorAssetFolder& assetFolder);

public:
	void ScanAssets();


private:
	nsGUIWindow Window;
	nsGUITable Table;

private:
	void DrawAssetFolder(nsGUIContext& context, float indent, const cstEditorAssetFolder& assetFolder);
	void DrawAssetInfo(nsGUIContext& context, const nsAssetInfo& assetInfo);

public:
	void DrawGUI(nsGUIContext& context);

};
