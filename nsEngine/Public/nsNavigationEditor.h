#pragma once

#include "nsNavigationTypes.h"
#include "nsGUIFramework.h"



class NS_ENGINE_API nsNavigationEditor
{
private:
	nsGUIWindow Window;


public:
	nsNavigationEditor();
	void DrawGUI(nsGUIContext& context);

};
