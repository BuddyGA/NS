#pragma once

#include "nsNavigationTypes.h"



class NS_ENGINE_API nsNavigationManager
{
	NS_DECLARE_SINGLETON(nsNavigationManager)

private:
	bool bInitialized;
	bool bBuildingNavMesh;


public:
	void Initialize();
	void BuildNavMesh(nsWorld* world);
	void BuildNavMesh(const nsNavigationInputGeometry& inputGeometry, const nsNavigationBuildSettings& buildSettings);


	NS_NODISCARD_INLINE bool IsBuildingNavMesh() const
	{
		return bBuildingNavMesh;
	}



#ifdef NS_ENGINE_DEBUG_DRAW
public:
	void DebugDraw(class nsRenderer* renderer);
#endif // NS_ENGINE_DEBUG_DRAW

};
