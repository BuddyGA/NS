#include "nsNavigationEditor.h"



nsNavigationEditor::nsNavigationEditor()
{
	Window.Name = "editor_navigation";
	Window.Title = "Navigation System";
	Window.InitialRect = nsGUIRect(16.0f, 16.0f, 200.0f, 200.0f);
	Window.bTitleBar = true;
	Window.bMoveable = true;
	Window.bResizable = false;
}


void nsNavigationEditor::DrawGUI(nsGUIContext& context)
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

			}
		}
		context.EndRegion();
	}
	Window.EndDraw(context);
}
