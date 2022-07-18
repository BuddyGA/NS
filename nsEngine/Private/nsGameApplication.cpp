#include "nsGameApplication.h"
#include "nsRenderManager.h"
#include "nsWorld.h"
#include "nsAssetManager.h"
#include "nsEngine.h"
#include "nsPhysicsComponents.h"
#include "nsRenderComponents.h"
#include "nsNavigationManager.h"
#include "nsConsole.h"



nsGameApplication::nsGameApplication(const wchar_t* title, int width, int height, nsEWindowFullscreenMode fullscreenMode) noexcept
	: nsWindow(title, width, height, fullscreenMode)
{
	bShowFPS = true;
}


void nsGameApplication::Initialize() noexcept
{
	NS_CONSOLE_RegisterCommand(TEXT("fps"));

#ifndef __NS_ENGINE_SHIPPING__
	NS_CONSOLE_RegisterCommand(TEXT("class"));
	NS_CONSOLE_RegisterCommand(TEXT("gui"));
#endif // __NS_ENGINE_SHIPPING__

	MainWorld = g_Engine->CreateWorld(TEXT("world_main"), true);

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
	if (command == TEXT("fps"))
	{
		bShowFPS = !bShowFPS;
	}


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
	else if (command == TEXT("gui") && paramCount > 0)
	{
		if (params[0] == TEXT("debugrect"))
		{
			GUIContext.bDrawDebugRect = !GUIContext.bDrawDebugRect;
		}
		else if (params[0] == TEXT("debugrecth"))
		{
			GUIContext.bDrawDebugHoveredRect = !GUIContext.bDrawDebugHoveredRect;
		}
	}
#endif // __NS_ENGINE_SHIPPING__
}


void nsGameApplication::TickUpdate(float deltaTime) noexcept 
{

}


void nsGameApplication::PhysicsTickUpdate(float deltaTime) noexcept
{
}


void nsGameApplication::PostPhysicsTickUpdate() noexcept
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
	MainRenderer->World = MainWorld;
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

		const nsString fpsText = nsString::Format(TEXT("Frame: %.2f ms (%i FPS)"), fpsTime, static_cast<int>(fps));
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

}


void nsGameApplication::OnMouseWheel(const nsMouseWheelEventArgs& e) noexcept
{
	GUIContext.MouseWheel(e);

}


void nsGameApplication::OnKeyboardButton(const nsKeyboardButtonEventArgs& e) noexcept
{
	GUIContext.KeyboardButton(e);

	if (e.ButtonState == nsEButtonState::PRESSED && e.Key == nsEInputKey::KEYBOARD_TILDE)
	{
		ConsoleWindow.Toggle();
	}

}


void nsGameApplication::OnCharInput(char c) noexcept
{
	//NS_LogDebug(nsTempLog, "CharInput: %c", c);

	GUIContext.AddCharInput(c);
}


void nsGameApplication::OnMinimized() noexcept
{
	NS_CONSOLE_Log(nsTempLog, TEXT("Window [%s] minimized!"), GetTitle());
	nsRenderManager::Get().UnregisterRenderer(MainRenderer);
}


void nsGameApplication::OnMaximized() noexcept
{
	NS_CONSOLE_Log(nsTempLog, TEXT("Window [%s] maximized!"), GetTitle());
	nsRenderManager::Get().RegisterRenderer(MainRenderer);
}


void nsGameApplication::OnRestored() noexcept
{
	NS_CONSOLE_Log(nsTempLog, TEXT("Window [%s] restored!"), GetTitle());
	nsRenderManager::Get().RegisterRenderer(MainRenderer);
}


void nsGameApplication::LoadTestLevel_Boxes()
{
	nsAssetManager& assetManager = nsAssetManager::Get();

	{
		nsActor* floorActor = MainWorld->CreateActor("floor_actor", true, nsVector3(0.0f, -8.0f, 0.0f));
		nsBoxCollisionComponent* boxCollisionComp = floorActor->AddComponent<nsBoxCollisionComponent>("box_collision");
		boxCollisionComp->HalfExtents = nsVector3(1600.0f, 8.0f, 1600.0f);

		nsMeshComponent* meshComp = floorActor->AddComponent<nsMeshComponent>("mesh");
		meshComp->SetMesh(assetManager.LoadModelAsset(NS_ENGINE_ASSET_MODEL_DEFAULT_FLOOR_NAME));

		floorActor->SetRootComponent(boxCollisionComp);
		MainWorld->AddActorToLevel(floorActor);
	}


	nsSharedModelAsset wallModelAsset = assetManager.LoadModelAsset(NS_ENGINE_ASSET_MODEL_DEFAULT_WALL_NAME);
	{
		nsActor* wall0 = MainWorld->CreateActor("wall_0", true, nsVector3(-500.0f, 138.0f, 100.0f));
		nsBoxCollisionComponent* boxCollisionComp = wall0->AddComponent<nsBoxCollisionComponent>("box_collision");
		boxCollisionComp->HalfExtents = nsVector3(256.0f, 128.0f, 8.0f);
		nsMeshComponent* meshComp = wall0->AddComponent<nsMeshComponent>("mesh");
		meshComp->SetMesh(wallModelAsset);
		wall0->SetRootComponent(boxCollisionComp);
		MainWorld->AddActorToLevel(wall0);
	}
	{
		nsActor* wall1 = MainWorld->CreateActor("wall_1", true, nsVector3(-617.0f, 128.0f, -272.0f));
		nsBoxCollisionComponent* boxCollisionComp = wall1->AddComponent<nsBoxCollisionComponent>("box_collision");
		boxCollisionComp->HalfExtents = nsVector3(256.0f, 128.0f, 8.0f);
		nsMeshComponent* meshComp = wall1->AddComponent<nsMeshComponent>("mesh");
		meshComp->SetMesh(wallModelAsset);
		wall1->SetRootComponent(boxCollisionComp);
		MainWorld->AddActorToLevel(wall1);
	}
	{
		nsActor* wall2 = MainWorld->CreateActor("wall_2", true, nsVector3(-741.0f, 128.0f, 100.0f), nsQuaternion::FromRotation(0.0f, 90.0f, 0.0f));
		nsBoxCollisionComponent* boxCollisionComp = wall2->AddComponent<nsBoxCollisionComponent>("box_collision");
		boxCollisionComp->HalfExtents = nsVector3(256.0f, 128.0f, 8.0f);
		nsMeshComponent* meshComp = wall2->AddComponent<nsMeshComponent>("mesh");
		meshComp->SetMesh(wallModelAsset);
		wall2->SetRootComponent(boxCollisionComp);
		MainWorld->AddActorToLevel(wall2);
	}

	nsSharedModelAsset platformModelAsset = assetManager.LoadModelAsset(NS_ENGINE_ASSET_MODEL_DEFAULT_PLATFORM_NAME);
	{
		nsActor* slide0 = MainWorld->CreateActor("slide_0", true, nsVector3(-617.0f, 128.0f, -485.0f), nsQuaternion::FromRotation(0.0f, 0.0f, -30.0f));
		nsBoxCollisionComponent* boxCollisionComp = slide0->AddComponent<nsBoxCollisionComponent>("box_collision");
		boxCollisionComp->HalfExtents = nsVector3(256.0f, 8.0f, 128.0f);
		nsMeshComponent* meshComp = slide0->AddComponent<nsMeshComponent>("mesh");
		meshComp->SetMesh(platformModelAsset);
		slide0->SetRootComponent(boxCollisionComp);
		MainWorld->AddActorToLevel(slide0);
	}
	{
		nsActor* slide1 = MainWorld->CreateActor("slide_1", true, nsVector3(-592.0f, 186.0f, -850.0f), nsQuaternion::FromRotation(0.0f, 0.0f, -45.0f));
		nsBoxCollisionComponent* boxCollisionComp = slide1->AddComponent<nsBoxCollisionComponent>("box_collision");
		boxCollisionComp->HalfExtents = nsVector3(256.0f, 8.0f, 128.0f);
		nsMeshComponent* meshComp = slide1->AddComponent<nsMeshComponent>("mesh");
		meshComp->SetMesh(platformModelAsset);
		slide1->SetRootComponent(boxCollisionComp);
		MainWorld->AddActorToLevel(slide1);
	}


	nsSharedModelAsset boxModelAsset = assetManager.LoadModelAsset(NS_ENGINE_ASSET_MODEL_DEFAULT_BOX_NAME);

	nsActor* boxCenter = MainWorld->CreateActor("box_center", true, nsVector3());
	{
		nsMeshComponent* meshComp = boxCenter->AddComponent<nsMeshComponent>("mesh");
		meshComp->SetMesh(boxModelAsset);

		boxCenter->SetRootComponent(meshComp);
		MainWorld->AddActorToLevel(boxCenter);
	}

	nsActor* capsuleS = MainWorld->CreateActor("capsule_static", true, nsVector3(-50.0f, 100.0f, -10.0f));
	{
		nsCapsuleCollisionComponent* collisionComp = capsuleS->AddComponent<nsCapsuleCollisionComponent>("capsule_collision");
		capsuleS->SetRootComponent(collisionComp);

		MainWorld->AddActorToLevel(capsuleS);
	}

	nsNavigationManager::Get().BuildNavMesh(MainWorld);

	nsActor* capsuleD = MainWorld->CreateActor("capsule_dynamic", false, nsVector3(-50.0f, 800.0f, -10.0f));
	{
		nsCapsuleCollisionComponent* collisionComp = capsuleD->AddComponent<nsCapsuleCollisionComponent>("capsule_collision");
		capsuleD->SetRootComponent(collisionComp);

		MainWorld->AddActorToLevel(capsuleD);
	}

	// ================================================================================================================= //
	// Random boxes
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
				nsActor* actor = MainWorld->CreateActor(nsString::Format(TEXT("box_actor_%i"), count++), false, spawnPosition, nsQuaternion::FromRotation(nsMath::RandomInRange(-30.0f, 30.0f), nsMath::RandomInRange(-80.0f, 80.0f), 0.0f));
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
