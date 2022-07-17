#pragma once

#include "nsAudioTypes.h"



class NS_ENGINE_API nsAudioManager
{
	NS_DECLARE_SINGLETON(nsAudioManager)

private:
	bool bInitialized;


public:
	void Initialize();

};
