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


	struct AgentState
	{
		nsVector3 MoveTargetPosition;
		int ActiveAgentIndex;
	};

	nsTArrayFreeList<nsNavigationAgentParams> AgentParams;
	nsTArrayFreeList<AgentState> AgentActiveStates;


public:
	void Initialize();
	void BuildNavMesh(nsWorld* world);
	void BuildNavMesh(const nsNavigationInputGeometry& inputGeometry, const nsNavigationBuildSettings& buildSettings);


	nsNavigationAgentID CreateAgent(float radius, float height, float maxAcceleration, float maxSpeed);
	void DestroyAgent(nsNavigationAgentID& agent);
	void UpdateAgentParams(nsNavigationAgentID agent, float radius, float height, float maxAcceleration, float maxSpeed);
	void SetAgentActive(nsNavigationAgentID agent, bool bActive, nsNavigationAgentComponent* component);
	void SetAgentMoveTarget(nsNavigationAgentID agent, const nsVector3& targetPosition);
	void MoveAgents(float deltaTime);

	NS_NODISCARD_INLINE bool IsAgentValid(nsNavigationAgentID agent) const
	{
		return agent.IsValid() && AgentParams.IsValid(agent.Id);
	}
	


#ifdef NS_ENGINE_DEBUG_DRAW
public:
	void DebugDraw(class nsRenderer* renderer);
#endif // NS_ENGINE_DEBUG_DRAW

};
