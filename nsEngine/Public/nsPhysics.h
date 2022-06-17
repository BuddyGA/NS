#pragma once

#include "nsEngineTypes.h"



class NS_ENGINE_API nsPhysicsManager
{
private:
	bool bInitialized;


public:
	void Initialize() noexcept;

	void Update(float deltaTime) noexcept;

	NS_DECLARE_SINGLETON(nsPhysicsManager)

};
