#include "cstGame.h"
#include "nsRenderManager.h"
#include "nsPhysics.h"

#if CST_GAME_WITH_EDITOR
#include "Editor/cstEditor.h"
#endif // CST_GAME_WITH_EDITOR



nsGameApplication* CreateGameApplication(int windowResX, int windowResY, nsEWindowFullscreenMode fullscreenMode) noexcept
{
	return new cstGame("cst_game", windowResX, windowResY, fullscreenMode);
}




cstGame::cstGame(const char* title, int width, int height, nsEWindowFullscreenMode fullscreenMode) noexcept
	: nsGameApplication(title, width, height, fullscreenMode)
{
	CurrentState = cstEGameState::NONE;
	PendingChangeState = cstEGameState::NONE;
}


void cstGame::Initialize() noexcept
{
	nsGameApplication::Initialize();

#if CST_GAME_WITH_EDITOR
	g_Editor = ns_CreateObject<cstEditor>(this);
	ChangeState(cstEGameState::EDITING);

#else
	ChangeState(cstEGameState::INTRO);

#endif // CST_GAME_WITH_EDITOR
}


void cstGame::Shutdown() noexcept
{
	// TODO: ...

	nsGameApplication::Shutdown();
}


void cstGame::TickUpdate(float deltaTime) noexcept
{
	nsGameApplication::TickUpdate(deltaTime);


	if (PendingChangeState != cstEGameState::NONE && PendingChangeState != CurrentState)
	{
		switch (PendingChangeState)
		{
			case cstEGameState::INTRO: HandleGameState_Intro(); break;
			case cstEGameState::MAIN_MENU: HandleGameState_MainMenu(); break;
			case cstEGameState::LOADING: HandleGameState_Loading(); break;
			case cstEGameState::PLAYING: HandleGameState_Playing(); break;
			case cstEGameState::CUTSCENE: HandleGameState_Cutscene(); break;

		#if CST_GAME_WITH_EDITOR
			case cstEGameState::EDITING: HandleGameState_Editing(); break;
		#endif // CST_GAME_WITH_EDITOR

			default: NS_Assert(0); break;
		}

		CurrentState = PendingChangeState;
		PendingChangeState = cstEGameState::NONE;
	}


	switch (CurrentState)
	{
		case cstEGameState::INTRO:
		{
			break;
		}


		case cstEGameState::MAIN_MENU:
		{
			break;
		}


		case cstEGameState::LOADING:
		{
			break;
		}


		case cstEGameState::PLAYING:
		{
			break;
		}


		case cstEGameState::CUTSCENE:
		{
			break;
		}


	#if CST_GAME_WITH_EDITOR
		case cstEGameState::EDITING:
		{
			g_Editor->TickUpdate(deltaTime);

			break;
		}
	#endif // CST_GAME_WITH_EDITOR


		default:
			NS_Assert(0);
			break;
	}
}


void cstGame::PreRender() noexcept
{
	nsRenderContextWorld& renderContext = nsRenderManager::Get().GetWorldRenderContext(MainWorld);

	/*
	const nsVector3 testAABB(16.0f);
	renderContext.AddPrimitiveMesh_AABB(-testAABB, testAABB, nsColor::RED);

	const nsVector3 testCenter(500.0f, 300.0f, 200.0f);
	renderContext.AddPrimitiveMesh_AABB(testCenter - 50.0f, testCenter + 50.0f, nsColor::BLUE);

	renderContext.AddPrimitiveMesh_Arrow(nsVector3(-300.0f, 300.0f, 100.0f), nsQuaternion::FromRotation(45.0f, 0.0f, 0.0f), 100.0f, 8.0f, 32.0f, nsColor::CYAN, nsColor::BLUE);
	renderContext.AddPrimitiveMesh_Plane(nsVector3::ZERO, nsVector3(1.0f, 0.0f, 0.0f), 100.0f, nsColor::WHITE);

	renderContext.AddPrimitiveLine(nsVector3::ZERO, nsVector3(500.0f), nsColor::RED);

	nsLine line(nsVector3(100.0f, 100.0f, 100.0f), nsVector3(300.0f, 300.0f, 300.0f));
	renderContext.AddPrimitiveLine(line.A, line.B, nsColor::RED);
	renderContext.AddPrimitiveLine_CircleAroundAxis(line.A, line.GetDirection(), 100.0f, NS_MATH_PI, nsColor::CYAN);
	*/


#if CST_GAME_WITH_EDITOR
	if (CurrentState == cstEGameState::EDITING)
	{
		g_Editor->PreRender(renderContext);
	}
	else
	{
		nsGameApplication::PreRender();
	}
#endif // CST_GAME_WITH_EDITOR
}


void cstGame::OnMouseMove(const nsMouseMoveEventArgs& e) noexcept
{
	nsGameApplication::OnMouseMove(e);

#if CST_GAME_WITH_EDITOR
	if (CurrentState == cstEGameState::EDITING)
	{
		g_Editor->OnMouseMove(e);
	}
#endif // CST_GAME_WITH_EDITOR
}


void cstGame::OnMouseButton(const nsMouseButtonEventArgs& e) noexcept
{
	nsGameApplication::OnMouseButton(e);

#if CST_GAME_WITH_EDITOR
	if (CurrentState == cstEGameState::EDITING)
	{
		g_Editor->OnMouseButton(e);
	}
#endif // CST_GAME_WITH_EDITOR
}


void cstGame::OnMouseWheel(const nsMouseWheelEventArgs& e) noexcept
{
	nsGameApplication::OnMouseWheel(e);

#if CST_GAME_WITH_EDITOR
	if (CurrentState == cstEGameState::EDITING)
	{
		g_Editor->OnMouseWheel(e);
	}
#endif // CST_GAME_WITH_EDITOR
}


void cstGame::OnKeyboardButton(const nsKeyboardButtonEventArgs& e) noexcept
{
	nsGameApplication::OnKeyboardButton(e);

	if (e.ButtonState == nsEButtonState::PRESSED)
	{
		
	}
	else
	{

	}

#if CST_GAME_WITH_EDITOR
	if (CurrentState == cstEGameState::EDITING)
	{
		g_Editor->OnKeyboardButton(e);
	}
#endif // CST_GAME_WITH_EDITOR
}


void cstGame::OnGUI(nsGUIContext& context) noexcept
{
#if CST_GAME_WITH_EDITOR
	g_Editor->DrawGUI(context);
#endif // CST_GAME_WITH_EDITOR
}


void cstGame::HandleGameState_Intro()
{

}


void cstGame::HandleGameState_MainMenu()
{
}


void cstGame::HandleGameState_Loading()
{
}


void cstGame::HandleGameState_Playing()
{
}


void cstGame::HandleGameState_Cutscene()
{
}



#if CST_GAME_WITH_EDITOR

void cstGame::HandleGameState_Editing()
{
	g_Editor->MainViewport = &MainViewport;
	g_Editor->MainRenderer = MainRenderer;
	g_Editor->MainWorld = MainWorld;
}

#endif // CST_GAME_WITH_EDITOR
