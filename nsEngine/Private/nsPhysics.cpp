#include "nsPhysics.h"



static nsLogCategory PhysicsLog("nsPhysicsLog", nsELogVerbosity::LV_DEBUG);



nsPhysicsManager::nsPhysicsManager() noexcept
	: bInitialized(false)
{

}


void nsPhysicsManager::Initialize() noexcept
{
	if (bInitialized)
	{
		return;
	}

	NS_LogInfo(PhysicsLog, "Initialize physics manager");

	bInitialized = true;
}


void nsPhysicsManager::Update(float deltaTime) noexcept
{

}
