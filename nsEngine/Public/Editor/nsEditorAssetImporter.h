#pragma once

#include "nsEditorTypes.h"



class nsEditorAssetImporter
{
public:
	nsString SourceFile;

private:
	bool bImportingModelAsset;
	bool bImportingTextureAsset;

	nsGUIWindow Window;


public:
	nsEditorAssetImporter();
	void ImportAssetFromFile(const nsString& sourceFile);
	void DrawGUI(nsGUIContext& context);

};
