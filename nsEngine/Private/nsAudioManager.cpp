#include "nsAudioManager.h"



NS_ENGINE_DEFINE_HANDLE(nsAudioID);


nsLogCategory AudioLog(TEXT("nsAudioLog"), nsELogVerbosity::LV_DEBUG);



nsAudioManager::nsAudioManager() noexcept
{
	bInitialized = false;
}


void nsAudioManager::Initialize()
{
	if (bInitialized)
	{
		return;
	}


	bInitialized = true;
}
