#pragma once

#include "nsEngineTypes.h"


class nsGameApplication;



class NS_ENGINE_API nsEngine 
{
private:
	nsString GameModuleName;
	nsPlatformModuleHandle GameModuleHandle;
	nsGameApplication* Game;
	int64 StartTick;
	int64 PrevTick;
	int64 FrameCounter;
	float DeltaTimeSeconds;
	float FpsTimeMs;
	float Fps;
	nsTArray<nsWorld*> Worlds;

public:
	float PhysicsTimeSteps;


public:
	nsEngine() noexcept;
	void Initialize();
	void HandleConsoleCommand(const nsString& command, const nsString* params, int paramCount);

private:
	void ReloadGameModule(bool bIsHotReload);
	void CalculateAverageFPS();

public:
	void MainLoop();
	void Shutdown();
	NS_NODISCARD nsWorld* FindWorld(const nsString& name) const;
	nsWorld* CreateWorld(nsString name, bool bHasPhysics);
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
