#include "nsGameApplication.h"
#include "nsConsole.h"
#include "nsRenderManager.h"
#include "nsWorld.h"
#include "nsAssetManager.h"
#include "nsEngine.h"



nsGameApplication::nsGameApplication(const char* title, int width, int height, nsEWindowFullscreenMode fullscreenMode) noexcept
	: nsWindow(title, width, height, fullscreenMode)
{
	bShowFPS = true;

	TestWindow0.Name = "window_test_0";
	TestWindow0.Title = "Window Test 0";
	TestWindow0.InitialRect = nsGUIRect(100.0f, 100.0f, 500.0f, 400.0f);

	TestTable.AddColumn("", 0.45f);
	TestTable.AddColumn("", 0.45f);
	TestTable.AddColumn("", 0.1f);

	/*
	TestWindow1.Name = "window_test_1";
	TestWindow1.Title = "Window Test 1";
	TestWindow1.InitialRect = nsGUIRect(100.0f, 100.0f, 500.0f, 400.0f);
	*/
}


void nsGameApplication::Initialize() noexcept
{
	NS_CONSOLE_RegisterCommand("fps");

#if _DEBUG
	NS_CONSOLE_RegisterCommand("gui");
#endif // _DEBUG

	MainWorld = g_Engine->CreateWorld("world_main", true);

	const nsPointInt windowDimension = GetDimension();
	MainViewport.SetDimension(static_cast<float>(windowDimension.X), static_cast<float>(windowDimension.Y));

	MainRenderer = ns_CreateObject<nsRenderer>(GetHandle());
	MainRenderer->GUIContext = &GUIContext;

	nsRenderManager::Get().RegisterRenderer(MainRenderer);

	LoadTestLevel_Boxes();
}


void nsGameApplication::Shutdown() noexcept
{

}


void nsGameApplication::HandleConsoleCommand(const nsString& command, const nsString* params, int paramCount) noexcept
{
	if (command == "fps")
	{
		bShowFPS = !bShowFPS;
	}


#if _DEBUG
	if (command == "gui" && paramCount > 0)
	{
		if (params[0] == "debugrect")
		{
			GUIContext.bDrawDebugRect = !GUIContext.bDrawDebugRect;
		}
		else if (params[0] == "debugrecth")
		{
			GUIContext.bDrawDebugHoveredRect = !GUIContext.bDrawDebugHoveredRect;
		}
	}
#endif // _DEBUG
}


void nsGameApplication::StartPlay() noexcept
{
	MainWorld->DispatchStartPlay();
}


void nsGameApplication::TickUpdate(float deltaTime) noexcept 
{

}


void nsGameApplication::PostPhysicsUpdate() noexcept
{

}


void nsGameApplication::PreRender() noexcept
{
	const nsPointInt windowDimension = GetDimension();
	MainViewport.SetDimension(static_cast<float>(windowDimension.X), static_cast<float>(windowDimension.Y));

	MainRenderer->Viewport = MainViewport;
	MainRenderer->RenderTargetDimension = windowDimension;
	MainRenderer->RenderFinalTexture = nsERenderFinalTexture::SCENE_RENDER_TARGET;
	MainRenderer->RenderContextWorld = &nsRenderManager::Get().GetWorldRenderContext(MainWorld);
}


void nsGameApplication::DrawGUI() noexcept
{
	const nsPointInt dimension = GetDimension();

	nsGUIRect canvasRect;
	canvasRect.Left = 1.0f;
	canvasRect.Top = 1.0f;
	canvasRect.Right = static_cast<float>(dimension.X) - 1.0f;
	canvasRect.Bottom = static_cast<float>(dimension.Y) - 1.0f;

	GUIContext.BeginRender(nsPointFloat(static_cast<float>(dimension.X), static_cast<float>(dimension.Y)), canvasRect);

	OnGUI(GUIContext);

	if (bShowFPS)
	{
		float fpsTime = 0.0f;
		float fps = 0.0f;
		g_Engine->GetAverageFPS(fpsTime, fps);

		const nsName fpsText = nsName::Format("Frame: %.2f ms (%i FPS)", fpsTime, static_cast<int>(fps));
		nsColor fpsTextColor = nsColor::GREEN;

		if (fpsTime > 20.0f && fpsTime < 33.333f)
		{
			fpsTextColor = nsColor::YELLOW;
		}
		else if (fpsTime > 33.333f)
		{
			fpsTextColor = nsColor::RED;
		}

		const nsGUIRect& canvasRect = GUIContext.GetCanvasRect();
		GUIContext.AddDrawText(*fpsText, fpsText.GetLength(), nsPointFloat(canvasRect.Right - 156.0f, canvasRect.Top + 4.0f), fpsTextColor);
	}

	/*
	GUIContext.AddDrawTriangle(nsPointFloat(300.0f, 300.0f), 32.0f, 0.0f, nsColor::WHITE);
	GUIContext.AddDrawTriangleLeft(nsPointFloat(400.0f, 400.0f), 32.0f, nsColor::RED);
	GUIContext.AddDrawTriangleRight(nsPointFloat(400.0f, 500.0f), 32.0f, nsColor::GREEN);
	GUIContext.AddDrawTriangleDown(nsPointFloat(500.0f, 500.0f), 32.0f, nsColor::BLUE);


	TestWindow0.BeginDraw(GUIContext);
	{
		const nsGUIRect contentRect = TestWindow0.GetContentRect();
		
		GUIContext.BeginRegion(nullptr, contentRect, nsPointFloat(), nsEGUIElementLayout::NONE, nsEGUIScrollOption::None, false);

		TestTable.Size = nsPointFloat(contentRect.GetWidth(), contentRect.GetHeight());
		TestTable.BeginDraw(GUIContext);
		{
			TestTable.BeginColumn(GUIContext, 0, nsPointFloat(4.0f));
			{
				GUIContext.AddControlText("column_0", nsColor::RED);
			}
			TestTable.EndColumn(GUIContext);

			TestTable.BeginColumn(GUIContext, 1);
			{
				GUIContext.AddControlText("column_1", nsColor::GREEN);
			}
			TestTable.EndColumn(GUIContext);


			TestTable.BeginColumn(GUIContext, 2);
			{
				GUIContext.AddControlText("column_2", nsColor::BLUE);
			}
			TestTable.EndColumn(GUIContext);
		}
		TestTable.EndDraw(GUIContext);

		GUIContext.EndRegion();
	}
	TestWindow0.EndDraw(GUIContext);


	TestWindow1.BeginDraw(GUIContext);
	{
	}
	TestWindow1.EndDraw(GUIContext);
	*/

	ConsoleWindow.Draw(GUIContext);

	GUIContext.EndRender();
}


void nsGameApplication::OnMouseMove(const nsMouseMoveEventArgs& e) noexcept
{
	GUIContext.MouseMove(e);

}


void nsGameApplication::OnMouseButton(const nsMouseButtonEventArgs& e) noexcept
{
	GUIContext.MouseButton(e);

	/*
	if (e.ButtonState == nsEButtonState::PRESSED)
	{
		NS_LogDebug(nsTempLog, "Mouse pressed!");
	}
	else if (e.ButtonState == nsEButtonState::RELEASED)
	{
		NS_LogDebug(nsTempLog, "Mouse released!");
	}
	else if (e.ButtonState == nsEButtonState::REPEAT)
	{
		NS_LogDebug(nsTempLog, "Mouse double clicked!");
	}
	*/
}


void nsGameApplication::OnMouseWheel(const nsMouseWheelEventArgs& e) noexcept
{
	GUIContext.MouseWheel(e);
}


void nsGameApplication::OnKeyboardButton(const nsKeyboardButtonEventArgs& e) noexcept
{
	GUIContext.KeyboardButton(e);

	if (e.ButtonState == nsEButtonState::PRESSED)
	{
		/*
		if (e.Key == nsEInputKey::KEYBOARD_SHIFT_LEFT)
		{
			NS_LogDebug(nsTempLog, "KEYBOARD_SHIFT_LEFT pressed!");
		}
		else if (e.Key == nsEInputKey::KEYBOARD_SHIFT_RIGHT)
		{
			NS_LogDebug(nsTempLog, "KEYBOARD_SHIFT_RIGHT pressed!");
		}
		else if (e.Key == nsEInputKey::KEYBOARD_CTRL_LEFT)
		{
			NS_LogDebug(nsTempLog, "KEYBOARD_CTRL_LEFT pressed!");
		}
		else if (e.Key == nsEInputKey::KEYBOARD_CTRL_RIGHT)
		{
			NS_LogDebug(nsTempLog, "KEYBOARD_CTRL_RIGHT pressed!");
		}
		else if (e.Key == nsEInputKey::KEYBOARD_ALT_LEFT)
		{
			NS_LogDebug(nsTempLog, "KEYBOARD_ALT_LEFT pressed!");
		}
		else if (e.Key == nsEInputKey::KEYBOARD_ALT_RIGHT)
		{
			NS_LogDebug(nsTempLog, "KEYBOARD_ALT_RIGHT pressed!");
		}
		else if (e.Key == nsEInputKey::KEYBOARD_F10)
		{
			NS_LogDebug(nsTempLog, "KEYBOARD_F10 pressed!");
		}
		*/

		if (e.Key == nsEInputKey::KEYBOARD_TILDE)
		{
			ConsoleWindow.Toggle();
		}
	}
	else if (e.ButtonState == nsEButtonState::RELEASED)
	{
		/*
		if (e.Key == nsEInputKey::KEYBOARD_SHIFT_LEFT)
		{
			NS_LogDebug(nsTempLog, "KEYBOARD_SHIFT_LEFT released!");
		}
		else if (e.Key == nsEInputKey::KEYBOARD_SHIFT_RIGHT)
		{
			NS_LogDebug(nsTempLog, "KEYBOARD_SHIFT_RIGHT released!");
		}
		else if (e.Key == nsEInputKey::KEYBOARD_CTRL_LEFT)
		{
			NS_LogDebug(nsTempLog, "KEYBOARD_CTRL_LEFT released!");
		}
		else if (e.Key == nsEInputKey::KEYBOARD_CTRL_RIGHT)
		{
			NS_LogDebug(nsTempLog, "KEYBOARD_CTRL_RIGHT released!");
		}
		else if (e.Key == nsEInputKey::KEYBOARD_ALT_LEFT)
		{
			NS_LogDebug(nsTempLog, "KEYBOARD_ALT_LEFT released!");
		}
		else if (e.Key == nsEInputKey::KEYBOARD_ALT_RIGHT)
		{
			NS_LogDebug(nsTempLog, "KEYBOARD_ALT_RIGHT released!");
		}
		else if (e.Key == nsEInputKey::KEYBOARD_F10)
		{
			NS_LogDebug(nsTempLog, "KEYBOARD_F10 released!");
		}
		*/
	}
}


void nsGameApplication::OnCharInput(char c) noexcept
{
	//NS_LogDebug(nsTempLog, "CharInput: %c", c);

	GUIContext.AddCharInput(c);
}


void nsGameApplication::OnMinimized() noexcept
{
	NS_CONSOLE_Log(nsTempLog, "Window [%s] minimized!", GetTitle());
	nsRenderManager::Get().UnregisterRenderer(MainRenderer);
}


void nsGameApplication::OnMaximized() noexcept
{
	NS_CONSOLE_Log(nsTempLog, "Window [%s] maximized!", GetTitle());
	nsRenderManager::Get().RegisterRenderer(MainRenderer);
}


void nsGameApplication::OnRestored() noexcept
{
	NS_CONSOLE_Log(nsTempLog, "Window [%s] restored!", GetTitle());
	nsRenderManager::Get().RegisterRenderer(MainRenderer);
}


void nsGameApplication::LoadTestLevel_Boxes()
{
	nsAssetManager& assetManager = nsAssetManager::Get();

	nsActor* floorActor = MainWorld->CreateActor("floor_actor", nsVector3(0.0f, -8.0f, 0.0f));
	{
		nsBoxCollisionComponent* boxCollisionComp = floorActor->AddComponent<nsBoxCollisionComponent>("box_collision");
		boxCollisionComp->HalfExtent = nsVector3(1600.0f, 8.0f, 1600.0f);
		boxCollisionComp->UpdateCollisionVolume();

		nsMeshComponent* meshComp = floorActor->AddComponent<nsMeshComponent>("mesh");
		meshComp->SetMesh(assetManager.LoadModelAsset(NS_ENGINE_ASSET_MODEL_DEFAULT_FLOOR_NAME));

		floorActor->SetRootComponent(boxCollisionComp);
		MainWorld->AddActorToLevel(floorActor);
	}


	nsActor* wallActor = MainWorld->CreateActor("wall_actor", nsVector3(-500.0f, 138.0f, 100.0f));
	{
		nsMeshComponent* meshComp = wallActor->AddComponent<nsMeshComponent>("mesh");
		meshComp->SetMesh(assetManager.LoadModelAsset(NS_ENGINE_ASSET_MODEL_DEFAULT_WALL_NAME));

		wallActor->SetRootComponent(meshComp);
		MainWorld->AddActorToLevel(wallActor);
	}

	nsSharedModelAsset boxModelAsset = assetManager.LoadModelAsset(NS_ENGINE_ASSET_MODEL_DEFAULT_BOX_NAME);

	nsActor* boxCenter = MainWorld->CreateActor("box_center", nsVector3());
	{
		nsMeshComponent* meshComp = boxCenter->AddComponent<nsMeshComponent>("mesh");
		meshComp->SetMesh(boxModelAsset);

		boxCenter->SetRootComponent(meshComp);
		MainWorld->AddActorToLevel(boxCenter);
	}


	const int boxCountX = 4;
	const int boxCountY = 4; 
	const int boxCountZ = 4;
	
	const nsVector3 baseSpawnPosition(-300.0f, 300.0f, 300.0f);
	nsVector3 spawnPosition = baseSpawnPosition;
	int count = 0;

	for (int x = 0; x < boxCountX; ++x)
	{
		for (int y = 0; y < boxCountY; ++y)
		{
			for (int z = 0; z < boxCountZ; ++z)
			{
				nsActor* actor = MainWorld->CreateActor(nsName::Format("box_actor_%i", count++), spawnPosition, nsQuaternion::FromRotation(nsMath::RandomInRange(-30.0f, 30.0f), nsMath::RandomInRange(-80.0f, 80.0f), 0.0f));
				{
					nsBoxCollisionComponent* boxCollisionComp = actor->AddComponent<nsBoxCollisionComponent>("box_collision");

					nsMeshComponent* meshComp = actor->AddComponent<nsMeshComponent>("mesh");
					meshComp->SetMesh(boxModelAsset);

					actor->SetRootComponent(boxCollisionComp);
					MainWorld->AddActorToLevel(actor);
				}

				spawnPosition.Z += 300.0f;
			}

			spawnPosition.Y += 300.0f;
			spawnPosition.Z = baseSpawnPosition.Z;
		}

		spawnPosition.X += 300.0f;
		spawnPosition.Y = baseSpawnPosition.Y;
	}
}
