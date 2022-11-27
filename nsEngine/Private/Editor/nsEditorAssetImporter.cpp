#include "Editor/nsEditorAssetImporter.h"



nsEditorAssetImporter::nsEditorAssetImporter()
{
	bImportingModelAsset = false;
	bImportingTextureAsset = false;

	Window.Name = "editor_asset_importer";
	Window.bMoveable = true;
	Window.bResizable = true;
}


void nsEditorAssetImporter::ImportAssetFromFile(const nsString& sourceFile)
{
	SourceFile = sourceFile;

	const nsString fileExt = nsFileSystem::FileGetExtension(SourceFile);

	if (fileExt == TEXT(".glb"))
	{
		NS_CONSOLE_Log(nsEditorLog, TEXT("Importing model asset from source file [%s]"), *SourceFile);
		bImportingModelAsset = true;
	}
	else if (fileExt == TEXT(".bmp") || fileExt == TEXT(".png") || fileExt == TEXT(".tga"))
	{
		NS_CONSOLE_Log(nsEditorLog, TEXT("Importing texture asset from source file [%s]"), *SourceFile);
		bImportingTextureAsset = true;
	}
}


void nsEditorAssetImporter::DrawGUI(nsGUIContext& context)
{
	if (SourceFile.IsEmpty())
	{
		return;
	}

	if (bImportingModelAsset)
	{
		Window.Title = "Import Model";
	}
	else if (bImportingTextureAsset)
	{
		Window.Title = "Import Texture";
	}

	const nsGUIRect canvasRect = context.GetCanvasRect();

	nsGUIRect rect;
	rect.Left = canvasRect.Right / 2.0f;
	rect.Top = canvasRect.Bottom / 2.0f;
	rect.Right = rect.Left + 256.0f;
	rect.Bottom = rect.Top + 256.0f;

	//Window.SetRect(rect);

	Window.BeginDraw(context);
	{
		const nsGUIRect contentRect = Window.GetContentRect();
		context.BeginRegion(nullptr, contentRect, nsPointFloat(2.0f), nsEGUIElementLayout::VERTICAL, nsEGUIScrollOption::None, true, "ed_asset_importer");
		{
			nsGUIButton cancelButton;
			cancelButton.Text = "Cancel";

			if (cancelButton.Draw(context))
			{
				SourceFile.Clear();
			}
		}
		context.EndRegion();
	}
	Window.EndDraw(context);
}
