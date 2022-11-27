#include "cstGame.h"
#include "cstCharacter.h"
#include "cstAbility.h"
#include "nsPhysicsManager.h"
#include "nsRenderManager.h"
#include "nsWorld.h"

#ifdef CST_GAME_WITH_EDITOR
#include "Editor/nsEditor.h"
#endif // CST_GAME_WITH_EDITOR



cstGameplaySettings::cstGameplaySettings()
{
	ResetSettings();
}


void cstGameplaySettings::ResetSettings()
{
	CameraDistance = 1000.0f;
	CameraMoveSpeed = 1000.0f;
}


void cstGameplaySettings::SaveSettings()
{

}


void cstGameplaySettings::LoadSettings()
{

}





cstGame::cstGame(const wchar_t* title, int width, int height, nsEWindowFullscreenMode fullscreenMode) noexcept
	: nsGameApplication(title, width, height, fullscreenMode)
{
	PendingChangeState = cstEGameState::NONE;
	CurrentState = cstEGameState::NONE;

	CameraTransform = nsTransform();
	CameraTransform.Rotation = nsQuaternion::FromRotation(60.0f, -45.0f, 0.0f);
	CameraMoveAxis = nsVector2::ZERO;
	bCameraPanning = false;

	FocusedCharacterIndex = 0;

#ifdef CST_GAME_WITH_EDITOR
	bShowDebugFocusedCharacter = true;
	bDebugDrawBorders = true;
#endif // CST_GAME_WITH_EDITOR

}


void cstGame::Initialize() noexcept
{
	nsGameApplication::Initialize();

	GameplaySettings.LoadSettings();

	InputManager.LoadBindings();
	InputManager.ActionBindingPressedDelegate.Bind(this, &cstGame::OnInputActionBindingPressed);
	InputManager.ActionBindingConflictedDelegate.Bind(this, &cstGame::OnInputActionBindingConflicted);

	cstCharacter* playerChar0 = MainWorld->CreateActor<cstCharacter>("player_char_0", false, nsVector3(0.0f, 100.0f, -300.0f));
	{
		playerChar0->AddOwningTags(cstTag::Character_Player);
		MainWorld->AddActorToLevel(playerChar0);
		PlayerCharacters.Add(playerChar0);

		cstWeapon* weapon = MainWorld->CreateActor<cstWeapon>("player_char_0_weapon", false);
		MainWorld->AddActorToLevel(weapon);

		playerChar0->EquipWeapon(weapon);
	}
	

	cstPlayerAbilitySlots& player0_AbilitySlots = PlayerAbilitySlots[0];
	player0_AbilitySlots.Resize(cstInputAction::ABILITY_SLOT_MAX_COUNT);
	player0_AbilitySlots[ cstInputAction::GetAbilitySlotIndex(cstInputAction::ABILITY_SLOT_STOP) ] = ns_GetDefaultObjectAs<cstAbility_Stop>(cstAbility_Stop::Class);

	cstPlayerItemSlots& player0_ItemSlots = PlayerItemSlots[0];
	player0_ItemSlots.Resize(cstInputAction::ITEM_SLOT_MAX_COUNT);



	// ========= DUMMY ENEMIES ========= //
	{
		cstCharacter* enemyChar0 = MainWorld->CreateActor<cstCharacter>("enemy_char_0", false, nsVector3(330.0f, 100.0f, -300.0f));
		enemyChar0->AddOwningTags(cstTag::Character_Enemy);
		MainWorld->AddActorToLevel(enemyChar0);
		PlayerCharacters.Add(enemyChar0);

		cstWeapon* weapon = MainWorld->CreateActor<cstWeapon>("enemy_char_0_weapon", false);
		MainWorld->AddActorToLevel(weapon);

		enemyChar0->EquipWeapon(weapon);
	}

	{
		cstCharacter* enemyChar1 = MainWorld->CreateActor<cstCharacter>("enemy_char_1", false, nsVector3(330.0f, 100.0f, -800.0f));
		enemyChar1->AddOwningTags(cstTag::Character_Enemy);
		MainWorld->AddActorToLevel(enemyChar1);
		PlayerCharacters.Add(enemyChar1);

		cstWeapon* weapon = MainWorld->CreateActor<cstWeapon>("enemy_char_1_weapon", false);
		MainWorld->AddActorToLevel(weapon);

		enemyChar1->EquipWeapon(weapon);
	}
	// ================================= //


#ifdef CST_GAME_WITH_EDITOR
	g_Editor = ns_CreateObject<nsEditor>(this);
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


	switch (CurrentState)
	{
		case cstEGameState::INTRO:
		{
			NS_ValidateV(0, TEXT("Not implemented yet!"));
			break;
		}


		case cstEGameState::MAIN_MENU:
		{
			NS_ValidateV(0, TEXT("Not implemented yet!"));
			break;
		}


		case cstEGameState::LOADING:
		{
			NS_ValidateV(0, TEXT("Not implemented yet!"));
			break;
		}


		case cstEGameState::PLAYING:
		{
			nsVector3 cameraMoveDirection = CameraTransform.GetAxisRight() * CameraMoveAxis.X;
			cameraMoveDirection += CameraTransform.GetAxisUp() * CameraMoveAxis.Y;
			cameraMoveDirection = cameraMoveDirection - nsVector3::Project(cameraMoveDirection, nsVector3::UP);
			cameraMoveDirection.Normalize();
			cameraMoveDirection *= GameplaySettings.CameraMoveSpeed;

			nsVector3 newCameraPosition = CameraTransform.Position + cameraMoveDirection;
			CameraTransform.Position = nsVector3::Lerp(CameraTransform.Position, newCameraPosition, deltaTime);

			CameraMoveAxis = nsVector2::ZERO;
			MainViewport.SetViewTransform(CameraTransform);

			break;
		}


		case cstEGameState::IN_GAME_MENU:
		{
			NS_ValidateV(0, TEXT("Not implemented yet!"));
			break;
		}


		case cstEGameState::PAUSE_MENU:
		{
			NS_ValidateV(0, TEXT("Not implemented yet!"));
			break;
		}


	#ifdef CST_GAME_WITH_EDITOR
		case cstEGameState::EDITING:
		{
			g_Editor->TickUpdate(deltaTime);

			break;
		}
	#endif // CST_GAME_WITH_EDITOR


		default: break;
	}


	if (PendingChangeState != cstEGameState::NONE && PendingChangeState != CurrentState)
	{
		switch (CurrentState)
		{
			case cstEGameState::INTRO: EndState_Intro(); break;
			case cstEGameState::MAIN_MENU: EndState_MainMenu(); break;
			case cstEGameState::LOADING: EndState_Loading(); break;
			case cstEGameState::PLAYING: EndState_Playing(); break;
			case cstEGameState::IN_GAME_MENU: EndState_InGameMenu(); break;
			case cstEGameState::PAUSE_MENU: EndState_PauseMenu(); break;

		#ifdef CST_GAME_WITH_EDITOR
			case cstEGameState::EDITING: EndState_Editing(); break;
		#endif // CST_GAME_WITH_EDITOR

			default: break;
		}


		switch (PendingChangeState)
		{
			case cstEGameState::INTRO: BeginState_Intro(); break;
			case cstEGameState::MAIN_MENU: BeginState_MainMenu(); break;
			case cstEGameState::LOADING: BeginState_Loading(); break;
			case cstEGameState::PLAYING: BeginState_Playing(); break;
			case cstEGameState::IN_GAME_MENU: BeginState_InGameMenu(); break;
			case cstEGameState::PAUSE_MENU: BeginState_PauseMenu(); break;

		#ifdef CST_GAME_WITH_EDITOR
			case cstEGameState::EDITING: BeginState_Editing(); break;
		#endif // CST_GAME_WITH_EDITOR

			default: break;
		}

		CurrentState = PendingChangeState;
		PendingChangeState = cstEGameState::NONE;
	}
}


void cstGame::PreRender() noexcept
{
	nsRenderContextWorld& renderContext = nsRenderManager::Get().GetWorldRenderContext(MainWorld);

#ifdef CST_GAME_WITH_EDITOR
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

	if (CurrentState == cstEGameState::PLAYING)
	{
		const nsVector2 mousePosition(static_cast<float>(e.Position.X), static_cast<float>(e.Position.Y));
		nsPhysicsManager::Get().SceneQueryMousePicking(MainWorld->GetPhysicsScene(), MouseRayHitResult, mousePosition, &MainViewport);
	}

#ifdef CST_GAME_WITH_EDITOR
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
		if (e.ButtonState == nsEButtonState::PRESSED && e.Key == nsEInputKey::MOUSE_RIGHT && FocusedCharacterIndex != -1 && MouseRayHitResult.Actor)
		{
			cstCharacter* attackTargetCharacter = ns_Cast<cstCharacter>(MouseRayHitResult.Actor);

			if (attackTargetCharacter && attackTargetCharacter->IsAlive())
			{
				PlayerCharacters[FocusedCharacterIndex]->CommandAttack(attackTargetCharacter);
			}
			else
			{
				PlayerCharacters[FocusedCharacterIndex]->CommandMove(MouseRayHitResult.WorldPosition);
			}
		}
	}

#ifdef CST_GAME_WITH_EDITOR
	else if (CurrentState == cstEGameState::EDITING)
	{
		g_Editor->OnMouseButton(e);
	}
#endif // CST_GAME_WITH_EDITOR
}


void cstGame::OnMouseWheel(const nsMouseWheelEventArgs& e) noexcept
{
	nsGameApplication::OnMouseWheel(e);

#ifdef CST_GAME_WITH_EDITOR
	if (CurrentState == cstEGameState::EDITING)
	{
		g_Editor->OnMouseWheel(e);
	}
#endif // CST_GAME_WITH_EDITOR
}


void cstGame::OnKeyboardButton(const nsKeyboardButtonEventArgs& e) noexcept
{
	nsGameApplication::OnKeyboardButton(e);

	if (CurrentState == cstEGameState::PLAYING)
	{
		InputManager.KeyboardButtonEvent(e);
	}


#ifdef CST_GAME_WITH_EDITOR
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


#ifdef CST_GAME_WITH_EDITOR
	if (CurrentState == cstEGameState::PLAYING)
	{
		static nsString debugText;
		debugText = nsString::Format(TEXT("Hovering: %s"), MouseRayHitResult.Actor ? *MouseRayHitResult.Actor->Name : TEXT("-"));
		context.AddDrawText(*debugText, debugText.GetLength(), nsPointFloat(32.0f, 32.0f));

		if (bShowDebugFocusedCharacter && FocusedCharacterIndex != -1)
		{
			PlayerCharacters[FocusedCharacterIndex]->DebugGUI(context);
		}

		if (bDebugDrawBorders)
		{
			context.AddDrawRect(borderTop, nsColor::GRAY);
			context.AddDrawRect(borderLeft, nsColor::GRAY);
			context.AddDrawRect(borderRight, nsColor::GRAY);
			context.AddDrawRect(borderBottom, nsColor::GRAY);
		}
	}
	else if (CurrentState == cstEGameState::EDITING)
	{
		g_Editor->DrawGUI(context);
	}
#endif // CST_GAME_WITH_EDITOR
}


void cstGame::OnInputActionBindingPressed(cstInputAction::EType inputActionType, bool bIsPressed, bool bIsDoubleClick)
{
	if (inputActionType == cstInputAction::CAMERA_PANNING)
	{
		NS_CONSOLE_Log(cstGameLog, TEXT("%s camera panning"), bIsPressed ? TEXT("Begin") : TEXT("End"));
		bCameraPanning = bIsPressed;
	}


	if (bIsPressed)
	{
		if (inputActionType == cstInputAction::MENU_INVENTORY)
		{
			NS_CONSOLE_Log(cstGameLog, TEXT("Toggle inventory menu"));
		}
		else if (inputActionType == cstInputAction::MENU_CHARACTER)
		{
			NS_CONSOLE_Log(cstGameLog, TEXT("Toggle character menu"));
		}
		else if (inputActionType == cstInputAction::MENU_ABILITY)
		{
			NS_CONSOLE_Log(cstGameLog, TEXT("Toggle ability menu"));
		}
		else if (inputActionType == cstInputAction::MENU_PAUSE)
		{
			NS_CONSOLE_Log(cstGameLog, TEXT("Toggle pause menu"));
		}


		if (inputActionType == cstInputAction::CHARACTER_0)
		{
			NS_CONSOLE_Log(cstGameLog, TEXT("Set character focus [0]"));
			FocusedCharacterIndex = 0;
		}
		else if (inputActionType == cstInputAction::CHARACTER_1)
		{
			NS_CONSOLE_Log(cstGameLog, TEXT("Set character focus [1]"));
			FocusedCharacterIndex = 1;
		}
		else if (inputActionType == cstInputAction::CHARACTER_2)
		{
			NS_CONSOLE_Log(cstGameLog, TEXT("Set character focus [2]"));
			FocusedCharacterIndex = 2;
		}
		else if (inputActionType == cstInputAction::CHARACTER_TOGGLE_FOCUS)
		{
			NS_CONSOLE_Log(cstGameLog, TEXT("Toggle character focus from all selected characters"));
		}
		else if (inputActionType == cstInputAction::CHARACTER_SELECT_ALL)
		{
			NS_CONSOLE_Log(cstGameLog, TEXT("Select all characters"));
		}

		FocusedCharacterIndex = nsMath::Clamp(FocusedCharacterIndex, 0, PlayerCharacters.GetCount() - 1);


		if (FocusedCharacterIndex != -1)
		{
			const wchar_t* inputActionName = cstInputAction::NAMES[inputActionType];

			if (inputActionType >= cstInputAction::ABILITY_SLOT_STOP && inputActionType <= cstInputAction::ABILITY_SLOT_7)
			{
				if (bIsDoubleClick)
				{
					NS_CONSOLE_Log(cstGameLog, TEXT("Target ability slot [%s] to self"), inputActionName);
				}
				else
				{
					NS_CONSOLE_Log(cstGameLog, TEXT("Begin targeting for ability slot [%s]"), inputActionName);
				}
			}

			if (inputActionType >= cstInputAction::ITEM_SLOT_0 && inputActionType <= cstInputAction::ITEM_SLOT_7)
			{
				if (bIsDoubleClick)
				{
					NS_CONSOLE_Log(cstGameLog, TEXT("Target item slot [%s] to self"), inputActionName);
				}
				else
				{
					NS_CONSOLE_Log(cstGameLog, TEXT("Begin targeting for item slot [%s]"), inputActionName);
				}
			}
		}


		if (inputActionType == cstInputAction::PRESETS_ABILITY_0)
		{
			NS_CONSOLE_Log(cstGameLog, TEXT("Change ability presets to [0]"));
		}
		else if (inputActionType == cstInputAction::PRESETS_ABILITY_1)
		{
			NS_CONSOLE_Log(cstGameLog, TEXT("Change ability presets to [1]"));
		}


		if (inputActionType == cstInputAction::PRESETS_ITEM_0)
		{
			NS_CONSOLE_Log(cstGameLog, TEXT("Change item presets to [0]"));
		}
		else if (inputActionType == cstInputAction::PRESETS_ITEM_1)
		{
			NS_CONSOLE_Log(cstGameLog, TEXT("Change item presets to [1]"));
		}
	}
}


void cstGame::OnInputActionBindingConflicted(cstInputAction::EType firstInputActionType, cstInputAction::EType secondInputActionType)
{
	NS_CONSOLE_Warning(cstGameLog, TEXT("Input action binding conflict! [First: %s, Second: %s]"), cstInputAction::NAMES[firstInputActionType], cstInputAction::NAMES[secondInputActionType]);
}


void cstGame::BeginState_Intro()
{

}


void cstGame::EndState_Intro()
{
}


void cstGame::BeginState_MainMenu()
{
}


void cstGame::EndState_MainMenu()
{
}


void cstGame::BeginState_Loading()
{
}


void cstGame::EndState_Loading()
{
}


void cstGame::BeginState_Playing()
{
	MainWorld->DispatchStartPlay();
	
	const nsPointFloat viewportDimension = MainViewport.GetDimension();
	
	nsRectInt clipMouseRect;
	clipMouseRect.X = 2;
	clipMouseRect.Y = 2;
	clipMouseRect.Width = static_cast<int>(viewportDimension.X - 4.0f);
	clipMouseRect.Height = static_cast<int>(viewportDimension.Y - 4.0f);
	
	ClipMouseCursor(clipMouseRect);

	CameraTransform.Position = nsVector3(500.0f, 1000.0f, -500.0f);

	if (FocusedCharacterIndex != -1)
	{
		const nsVector3 characterPosition = PlayerCharacters[FocusedCharacterIndex]->GetWorldPosition();
		CameraTransform.Position.X += characterPosition.X;
		CameraTransform.Position.Y += characterPosition.Y;
		CameraTransform.Position.Z += characterPosition.Z;
	}
}


void cstGame::EndState_Playing()
{

}


void cstGame::BeginState_InGameMenu()
{
}


void cstGame::EndState_InGameMenu()
{
}


void cstGame::BeginState_PauseMenu()
{

}


void cstGame::EndState_PauseMenu()
{

}



#ifdef CST_GAME_WITH_EDITOR

void cstGame::BeginState_Editing()
{
	ClipMouseCursor(false);
	g_Editor->MainViewport = &MainViewport;
	g_Editor->MainRenderer = MainRenderer;
	g_Editor->MainWorld = MainWorld;
}


void cstGame::EndState_Editing()
{

}

#endif // CST_GAME_WITH_EDITOR
