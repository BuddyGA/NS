#include "Editor/nsEditorAssetExplorer.h"
#include "Editor/nsEditor.h"
#include "nsFileSystem.h"
#include "nsAssetManager.h"
#include "nsConsole.h"



// ================================================================================================================================================================================== //
// EDITOR - ASSET EXPLORER
// ================================================================================================================================================================================== //
nsEditorAssetExplorer::nsEditorAssetExplorer()
{
	SelectedAssetFolder = nullptr;
	SelectedAsset = nullptr;

	Window.Name = "editor_asset_explorer";
	Window.Title = "Asset Explorer";
	Window.bMoveable = false;
	Window.bResizable = false;

	Table.AddColumn("", 0.5f);
	Table.AddColumn("", 0.5f);
	Table.SeparatorColor = nsColor::GRAY;
}


void nsEditorAssetExplorer::ScanAssetsInFolder(nsEditorAssetFolder& assetFolder)
{
	NS_Assert(!assetFolder.Path.IsEmpty());

	nsTArray<nsString> tempStringArray;

	assetFolder.Subfolders.Clear();
	nsFileSystem::FolderIterate(tempStringArray, assetFolder.Path, false);
	assetFolder.Subfolders.Resize(tempStringArray.GetCount());

	for (int i = 0; i < tempStringArray.GetCount(); ++i)
	{
		nsEditorAssetFolder& assetSubfolder = assetFolder.Subfolders[i];
		assetSubfolder.Path = tempStringArray[i];
		assetSubfolder.Name = *nsFileSystem::FileGetName(assetSubfolder.Path);
		ScanAssetsInFolder(assetSubfolder);
	}

	assetFolder.AssetInfos.Clear();
	nsAssetManager::Get().GetAssetInfosFromPath(assetFolder.Path, assetFolder.AssetInfos);
}


void nsEditorAssetExplorer::ScanAssets()
{
	nsAssetManager& assetManager = nsAssetManager::Get();

	EngineAssetFolder.Path = assetManager.GetEngineAssetsPath();
	EngineAssetFolder.Name = *nsFileSystem::FileGetName(EngineAssetFolder.Path);
	ScanAssetsInFolder(EngineAssetFolder);

	GameAssetFolder.Path = assetManager.GetGameAssetsPath();
	GameAssetFolder.Name = *nsFileSystem::FileGetName(GameAssetFolder.Path);
	ScanAssetsInFolder(GameAssetFolder);
}


void nsEditorAssetExplorer::DrawAssetFolder(nsGUIContext& context, float indent, const nsEditorAssetFolder& assetFolder)
{
	nsGUIControl control;
	control.Rect = nsGUIRect(indent, 0.0f, 0.0f, 16);
	context.UpdateControlInCurrentRegion(control, false);

	if (control.bIsVisible)
	{
		context.AddDrawRect(control.Rect, SelectedAssetFolder == &assetFolder ? nsColor::GRAY : nsColor::BLACK_TRANSPARENT);
		context.AddDrawTextOnRect(*assetFolder.Name.ToString(), assetFolder.Name.GetLength(), control.Rect, nsEGUIAlignmentHorizontal::LEFT, nsEGUIAlignmentVertical::CENTER);

		if (control.Interactions & nsEGUIRectInteraction::Pressed)
		{
			SelectedAssetFolder = &assetFolder;
		}

	#ifdef NS_ENGINE_DEBUG_DRAW
		if (context.bDrawDebugRect)
		{
			context.AddDrawDebugRectLine(control.Rect, nsColor::GREEN);
		}

		if (context.bDrawDebugHoveredRect && context.IsCurrentRegionHovered() && (control.Interactions & nsEGUIRectInteraction::Hovered))
		{
			context.AddDrawDebugRectLine(control.Rect, nsColor::WHITE);
		}
	#endif // NS_ENGINE_DEBUG_DRAW
	}

	const int subfolderCount = assetFolder.Subfolders.GetCount();

	if (subfolderCount > 0)
	{
		for (int i = 0; i < subfolderCount; ++i)
		{
			const nsEditorAssetFolder& subfolder = assetFolder.Subfolders[i];
			DrawAssetFolder(context, indent + 8.0f, subfolder);
		}
	}
}


void nsEditorAssetExplorer::DrawAssetInfo(nsGUIContext& context, const nsAssetInfo& assetInfo)
{
	const nsGUIControl control = context.AddControlRect(0.0f, 16.0f, SelectedAsset == &assetInfo ? nsColor::GRAY : nsColor::BLACK_TRANSPARENT);

	if (control.bIsVisible)
	{
		if (control.Interactions & nsEGUIRectInteraction::Pressed)
		{
			SelectedAsset = &assetInfo;
		}

		if (SelectedAsset && SelectedAsset == &assetInfo && (control.Interactions & nsEGUIRectInteraction::Hovered) && (context.IsMouseDragging(0)))
		{
			g_Editor->BeginDragDropAsset(assetInfo);
		}

		context.AddDrawTextOnRect(*assetInfo.Name.ToString(), assetInfo.Name.GetLength(), control.Rect, nsEGUIAlignmentHorizontal::LEFT, nsEGUIAlignmentVertical::CENTER);

	#ifdef NS_ENGINE_DEBUG_DRAW
		if (context.bDrawDebugRect)
		{
			context.AddDrawDebugRectLine(control.Rect, nsColor::GREEN);
		}

		if (context.bDrawDebugHoveredRect && context.IsCurrentRegionHovered() && (control.Interactions & nsEGUIRectInteraction::Hovered))
		{
			context.AddDrawDebugRectLine(control.Rect, nsColor::WHITE);
		}
	#endif // NS_ENGINE_DEBUG_DRAW
	}
}


void nsEditorAssetExplorer::DrawGUI(nsGUIContext& context)
{
	bool bOpenImportAssetFileDialog = false;

	const nsGUIRect& canvasRect = context.GetCanvasRect();

	nsGUIRect rect;
	rect.Left = 8.0f;
	rect.Top = 22.0f;
	rect.Right = canvasRect.Left + 400.0f;
	rect.Bottom = rect.Top + 400.0f;

	Window.SetRect(rect);

	Window.BeginDraw(context);
	{
		const nsGUIRect contentRect = Window.GetContentRect();
		context.BeginRegion(nullptr, contentRect, nsPointFloat(2.0f), nsEGUIElementLayout::VERTICAL, nsEGUIScrollOption::None, false, "asset_expl_content_region");
		{
			// Import asset button
			nsGUIButton importButton;
			importButton.DefaultColor = nsColor(50, 150, 50);
			importButton.HoveredColor = nsColor(80, 200, 80);
			importButton.PressedColor = importButton.DefaultColor;
			importButton.Text = "Import Asset";

			if (importButton.Draw(context))
			{
				// Can only open file dialog if we're not importing asset
				if (ImportAssetSourceFile.IsEmpty())
				{
					ImportAssetSourceFile = nsFileSystem::OpenFileDialog_ImportAsset();
				}

				if (!ImportAssetSourceFile.IsEmpty())
				{
					NS_CONSOLE_Log(nsEditorLog, TEXT("Importing asset from source file [%s]"), *ImportAssetSourceFile);
				}
			}

			// Separator
			context.AddControlRect(0.0f, 2.0f, nsColor::GRAY);

			// Region assets
			Table.Size = nsPointFloat(0.0f);
			Table.BeginDraw(context);
			{
				Table.BeginColumn(context, 0, nsPointFloat(4.0f));
				{
					DrawAssetFolder(context, 0.0f, EngineAssetFolder);
					DrawAssetFolder(context, 0.0f, GameAssetFolder);
				}
				Table.EndColumn(context);

				Table.BeginColumn(context, 1, nsPointFloat(4.0f));
				{
					if (SelectedAssetFolder)
					{
						for (int i = 0; i < SelectedAssetFolder->AssetInfos.GetCount(); ++i)
						{
							DrawAssetInfo(context, SelectedAssetFolder->AssetInfos[i]);
						}
					}
				}
				Table.EndColumn(context);
			}
			Table.EndDraw(context);
		}
		context.EndRegion();
	}
	Window.EndDraw(context);
}
