#pragma once

#include "nsNavigationTypes.h"
#include "nsWorld.h"
#include "nsGUIFramework.h"



class NS_ENGINE_API nsNavigationVolumeActor : public nsActor
{
	NS_DECLARE_OBJECT(nsNavigationVolumeActor)

public:
	nsNavigationVolumeActor();
	NS_NODISCARD nsAABB GetScaledBoundingBox();

};




class NS_ENGINE_API nsEditorNavigationBuilder
{
private:
	nsGUIWindow Window;

public:
	nsWorld* World;


public:
	nsEditorNavigationBuilder();
	void DrawGUI(nsGUIContext& context);

private:
	void BuildNavMesh();

};
