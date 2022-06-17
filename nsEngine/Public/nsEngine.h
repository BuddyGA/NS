#pragma once

#include "nsEngineTypes.h"


class nsWorld;
class nsGameApplication;



class NS_ENGINE_API nsEngine 
{
private:
	nsName GameModuleName;
	nsModuleHandle GameModuleHandle;
	nsGameApplication* Game;
	int64 StartTick;
	int64 PrevTick;
	int64 FrameCounter;
	float DeltaTimeSeconds;

	float FpsTimeMs;
	float Fps;

	nsTArray<nsWorld*> Worlds;


public:
	nsEngine() noexcept;
	void Initialize();

private:
	void ReloadGameModule(bool bIsHotReload);
	void CalculateAverageFPS();

public:
	void MainLoop();
	void Shutdown();
	NS_NODISCARD nsWorld* FindWorld(const nsName& name) const;
	nsWorld* CreateWorld(nsName name, bool bHasPhysics);
	void DestroyWorld(nsWorld*& world);


	NS_NODISCARD_INLINE float GetDeltaTimeSeconds() const noexcept
	{
		return DeltaTimeSeconds;
	}


	NS_NODISCARD_INLINE void GetAverageFPS(float& outMs, float& outFps) const noexcept
	{
		outMs = FpsTimeMs;
		outFps = Fps;
	}


	NS_DECLARE_NOCOPY_NOMOVE(nsEngine)

};


extern NS_ENGINE_API nsEngine* g_Engine;
