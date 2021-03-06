#pragma once

#include "nsNavigationTypes.h"



class NS_ENGINE_API nsNavigationManager
{
	NS_DECLARE_SINGLETON(nsNavigationManager)

private:
	bool bInitialized;

	class dtCrowd* DetourCrowd;
	class dtNavMesh* DetourNavMesh;
	class dtNavMeshQuery* DetourNavMeshQuery;


	struct NavAgentState
	{
		nsVector3 MoveTargetPosition;
		int AgentIndex;
	};

	nsTArrayFreeList<NavAgentState> AgentStates;


public:
	void Initialize();
	void BuildNavMesh(nsWorld* world);
	void BuildNavMesh(const nsNavigationInputGeometry& inputGeometry, const nsNavigationBuildSettings& buildSettings);

	NS_NODISCARD nsNavigationAgentID CreateAgent(nsNavigationAgentComponent* component);
	void DestroyAgent(nsNavigationAgentID& agent);
	void UpdateAgentParams(nsNavigationAgentID agent);
	void SetAgentMoveTarget(nsNavigationAgentID agent, const nsVector3& targetPosition);
	void StopAgentMovement(nsNavigationAgentID agent);
	void MoveAgents(float deltaTime);

	NS_NODISCARD_INLINE bool IsAgentValid(nsNavigationAgentID agent) const
	{
		return agent.IsValid() && AgentStates.IsValid(agent.Id);
	}



#ifdef NS_ENGINE_DEBUG_DRAW
public:
	void DebugDraw(class nsRenderer* renderer);
#endif // NS_ENGINE_DEBUG_DRAW

};
