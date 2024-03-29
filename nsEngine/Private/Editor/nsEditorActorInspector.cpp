#include "Editor/nsEditorActorInspector.h"



// ================================================================================================================================================================================== //
// EDITOR - ACTOR INSPECTOR
// ================================================================================================================================================================================== //
nsEditorActorInspector::nsEditorActorInspector()
{
	InspectActor = nullptr;

	Window.Name = "editor_actor_inspector";
	Window.Title = "Actor Inspector";
	Window.bMoveable = false;
	Window.bResizable = false;

	InputPosition.Name = "inspect_actor_position";
	InputPosition.DragSpeed = 5.25f;

	InputRotation.Name = "inspect_actor_rotation";
	InputRotation.DragSpeed = 1.25f;

	InputScale.Name = "inspect_actor_scale";
	InputScale.DragSpeed = 0.025f;
	InputScale.MinValue = 0.01f;
	InputScale.MaxValue = 1000.0f;

	bIsLocalCoordSpace = true;
}


void nsEditorActorInspector::DrawGUI(nsGUIContext& context)
{
	const nsGUIRect& canvasRect = context.GetCanvasRect();

	nsGUIRect rect;
	rect.Left = canvasRect.Right - 280.0f;
	rect.Top = canvasRect.Top + canvasRect.GetHeight() * 0.5f;
	rect.Right = canvasRect.Right - 10.0f;
	rect.Bottom = canvasRect.Bottom - 10.0f;

	Window.SetRect(rect);

	Window.BeginDraw(context);
	{
		const nsGUIRect contentRect = Window.GetContentRect();

		context.BeginRegion("actor_properties", contentRect, nsPointFloat(2.0f, 4.0f), nsEGUIElementLayout::VERTICAL, nsEGUIScrollOption::Scrollable_Y, false, "actor_properties");

		if (InspectActor)
		{
			context.AddControlText(*InspectActor->Name);

			nsGUIControl control = context.AddControlRect(0.0f, 22.0f, nsColor(20, 30, 50, 255));

			if (control.bIsVisible)
			{
				context.AddDrawTextOnRect(TEXT("TRANSFORM"), 9, control.Rect, nsEGUIAlignmentHorizontal::LEFT, nsEGUIAlignmentVertical::CENTER);
			}

			bool bTransformChanged = false;

			context.AddControlText(TEXT("Position:"));

			if (InputPosition.Draw(context, CachePosition))
			{
				bTransformChanged = true;
			}

			context.AddControlText(TEXT("Rotation:"));

			if (InputRotation.Draw(context, CacheRotation))
			{
				CacheRotation = nsMath::ClampVector3Degrees(CacheRotation);
				bTransformChanged = true;
			}

			context.AddControlText(TEXT("Scale:"));

			if (InputScale.Draw(context, CacheScale))
			{
				bTransformChanged = true;
			}

			if (bTransformChanged)
			{
				//NS_Console_Debug(EditorLog, "Update transform [%s]", *actor->Name);

				if (bIsLocalCoordSpace)
				{
					InspectActor->SetLocalTransform(nsTransform(CachePosition, nsQuaternion::FromRotation(CacheRotation), CacheScale));
				}
				else
				{
					InspectActor->SetWorldTransform(nsTransform(CachePosition, nsQuaternion::FromRotation(CacheRotation), CacheScale));
				}
			}
		}

		context.EndRegion();
	}
	Window.EndDraw(context);
}
