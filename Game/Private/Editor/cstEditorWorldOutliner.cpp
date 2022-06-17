#include "cstEditorWorldOutliner.h"




// ================================================================================================================================================================================== //
// EDITOR - WORLD OUTLINER
// ================================================================================================================================================================================== //
cstEditorWorldOutliner::cstEditorWorldOutliner()
{
	Window.Name = "editor_world_outliner";
	Window.Title = "World Outliner";
	Window.bMoveable = false;
	Window.bResizable = false;
}


nsActor* cstEditorWorldOutliner::DrawGUI(nsGUIContext& context, nsWorld* world, nsActor* focusActor)
{
	nsActor* selectedActor = focusActor;

	const nsGUIRect& canvasRect = context.GetCanvasRect();

	nsGUIRect rect;
	rect.Left = canvasRect.Right - 280.0f;
	rect.Top = canvasRect.Top + canvasRect.GetHeight() * 0.5f;
	rect.Right = canvasRect.Right - 10.0f;
	rect.Bottom = canvasRect.Bottom - 10.0f;

	Window.SetRect(rect);

	Window.BeginDraw(context);
	{
		static const char* _idActorList = "actor_list";

		const nsGUIRect contentRect = Window.GetContentRect();
		context.BeginRegion(_idActorList, contentRect, nsPointFloat(4.0f), nsEGUIElementLayout::VERTICAL, nsEGUIScrollOption::Scrollable_Y, false, "world_outliner_actor_list");

		if (world)
		{
			const nsTArray<nsActor*>& actorList = world->GetAllActors();

			for (int i = 0; i < actorList.GetCount(); ++i)
			{
				nsActor* actor = actorList[i];
				const nsGUIControl control = context.AddControlRect(0.0f, 16.0f, (actor == selectedActor) ? nsColor::GRAY : nsColor::BLACK_TRANSPARENT);

				if (control.bIsVisible)
				{
					const nsName& name = actor->Name;
					context.AddDrawTextOnRect(*name, name.GetLength(), control.Rect, nsEGUIAlignmentHorizontal::LEFT, nsEGUIAlignmentVertical::CENTER);

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
