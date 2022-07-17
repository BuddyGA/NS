#include "cstTypes.h"
#include "cstGame.h"



nsGameApplication* CreateGameApplication(int windowResX, int windowResY, nsEWindowFullscreenMode fullscreenMode) noexcept
{
	return new cstGame(TEXT("cst_game"), windowResX, windowResY, fullscreenMode);
}



nsLogCategory cstGameLog(TEXT("cstGameLog"), nsELogVerbosity::LV_DEBUG);
nsLogCategory cstPlayerLog(TEXT("cstPlayerLog"), nsELogVerbosity::LV_DEBUG);
nsLogCategory cstCharacterLog(TEXT("cstCharacterLog"), nsELogVerbosity::LV_DEBUG);
nsLogCategory cstAbilityLog(TEXT("cstAbilityLog"), nsELogVerbosity::LV_DEBUG);
