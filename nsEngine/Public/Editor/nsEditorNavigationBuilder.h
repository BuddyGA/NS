#pragma once

#include "nsNavigationTypes.h"
#include "nsWorld.h"
#include "nsGUIFramework.h"



class NS_ENGINE_API nsNavigationVolumeActor : public nsActor
{
	NS_DECLARE_OBJECT()

public:
	nsNavigationVolumeActor();
	NS_NODISCARD nsAABB GetScaledBoundingBox();

};




class NS_ENGINE_API nsNavigationEditor
{
private:
	nsGUIWindow Window;

public:
	nsWorld* World;


public:
	nsNavigationEditor();
	void DrawGUI(nsGUIContext& context);

private:
	void BuildNavMesh();

};
