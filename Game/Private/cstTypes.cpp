#include "cstTypes.h"
#include "cstGame.h"



nsGameApplication* CreateGameApplication(int windowResX, int windowResY, nsEWindowFullscreenMode fullscreenMode) noexcept
{
	return new cstGame("cst_game", windowResX, windowResY, fullscreenMode);
}



nsLogCategory cstGameLog("cstGameLog", nsELogVerbosity::LV_DEBUG);
nsLogCategory cstPlayerLog("cstPlayerLog", nsELogVerbosity::LV_DEBUG);
nsLogCategory cstCharacterLog("cstCharacterLog", nsELogVerbosity::LV_DEBUG);
