#include "Editor/nsEditorActorAsset.h"



nsEditorActorAsset::nsEditorActorAsset()
{
	Window.Name = "editor_actor_asset";
	Window.InitialRect = nsGUIRect(0.0f, 0.0f, 256.0f, 256.0f);
	Window.Title = "Actor Asset";
}


void nsEditorActorAsset::DrawGUI(nsGUIContext& context)
{
	Window.BeginDraw(context);
	{

	}
	Window.EndDraw(context);
}
