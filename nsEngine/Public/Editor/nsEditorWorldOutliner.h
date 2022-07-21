#pragma once

#include "nsEditorTypes.h"



class NS_ENGINE_API nsEditorWorldOutliner
{
private:
	nsGUIWindow Window;

public:
	nsEditorWorldOutliner();
	nsActor* DrawGUI(nsGUIContext& context, nsWorld* world, nsActor* focusActor);

};
