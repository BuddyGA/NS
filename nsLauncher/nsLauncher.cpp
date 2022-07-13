#include "nsWindow.h"
#include "nsLogger.h"
#include "nsThreadPool.h"
#include "nsCommandLines.h"
#include "nsEngine.h"



#ifdef NS_PLATFORM_WINDOWS
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	nsCommandLines::Get().Initialize(GetCommandLine());

	if (nsCommandLines::Get().HasCommand(TEXT("console")) && AllocConsole())
	{
		ShowWindow(GetConsoleWindow(), SW_SHOW);
	}

#else
#error Unknown platform!

#endif // NS_PLATFORM_WINDOWS


	nsPlatform::Initialize();

	nsLogger::Get().Initialize(nsELogVerbosity::LV_DEBUG, TEXT("Log.txt"));

	nsThreadPool::Initialize();


	// TODO: Init online platform (Steam, etc.)


	// TODO: Read config from file (nsConfig.ini)

	
	g_Engine = new nsEngine();
	g_Engine->Initialize();


#ifdef NS_PLATFORM_WINDOWS
	MSG msg{};
	bool bRunning = true;

	while (bRunning)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bRunning = false;
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if (!bRunning)
		{
			break;
		}

		g_Engine->MainLoop();
	}
#endif // NS_PLATFORM_WINDOWS


	g_Engine->Shutdown();

	nsThreadPool::Shutdown();

	nsPlatform::Shutdown();


	return 0;
}
