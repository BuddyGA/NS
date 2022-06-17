#include "nsWindow.h"
#include "nsLogger.h"
#include "nsThreadPool.h"
#include "nsCommandLines.h"
#include "nsEngine.h"



#ifdef NS_PLATFORM_WINDOWS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	nsCommandLines::Get().Initialize(GetCommandLineA());

	nsPlatform::Initialize();


#else
int main(int argc, char* argv[])
{
	nsCommandLines::Get().Initialize(argc, argv);

	nsPlatform::Initialize();

#endif // NS_PLATFORM_WINDOWS


	nsLogger::Get().Initialize(nsELogVerbosity::LV_DEBUG, "Log.txt");

	nsThreadPool::Initialize();

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
