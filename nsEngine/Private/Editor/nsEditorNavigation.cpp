#include "Editor/nsEditorNavigation.h"



NS_CLASS_BEGIN(nsNavigationVolumeActor, nsActor)
NS_CLASS_END(nsNavigationVolumeActor)

nsNavigationVolumeActor::nsNavigationVolumeActor()
{
	Flags |= nsEActorFlag::EditorOnly;
}


nsAABB nsNavigationVolumeActor::GetScaledBoundingBox()
{
	const nsTransform worldTransform = GetWorldTransform();
	
	nsAABB aabb;
	aabb.Min = worldTransform.Position - (nsVector3(-256.0f) * worldTransform.Scale);
	aabb.Max = worldTransform.Position + (nsVector3(356.0f) * worldTransform.Scale);

	return aabb;
}




nsEditorNavigationBuilder::nsEditorNavigationBuilder()
{
	Window.Name = "editor_navigation";
	Window.Title = "Navigation System";
	Window.InitialRect = nsGUIRect(16.0f, 16.0f, 200.0f, 200.0f);
	Window.bTitleBar = true;
	Window.bMoveable = true;
	Window.bResizable = false;

	World = nullptr;
}


void nsEditorNavigationBuilder::DrawGUI(nsGUIContext& context)
{
	Window.BeginDraw(context);
	{
		const nsGUIRect contentRect = Window.GetContentRect();
		context.BeginRegion(nullptr, contentRect, nsPointFloat(2.0f), nsEGUIElementLayout::VERTICAL, nsEGUIScrollOption::None, false);
		{
			
			nsGUIButton button;
			button.Text = "Build";
			button.Size = nsPointFloat(0.0f, 22.0f);
			
			if (button.Draw(context))
			{
				BuildNavMesh();
			}
		}
		context.EndRegion();
	}
	Window.EndDraw(context);
}


void nsEditorNavigationBuilder::BuildNavMesh()
{

}
