#include "nsAnimation.h"


NS_ENGINE_DEFINE_HANDLE(nsSkeletonID);
NS_ENGINE_DEFINE_HANDLE(nsAnimationSequenceID);


static nsLogCategory AnimationLog("nsAnimationLog", nsELogVerbosity::LV_DEBUG);



nsAnimationManager::nsAnimationManager() noexcept
	: bInitialized(false)
{

}


void nsAnimationManager::Initialize() noexcept
{
	if (bInitialized)
	{
		return;
	}

	NS_LogInfo(AnimationLog, "Initialize animation manager");


	bInitialized = true;
}


void nsAnimationManager::Update(float deltaTime) noexcept
{

}
