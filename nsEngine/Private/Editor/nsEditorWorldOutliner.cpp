#include "Editor/nsEditorWorldOutliner.h"
#include "nsWorld.h"



// ================================================================================================================================================================================== //
// EDITOR - WORLD OUTLINER
// ================================================================================================================================================================================== //
nsEditorWorldOutliner::nsEditorWorldOutliner()
{
	Window.Name = "editor_world_outliner";
	Window.Title = "World Outliner";
	Window.bMoveable = false;
	Window.bResizable = false;
}


nsActor* nsEditorWorldOutliner::DrawGUI(nsGUIContext& context, nsWorld* world, nsActor* focusActor)
{
	nsActor* selectedActor = focusActor;

	const nsGUIRect& canvasRect = context.GetCanvasRect();

	nsGUIRect rect;
	rect.Left = canvasRect.Right - 280.0f;
	rect.Top = 22.0f;
	rect.Right = canvasRect.Right - 10.0f;
	rect.Bottom = canvasRect.Top + canvasRect.GetHeight() * 0.49f;

	Window.SetRect(rect);

	Window.BeginDraw(context);
	{
		const nsGUIRect contentRect = Window.GetContentRect();
		context.BeginRegion("actor_list", contentRect, nsPointFloat(4.0f), nsEGUIElementLayout::VERTICAL, nsEGUIScrollOption::Scrollable_Y, false, "world_outliner_actor_list");

		if (world)
		{
			const nsTArray<nsActor*>& actorList = world->GetAllActors();

			for (int i = 0; i < actorList.GetCount(); ++i)
			{
				nsActor* actor = actorList[i];
				const nsGUIControl control = context.AddControlRect(0.0f, 16.0f, (actor == selectedActor) ? nsColor::GRAY : nsColor::BLACK_TRANSPARENT);

				if (control.bIsVisible)
				{
					context.AddDrawTextOnRect(*actor->Name, actor->Name.GetLength(), control.Rect, nsEGUIAlignmentHorizontal::LEFT, nsEGUIAlignmentVertical::CENTER);

					if (control.Interactions & nsEGUIRectInteraction::Pressed)
					{
						selectedActor = actor;
					}
				}
			}
		}

		context.EndRegion();
	}
	Window.EndDraw(context);

	return selectedActor;
}
