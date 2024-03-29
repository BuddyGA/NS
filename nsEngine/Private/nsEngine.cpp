#include "nsEngine.h"
#include "nsCommandLines.h"
#include "nsConsole.h"
#include "nsPhysicsManager.h"
#include "nsRenderManager.h"
#include "nsAnimationManager.h"
#include "nsNavigationManager.h"
#include "nsAssetManager.h"
#include "nsAssetImporter.h"
#include "nsGameApplication.h"
#include "nsWorld.h"



static nsLogCategory EngineLog(TEXT("nsEngineLog"), nsELogVerbosity::LV_DEBUG);
nsMemory g_EngineDefaultMemory("engine_default", NS_MEMORY_SIZE_MiB(1), 16);



nsEngine::nsEngine() noexcept
	: GameModuleName("")
	, GameModuleHandle(nullptr)
	, Game(nullptr)
	, StartTick(0)
	, PrevTick(0)
	, FrameCounter(0)
	, DeltaTimeSeconds(0.0f)
	, FpsTimeMs(0.0f)
	, Fps(0.0f)
{
	Worlds.Reserve(4);
	PhysicsTimeSteps = 0.016667f;
}


void nsEngine::Initialize()
{
	NS_LogInfo(EngineLog, TEXT("Initialize engine"));

	nsConsoleManager::Get().Initialize();
	nsPhysicsManager::Get().Initialize();
	nsRenderManager::Get().Initialize();
	nsAnimationManager::Get().Initialize();
	nsNavigationManager::Get().Initialize();
	nsAssetManager::Get().Initialize();
	nsAssetImporter::Get().Initialize();

	nsCommandLines& commandLines = nsCommandLines::Get();
	GameModuleName = commandLines.GetValue(TEXT("game"));

	if (GameModuleName.GetLength() > 0)
	{
		ReloadGameModule(false);
	}
	else
	{
		NS_LogWarning(EngineLog, TEXT("Game module name not specified in command lines! Ex: -game=[game_module_name]"));
	}

	if (Game)
	{
		Game->Initialize();
	}

	StartTick = nsPlatform::PerformanceQuery_Counter();


#ifndef __NS_ENGINE_SHIPPING__
	NS_CONSOLE_RegisterCommand(TEXT("class"));
	NS_CONSOLE_RegisterCommand(TEXT("navigation"));
#endif // !__NS_ENGINE_SHIPPING__

}


void nsEngine::HandleConsoleCommand(const nsString& command, const nsString* params, int paramCount)
{
#ifndef __NS_ENGINE_SHIPPING__
	if (command == TEXT("class") && paramCount > 0)
	{
		const nsTArray<const nsClass*> classes = nsReflection::FindAllClasses(*params[0]);

		if (classes.GetCount() > 0)
		{
			nsString stringMessage = nsString::Format(TEXT("Class list [Count: %i]\n"), classes.GetCount());
			nsString classNameString;

			for (int i = 0; i < classes.GetCount(); ++i)
			{
				classNameString = *classes[i]->GetName();
				stringMessage += nsString::Format(TEXT("%s\n"), *classNameString);
			}

			NS_CONSOLE_Log(nsTempLog, TEXT("%s"), *stringMessage);
		}
		else
		{
			NS_CONSOLE_Log(nsTempLog, TEXT("Class not found!"));
		}
	}
	else if (command == "navigation" && paramCount > 0)
	{
		const nsString& param0 = params[0];

		if (param0 == "build")
		{
			nsNavigationManager::Get().BuildNavMesh(Worlds[0]);
		}
	}
#endif // !__NS_ENGINE_SHIPPING__
}


void nsEngine::ReloadGameModule(bool bIsHotReload)
{
	NS_Assert(GameModuleName.GetLength() > 0);

	static nsString _gameDLL = nsString::Format(TEXT("%s.dll"), *GameModuleName);

#if NS_ENGINE_GAME_MODULE_HOTRELOAD
	if (nsCommandLines::Get().HasCommand("hotreload"))
	{
		static int _hotReloadCounter = 0;
		static nsString _prevHotReloadDLL = "";

		if (GameModuleHandle)
		{
			nsPlatform::Module_Unload(GameModuleHandle);
			GameModuleHandle = nullptr;
		}

		const nsString hotReloadDLL = nsString::Format(TEXT("%s_%i.dll"), *GameModuleName, _hotReloadCounter);

		bool bSuccess = nsPlatform::File_Copy(*_gameDLL, *hotReloadDLL);
		NS_Assert(bSuccess);

		GameModuleHandle = nsPlatform::Module_Load(*hotReloadDLL);
		NS_ValidateV(GameModuleHandle, TEXT("Fail to load game module [%s]"), *_gameDLL);

		if (_hotReloadCounter > 0)
		{
			nsPlatform::File_Delete(*_prevHotReloadDLL);
		}

		_hotReloadCounter++;
		_prevHotReloadDLL = hotReloadDLL;

		if (bIsHotReload)
		{
			NS_LogInfo(EngineLog, TEXT("Hot reload game module [%s]"), *hotReloadDLL);
		}
	}
	else
	{
		GameModuleHandle = nsPlatform::Module_Load(*_gameDLL);
	}

	NS_Assert(GameModuleHandle);

	if (Game == nullptr)
	{
		auto createGameApplication = nsPlatform::Module_GetFunctionAs<nsFCreateGameApplication>(GameModuleHandle, "CreateGameApplication");
		const int windowResX = nsCommandLines::Get().GetValueAsInt("resx");
		const int windowResY = nsCommandLines::Get().GetValueAsInt("resy");
		Game = createGameApplication(windowResX, windowResY, nsEWindowFullscreenMode::WINDOWED);
	}

#else
	GameModuleHandle = nsPlatform::Module_Load(*_gameDLL);
	auto createGameApplication = nsPlatform::Module_GetFunctionAs<nsFCreateGameApplication>(GameModuleHandle, "CreateGameApplication");
	const int windowResX = nsCommandLines::Get().GetValueAsInt("resx");
	const int windowResY = nsCommandLines::Get().GetValueAsInt("resy");
	Game = createGameApplication(windowResX, windowResY, nsEWindowFullscreenMode::WINDOWED);

#endif // NS_ENGINE_GAME_MODULE_HOTRELOAD

	if (!bIsHotReload)
	{
		NS_LogInfo(EngineLog, TEXT("Loaded game module [%s]"), *GameModuleName);
	}
}


void nsEngine::CalculateAverageFPS()
{
	static const int FPS_SAMPLE_COUNT = 3;

	static float _fpsTickTimer = 0.0f;
	static int _fpsFrameCounter = 0;

	if (_fpsFrameCounter >= FPS_SAMPLE_COUNT)
	{
		Fps = FPS_SAMPLE_COUNT / _fpsTickTimer;
		FpsTimeMs = _fpsTickTimer * 1000.0f / FPS_SAMPLE_COUNT;
		_fpsTickTimer = 0.0f;
		_fpsFrameCounter = 0;
	}

	_fpsTickTimer += DeltaTimeSeconds;
	_fpsFrameCounter++;
}


void nsEngine::MainLoop()
{
	static const float PERF_FREQUENCY = static_cast<float>(nsPlatform::PerformanceQuery_Frequency());

	const int64 currentTick = nsPlatform::PerformanceQuery_Counter();
	DeltaTimeSeconds = static_cast<float>(currentTick - PrevTick) / PERF_FREQUENCY;
	PrevTick = currentTick;

	if (FrameCounter == 0)
	{
		FrameCounter++;
		return;
	}

	CalculateAverageFPS();


#if NS_ENGINE_GAME_MODULE_HOTRELOAD
	if (Game)
	{
		static float _hotReloadTimer = 5.0f;
		_hotReloadTimer -= DeltaTimeSeconds;

		if (_hotReloadTimer <= 0.0f)
		{
			_hotReloadTimer = 5.0f;
			ReloadGameModule(true);
		}
	}
#endif // NS_ENGINE_GAME_MODULE_HOTRELOAD


	// Pre physics update
	{
		if (Game)
		{
			Game->TickUpdate(DeltaTimeSeconds);
		}

		for (int i = 0; i < Worlds.GetCount(); ++i)
		{
			Worlds[i]->DispatchPrePhysicsTickUpdate(DeltaTimeSeconds);
		}

		nsNavigationManager::Get().MoveAgents(DeltaTimeSeconds);
	}
	

	// Physics update
	{
		for (int i = 0; i < Worlds.GetCount(); ++i)
		{
			Worlds[i]->DispatchPhysicsTickUpdate(PhysicsTimeSteps);
		}

		nsPhysicsManager::Get().Simulate(PhysicsTimeSteps);
	}


	// Post physics update
	{
		for (int i = 0; i < Worlds.GetCount(); ++i)
		{
			Worlds[i]->DispatchPostPhysicsTickUpdate();
		}

		// Update animation
		nsAnimationManager::Get().UpdateAnimationPoses(DeltaTimeSeconds);
	}


	if (Game && !Game->IsMinimized())
	{
		Game->DrawGUI();
	}


	for (int i = 0; i < Worlds.GetCount(); ++i)
	{
		nsWorld* world = Worlds[i];
		world->SyncActorTransformsWithPhysics();
		world->CleanupPendingDestroyLevelsAndActors();
	}


	// TODO: Update game to next frame and start async ops for rendering


	nsAssetManager::Get().Update();


	if (Game)
	{
		Game->PreRender();
	}

	// Render
	nsRenderManager::Get().Render(DeltaTimeSeconds);


	FrameCounter++;


	if (Game && Game->IsMinimized())
	{
		nsPlatform::Thread_Sleep(8);
	}
}


void nsEngine::Shutdown()
{
	if (Game)
	{
		Game->Shutdown();
	}
}


nsWorld* nsEngine::FindWorld(const nsString& name) const
{
	if (name.GetLength() == 0)
	{
		return nullptr;
	}

	for (int i = 0; i < Worlds.GetCount(); ++i)
	{
		nsWorld* world = Worlds[i];
		NS_Assert(world);

		if (world->Name == name)
		{
			return world;
		}
	}

	return nullptr;
}


nsWorld* nsEngine::CreateWorld(nsString name, bool bHasPhysics)
{
	if (FindWorld(name))
	{
		NS_CONSOLE_Warning(EngineLog, TEXT("Fail to create world. World with name [%s] already exists!"), *name);
		return nullptr;
	}

	NS_CONSOLE_Log(EngineLog, TEXT("Create new world [%s]"), *name);

	nsWorld* newWorld = ns_CreateObject<nsWorld>();
	newWorld->Name = name;
	newWorld->Initialize(bHasPhysics);
	Worlds.Add(newWorld);
	nsRenderManager::Get().AddWorldRenderContext(newWorld);

	return newWorld;
}


void nsEngine::DestroyWorld(nsWorld*& world)
{
	if (world == nullptr)
	{
		return;
	}

	const int index = Worlds.Find(world);

	if (index != NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Log(EngineLog, TEXT("Destroy world [%s]"), *world->Name);

		nsRenderManager::Get().RemoveWorldRenderContext(world);
		Worlds.RemoveAt(index);
		ns_DestroyObject(world);
	}
}


nsEngine* g_Engine;
