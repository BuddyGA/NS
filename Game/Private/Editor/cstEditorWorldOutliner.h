#pragma once

#include "cstEditorTypes.h"



class cstEditorWorldOutliner
{
private:
	nsGUIWindow Window;

public:
	cstEditorWorldOutliner();
	nsActor* DrawGUI(nsGUIContext& context, nsWorld* world, nsActor* focusActor);

};
