#pragma once

#include "nsEngineTypes.h"



class NS_ENGINE_API nsAnimationManager
{
private:
	bool bInitialized;


public:
	void Initialize() noexcept;

	void Update(float deltaTime) noexcept;

	NS_DECLARE_SINGLETON(nsAnimationManager)

};
