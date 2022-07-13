#include "cstGame.h"
#include "cstCharacter.h"
#include "nsRenderManager.h"

#if CST_GAME_WITH_EDITOR
#include "Editor/cstEditor.h"
#endif // CST_GAME_WITH_EDITOR



cstGame::cstGame(const wchar_t* title, int width, int height, nsEWindowFullscreenMode fullscreenMode) noexcept
	: nsGameApplication(title, width, height, fullscreenMode)
{
	CurrentState = cstEGameState::NONE;
	PendingChangeState = cstEGameState::NONE;
	Character = nullptr;

	CameraMoveAxis = nsVector3::ZERO;
	CameraDistance = 500.0f;
	CameraMoveSpeed = 1000.0f;
	bDebugDrawBorders = true;
}


void cstGame::Initialize() noexcept
{
	nsGameApplication::Initialize();

	Character = MainWorld->CreateActor<cstCharacter>("character_main", false, nsVector3(0.0f, 100.0f, -300.0f));
	MainWorld->AddActorToLevel(Character);


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
			nsVector3 cameraMoveDirection = CameraTransform.GetAxisRight() * CameraMoveAxis.X;
			cameraMoveDirection += CameraTransform.GetAxisUp() * CameraMoveAxis.Y;
			cameraMoveDirection = cameraMoveDirection - nsVector3::Project(cameraMoveDirection, nsVector3::UP);
			cameraMoveDirection.Normalize();
			cameraMoveDirection *= CameraMoveSpeed;

			nsVector3 newCameraPosition = CameraTransform.Position + cameraMoveDirection;
			CameraTransform.Position = nsVector3::Lerp(CameraTransform.Position, newCameraPosition, deltaTime);

			CameraMoveAxis = nsVector3::ZERO;
			MainViewport.SetViewTransform(CameraTransform);

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

	if (CurrentState == cstEGameState::PLAYING)
	{
		if (e.ButtonState == nsEButtonState::PRESSED)
		{
			if (e.Key == nsEInputKey::MOUSE_RIGHT && Character)
			{
				const nsVector2 mousePosition(static_cast<float>(e.Position.X), static_cast<float>(e.Position.Y));
				nsVector3 rayStart, rayDirection;

				if (MainViewport.ProjectToWorld(mousePosition, rayStart, rayDirection))
				{
					nsPhysicsHitResult hitResult;
					if (MainWorld->PhysicsRayCast(hitResult, rayStart, rayDirection, 10000.0f))
					{
						Character->SetMoveTargetPosition(hitResult.WorldPosition);
					}
				}
			}
		}
	}

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

	CameraMoveAxis.Z = static_cast<float>(e.ScrollValue.Y);

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

	if (e.ButtonState == nsEButtonState::RELEASED && e.Key == nsEInputKey::KEYBOARD_F9)
	{
		if (CurrentState == cstEGameState::EDITING)
		{
			ChangeState(cstEGameState::PLAYING);
		}
		else if (CurrentState == cstEGameState::PLAYING)
		{
			ChangeState(cstEGameState::EDITING);
		}

		return;
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
	nsGUIRect canvasRect = context.GetCanvasRect();
	canvasRect.Left -= 2.0f;
	canvasRect.Top -= 2.0f;
	canvasRect.Right += 2.0f;
	canvasRect.Bottom += 2.0f;

	const nsGUIRect borderTop(canvasRect.Left, canvasRect.Top, canvasRect.Right, canvasRect.Top + 4.0f);
	const nsGUIRect borderLeft(canvasRect.Left, canvasRect.Top, canvasRect.Left + 4.0f, canvasRect.Bottom);
	const nsGUIRect borderRight(canvasRect.Right - 4.0f, canvasRect.Top, canvasRect.Right, canvasRect.Bottom);
	const nsGUIRect borderBottom(canvasRect.Left, canvasRect.Bottom - 4.0f, canvasRect.Right, canvasRect.Bottom);
	const nsPointFloat mousePosition = context.GetMousePosition();
	
	if (borderLeft.IsPointInside(mousePosition))
	{
		CameraMoveAxis.X = -1.0f;
	}
	else if (borderRight.IsPointInside(mousePosition))
	{
		CameraMoveAxis.X = 1.0f;
	}

	if (borderTop.IsPointInside(mousePosition))
	{
		CameraMoveAxis.Y = 1.0f;
	}
	else if (borderBottom.IsPointInside(mousePosition))
	{
		CameraMoveAxis.Y = -1.0f;
	}


	if (CurrentState == cstEGameState::PLAYING && bDebugDrawBorders)
	{
		context.AddDrawRect(borderTop, nsColor::GRAY);
		context.AddDrawRect(borderLeft, nsColor::GRAY);
		context.AddDrawRect(borderRight, nsColor::GRAY);
		context.AddDrawRect(borderBottom, nsColor::GRAY);
	}

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
	MainWorld->DispatchStartPlay();
	
	const nsPointFloat viewportDimension = MainViewport.GetDimension();
	
	nsRectInt clipMouseRect;
	clipMouseRect.X = 2;
	clipMouseRect.Y = 2;
	clipMouseRect.Width = static_cast<int>(viewportDimension.X - 4.0f);
	clipMouseRect.Height = static_cast<int>(viewportDimension.Y - 4.0f);
	
	ClipMouseCursor(clipMouseRect);

	CameraTransform.Rotation = nsQuaternion::FromRotation(60.0f, -45.0f, 0.0f);
	CameraTransform.Position = nsVector3(500.0f, 1000.0f, -500.0f);

	if (Character)
	{
		const nsVector3 characterPosition = Character->GetWorldPosition();
		CameraTransform.Position.X += characterPosition.X;
		CameraTransform.Position.Y += characterPosition.Y;
		CameraTransform.Position.Z += characterPosition.Z;
	}
}


void cstGame::HandleGameState_Cutscene()
{
}



#if CST_GAME_WITH_EDITOR

void cstGame::HandleGameState_Editing()
{
	ClipMouseCursor(false);
	g_Editor->MainViewport = &MainViewport;
	g_Editor->MainRenderer = MainRenderer;
	g_Editor->MainWorld = MainWorld;
}

#endif // CST_GAME_WITH_EDITOR
