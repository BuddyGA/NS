#pragma once

#ifdef __CST_GAME_BUILD__
#define CST_GAME_API __declspec(dllexport)
#else
#define CST_GAME_API __declspec(dllimport)
#endif // __CST_GAME_BUILD__


#define CST_GAME_WITH_EDITOR			(1)


#include "nsGameApplication.h"
#include "nsConsole.h"
#include "nsActor.h"



extern "C" CST_GAME_API nsGameApplication* CreateGameApplication(int windowResX, int windowResY, nsEWindowFullscreenMode fullscreenMode) noexcept;



enum class cstEGameState : uint8
{
	NONE = 0,
	INTRO,
	MAIN_MENU,
	LOADING,
	PLAYING,
	CUTSCENE,

#if CST_GAME_WITH_EDITOR
	EDITING,
#endif // NS_GAME_WITH_EDITOR
};
