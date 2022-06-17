#pragma once

#include "nsEngineTypes.h"


NS_ENGINE_DECLARE_HANDLE(nsSkeletonID, nsAnimationManager)
NS_ENGINE_DECLARE_HANDLE(nsAnimationSequenceID, nsAnimationManager)




class NS_ENGINE_API nsAnimationManager
{
private:
	bool bInitialized;


public:
	void Initialize() noexcept;

	void Update(float deltaTime) noexcept;

	NS_DECLARE_SINGLETON(nsAnimationManager)

};
