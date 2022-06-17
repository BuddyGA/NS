#include "nsGUIFramework.h"
#include "nsConsole.h"



// ================================================================================================================================================================================ //
// GUI - WINDOW
// ================================================================================================================================================================================ //
nsGUIWindow::nsGUIWindow() noexcept
{
	Name = "";
	InitialRect = nsGUIRect(0.0f, 0.0f, 256.0f, 128.0f);
	BackgroundColor = NS_GUI_DEFAULT_COLOR_BACKGROUND;
	BorderColorDefault = NS_GUI_DEFAULT_COLOR_BORDER;
	BorderColorActive = NS_GUI_DEFAULT_COLOR_BORDER_ACTIVE;
	BorderWidth = 4.0f;

	Title = "None";
	TitleColor = NS_GUI_DEFAULT_COLOR_TEXT;
	TitleFont = nsFontID::INVALID;
	TitleBarHeight = 24.0f;

	bTitleBar = 1;
	bResizable = 1;
	bMoveable = 1;

	Rect = InitialRect;
	bIsFirstDraw = true;
	bResizing = false;
	bMoving = false;
}


void nsGUIWindow::UpdatePosition(nsGUIContext& context) noexcept
{
	if (!bMoveable || nsPlatform::Mouse_IsCursorHidden())
	{
		return;
	}

	if (bMoving)
	{
		if (context.IsMouseButtonReleased(0))
		{
			bMoving = false;
		}
		else
		{
			const nsPointFloat deltaMousePosition = context.GetMouseDeltaPosition();
			const float width = Rect.GetWidth();
			const float height = Rect.GetHeight();
			Rect.Left += deltaMousePosition.X;
			Rect.Top += deltaMousePosition.Y;
			Rect.Right = Rect.Left + width;
			Rect.Bottom = Rect.Top + height;
		}
	}
	else if (context.IsCurrentRegionHovered())
	{
		if (TitleBarControl.Interactions & nsEGUIRectInteraction::Pressed)
		{
			bMoving = true;
		}
	}
}


void nsGUIWindow::UpdateSize(nsGUIContext& context) noexcept
{
	if (!bResizable || bMoving || nsPlatform::Mouse_IsCursorHidden())
	{
		return;
	}

	if (bResizing)
	{
		if (context.IsMouseButtonReleased(0))
		{
			bResizing = false;
		}
		else
		{
			const nsPointFloat deltaMousePosition = context.GetMouseDeltaPosition();

			switch (ResizeBorder)
			{
				case EResizeBorder::LEFT:
				{
					Rect.Left += deltaMousePosition.X;
					break;
				}

				case EResizeBorder::TOP:
				{
					Rect.Top += deltaMousePosition.Y;
					break;
				}

				case EResizeBorder::RIGHT:
				{
					Rect.Right += deltaMousePosition.X;
					break;
				}

				case EResizeBorder::BOTTOM:
				{
					Rect.Bottom += deltaMousePosition.Y;
					break;
				}

				case EResizeBorder::BOTTOM_RIGHT:
				{
					Rect.Right += deltaMousePosition.X;
					Rect.Bottom += deltaMousePosition.Y;
					break;
				}

				default:
					break;
			}

			if (Rect.GetWidth() < 100.0f)
			{
				Rect.Right = Rect.Left + 100.0f;
			}

			if (Rect.GetHeight() < 100.0f)
			{
				Rect.Bottom = Rect.Top + 100.0f;
			}
		}
	}
	else if (context.IsCurrentRegionHovered())
	{
		if (LeftBorderControl.Interactions & nsEGUIRectInteraction::Hovered)
		{
			ResizeBorder = EResizeBorder::LEFT;
		}
		else if (TopBorderControl.Interactions & nsEGUIRectInteraction::Hovered)
		{
			ResizeBorder = EResizeBorder::TOP;
		}
		else if (RightBorderControl.Interactions & nsEGUIRectInteraction::Hovered)
		{
			ResizeBorder = EResizeBorder::RIGHT;
		}
		else if (BottomBorderControl.Interactions & nsEGUIRectInteraction::Hovered)
		{
			ResizeBorder = EResizeBorder::BOTTOM;
		}
		else if (ResizeBorderControl.Interactions & nsEGUIRectInteraction::Hovered)
		{
			ResizeBorder = EResizeBorder::BOTTOM_RIGHT;
		}
		else
		{
			ResizeBorder = EResizeBorder::NONE;
		}

		if (LeftBorderControl.Interactions & nsEGUIRectInteraction::Pressed ||
			TopBorderControl.Interactions & nsEGUIRectInteraction::Pressed ||
			RightBorderControl.Interactions & nsEGUIRectInteraction::Pressed ||
			BottomBorderControl.Interactions & nsEGUIRectInteraction::Pressed ||
			ResizeBorderControl.Interactions & nsEGUIRectInteraction::Pressed)
		{
			bResizing = true;
		}
	}
	else
	{
		ResizeBorder = EResizeBorder::NONE;
	}

	switch (ResizeBorder)
	{
		case EResizeBorder::LEFT: nsPlatform::Mouse_SetCursorShape(nsEMouseCursorShape::SIZE_WE); break;
		case EResizeBorder::TOP: nsPlatform::Mouse_SetCursorShape(nsEMouseCursorShape::SIZE_NS); break;
		case EResizeBorder::RIGHT: nsPlatform::Mouse_SetCursorShape(nsEMouseCursorShape::SIZE_WE); break;
		case EResizeBorder::BOTTOM: nsPlatform::Mouse_SetCursorShape(nsEMouseCursorShape::SIZE_NS); break;
		case EResizeBorder::BOTTOM_RIGHT: nsPlatform::Mouse_SetCursorShape(nsEMouseCursorShape::SIZE_NWSE); break;
		default: break;
	}
}


void nsGUIWindow::BeginDraw(nsGUIContext& context) noexcept
{
	if (Name.GetLength() == 0)
	{
		static int _counter = 0;
		Name = nsName::Format("#%i_gui_window", _counter++);
	}

	if (bIsFirstDraw)
	{
		bIsFirstDraw = false;
		Rect = InitialRect;
	}

	context.BeginRegion(nullptr, Rect, nsPointFloat(), nsEGUIElementLayout::NONE, nsEGUIScrollOption::None, true);
	const nsPointFloat windowDimension(Rect.GetWidth(), Rect.GetHeight());

	LeftBorderControl.Rect.Left = 0.0f;
	LeftBorderControl.Rect.Top = 0.0f;
	LeftBorderControl.Rect.Right = LeftBorderControl.Rect.Left + BorderWidth;
	LeftBorderControl.Rect.Bottom = LeftBorderControl.Rect.Top + windowDimension.Y;

	TopBorderControl.Rect.Left = LeftBorderControl.Rect.Right;
	TopBorderControl.Rect.Top = LeftBorderControl.Rect.Top;
	TopBorderControl.Rect.Right = TopBorderControl.Rect.Left + windowDimension.X - (BorderWidth * 2.0f);
	TopBorderControl.Rect.Bottom = TopBorderControl.Rect.Top + BorderWidth;

	RightBorderControl.Rect.Left = TopBorderControl.Rect.Right;
	RightBorderControl.Rect.Top = TopBorderControl.Rect.Top;
	RightBorderControl.Rect.Right = RightBorderControl.Rect.Left + BorderWidth;
	RightBorderControl.Rect.Bottom = RightBorderControl.Rect.Top + windowDimension.Y - BorderWidth;

	BottomBorderControl.Rect.Left = TopBorderControl.Rect.Left;
	BottomBorderControl.Rect.Top = LeftBorderControl.Rect.Bottom - BorderWidth;
	BottomBorderControl.Rect.Right = RightBorderControl.Rect.Left;
	BottomBorderControl.Rect.Bottom = BottomBorderControl.Rect.Top + BorderWidth;

	ResizeBorderControl.Rect.Left = RightBorderControl.Rect.Left;
	ResizeBorderControl.Rect.Top = BottomBorderControl.Rect.Top;
	ResizeBorderControl.Rect.Right = RightBorderControl.Rect.Right;
	ResizeBorderControl.Rect.Bottom = BottomBorderControl.Rect.Bottom;

	if (bTitleBar)
	{
		TitleBarControl.Rect.Left = TopBorderControl.Rect.Left;
		TitleBarControl.Rect.Top = TopBorderControl.Rect.Bottom;
		TitleBarControl.Rect.Right = TopBorderControl.Rect.Right;
		TitleBarControl.Rect.Bottom = TitleBarControl.Rect.Top + TitleBarHeight;
	}

	context.UpdateControlInCurrentRegion(LeftBorderControl, false);
	context.UpdateControlInCurrentRegion(TopBorderControl, false);
	context.UpdateControlInCurrentRegion(RightBorderControl, false);
	context.UpdateControlInCurrentRegion(BottomBorderControl, false);
	context.UpdateControlInCurrentRegion(ResizeBorderControl, false);

	bool bTitleBarPressed = false;

	if (bTitleBar)
	{
		context.UpdateControlInCurrentRegion(TitleBarControl, false);
		bTitleBarPressed = TitleBarControl.Interactions & nsEGUIRectInteraction::Pressed;
	}

	const nsColor useBorderColor = context.IsCurrentWindowRegionActive() ? BorderColorActive : BorderColorDefault;
	DrawBorderControl(context, LeftBorderControl, useBorderColor);
	DrawBorderControl(context, TopBorderControl, useBorderColor);
	DrawBorderControl(context, RightBorderControl, useBorderColor);
	DrawBorderControl(context, BottomBorderControl, useBorderColor);
	DrawBorderControl(context, ResizeBorderControl, useBorderColor);

	if (bTitleBar)
	{
		DrawBorderControl(context, TitleBarControl, useBorderColor);
		context.AddDrawTextOnRect(*Title, Title.GetLength(), TitleBarControl.Rect, nsEGUIAlignmentHorizontal::LEFT, nsEGUIAlignmentVertical::CENTER, nsPointFloat(4.0f, 0.0f));
	}

	nsGUIRect backgroundRect;
	backgroundRect.Left = TopBorderControl.Rect.Left;
	backgroundRect.Top = bTitleBar ? TitleBarControl.Rect.Bottom : TopBorderControl.Rect.Bottom;
	backgroundRect.Right = TopBorderControl.Rect.Right;
	backgroundRect.Bottom = BottomBorderControl.Rect.Top;
	context.AddDrawRect(backgroundRect, BackgroundColor);
}


void nsGUIWindow::EndDraw(nsGUIContext& context) noexcept
{
	UpdatePosition(context);
	UpdateSize(context);

	context.EndRegion();
}




// ================================================================================================================================================================================ //
// GUI - BUTTON
// ================================================================================================================================================================================ //
nsGUIButton::nsGUIButton()
{
	Size = nsPointFloat(100.0f, 22.0f);
	DefaultColor = nsColor(30, 60, 100);
	HoveredColor = nsColor(60, 120, 200);
	PressedColor = nsColor::WHITE;
	TextColor = nsColor::WHITE;
}


bool nsGUIButton::Draw(nsGUIContext& context)
{
	nsGUIControl buttonControl;
	buttonControl.Rect.Left = 0.0f;
	buttonControl.Rect.Top = 0.0f;
	buttonControl.Rect.Right = buttonControl.Rect.Left + Size.X;
	buttonControl.Rect.Bottom = buttonControl.Rect.Top + Size.Y;

	context.UpdateControlInCurrentRegion(buttonControl, false);

	if (!buttonControl.bIsVisible)
	{
		return false;
	}

	nsColor useColor = DefaultColor;
	const bool bReleased = buttonControl.Interactions & nsEGUIRectInteraction::Released;

	if (bReleased)
	{
		useColor = PressedColor;
	}
	else if (buttonControl.Interactions & nsEGUIRectInteraction::Hovered)
	{
		useColor = HoveredColor;
	}

	context.AddDrawRect(buttonControl.Rect, useColor);
	const int textLength = Text.GetLength();

	if (textLength > 0)
	{
		context.AddDrawTextOnRect(*Text, textLength, buttonControl.Rect, nsEGUIAlignmentHorizontal::CENTER, nsEGUIAlignmentVertical::CENTER, nsPointFloat(), TextColor);
	}

	return bReleased;
}




// ================================================================================================================================================================================ //
// GUI - TEXT BOX
// ================================================================================================================================================================================ //
nsGUITextBox::nsGUITextBox() noexcept
{
	DragValue = 0.0f;
	CaretCharIndex = 0;
	SelectStartCharIndex = -1;
	SelectedCharIndex = -1;
	SelectedCharCount = 0;
	InputState = EInputState::NONE;
	bComputeHighlightRect = false;

	Size = nsPointFloat(150.0f, 30.0f);
	BackgroundColor = NS_GUI_DEFAULT_COLOR_BACKGROUND;
	TextColor = NS_GUI_DEFAULT_COLOR_TEXT;
	HighlightColor = NS_GUI_DEFAULT_COLOR_TEXT_HIGHLIGHT;
	CaretColor = nsColor::WHITE;
	DragSpeed = 1.0f;
	bEnabled = true;
	bDragEnabled = false;
}


bool nsGUITextBox::RemoveSelectedChars() noexcept
{
	bool bRemoved = false;

	if (SelectedCharIndex >= 0 && SelectedCharCount > 0)
	{
		const int length = TextValue.GetLength();
		NS_Assert(SelectedCharIndex >= 0 && SelectedCharIndex < length);
		NS_Assert(SelectedCharIndex + SelectedCharCount <= length);
		bRemoved = TextValue.RemoveAtRange(SelectedCharIndex, SelectedCharCount);
		SelectedCharIndex = -1;
		SelectedCharCount = 0;
	}

	return bRemoved;
}


bool nsGUITextBox::IsValidChar(char c) noexcept
{
	if (c == '\0' || 
		c == '\b' /*backspace*/ || 
		c == '\r' /*enter*/ || 
		c == '\n' /*newline*/ || 
		c == 27 /*escape*/)
	{
		return false;
	}

	return true;
}


bool nsGUITextBox::Draw(nsGUIContext& context) noexcept
{
	if (Name.GetLength() == 0)
	{
		static int _counter = 0;
		Name = nsName::Format("#%i_gui_textbox", _counter++);
	}

	//NS_LogDebug(GUILog, "InputState-begin: [%s] %i", *Name, static_cast<int>(InputState));

	bool bCommitEnter = false;
	bool bCommitDrag = false;
	nsGUIControl& control = context.AddControlUnique(*Name, Size);

	if (bEnabled)
	{
		const int currentCharLength = TextValue.GetLength();

		if (context.IsCurrentRegionHovered() && (control.Interactions & nsEGUIRectInteraction::Hovered))
		{
			if (context.IsMouseButtonPressed(0))
			{
				//NS_LogDebug(GUILog, "Mouse L pressed: [%s]", *Name);
				InputState = EInputState::TYPING;
				context.SetControlInputFocus(*Name);
				CaretCharIndex = currentCharLength;
			}
			else if (context.IsMouseButtonPressed(2) && bDragEnabled)
			{
				//NS_LogDebug(GUILog, "Mouse R pressed: [%s]", *Name);
				InputState = EInputState::BEGIN_DRAG;
				context.SetControlInputFocus(*Name);
			}

			if (InputState == EInputState::TYPING)
			{
				nsPlatform::Mouse_SetCursorShape(nsEMouseCursorShape::BEAM);
			}
			else if (bDragEnabled)
			{
				nsPlatform::Mouse_SetCursorShape(nsEMouseCursorShape::SIZE_WE);
			}
		}

		if (context.IsControlFocused(*Name))
		{
			bool bUnselectChars = false;

			const char* charInput = context.GetCharInput();

			if (charInput[0] != '\0' && // null
				charInput[0] != 27 && // escape
				IsValidChar(charInput[0]))
			{
				RemoveSelectedChars();

				if (TextValue.InsertAt(charInput, CaretCharIndex))
				{
					CaretCharIndex++;
				}
			}

			if (context.IsMouseButtonDoubleClicked(0) && currentCharLength > 0)
			{
				CaretCharIndex = currentCharLength;
				SelectedCharIndex = 0;
				SelectedCharCount = CaretCharIndex;
				bComputeHighlightRect = true;
			}

			if (InputState == EInputState::TYPING && context.IsMouseDragging(0))
			{
				//NS_LogDebug(GUILog, "Start selecting [mouse]: [%s]", *Name);
				InputState = EInputState::SELECTING;
			}
			else if (InputState == EInputState::BEGIN_DRAG && context.IsMouseDragging(2))
			{
				//NS_LogDebug(GUILog, "Start dragging: [%s]", *Name);
				InputState = EInputState::DRAGGING;
			}

			if (InputState == EInputState::TYPING)
			{
				if (currentCharLength > 0)
				{
					if (context.IsKeyboardButtonPressed(nsEInputKey::KEYBOARD_SHIFT_LEFT))
					{
						//NS_LogDebug(GUILog, "START selecting [keyboard]: [%s]", *Name);
						InputState = EInputState::SELECTING;
						SelectStartCharIndex = CaretCharIndex;
					}
					else
					{
						if (context.IsKeyboardButtonPressed(nsEInputKey::KEYBOARD_ARROW_LEFT))
						{
							if (SelectStartCharIndex != -1 && SelectedCharCount > 0)
							{
								CaretCharIndex = nsMath::Min(CaretCharIndex, SelectStartCharIndex);
								SelectStartCharIndex = -1;
							}
							else
							{
								CaretCharIndex--;
							}

							bUnselectChars = true;
						}
						else if (context.IsKeyboardButtonPressed(nsEInputKey::KEYBOARD_ARROW_RIGHT))
						{
							if (SelectStartCharIndex != -1 && SelectedCharCount > 0)
							{
								CaretCharIndex = nsMath::Max(CaretCharIndex, SelectStartCharIndex);
								SelectStartCharIndex = -1;
							}
							else
							{
								CaretCharIndex++;
							}

							bUnselectChars = true;
						}
						else if (context.IsKeyboardButtonPressed(nsEInputKey::KEYBOARD_HOME))
						{
							CaretCharIndex = 0;
							SelectStartCharIndex = -1;
							bUnselectChars = true;
						}
						else if (context.IsKeyboardButtonPressed(nsEInputKey::KEYBOARD_END))
						{
							CaretCharIndex = currentCharLength;
							SelectStartCharIndex = -1;
							bUnselectChars = true;
						}
					}
				}
			}
			else if (InputState == EInputState::SELECTING)
			{
				if (context.IsKeyboardButtonReleased(nsEInputKey::KEYBOARD_SHIFT_LEFT))
				{
					InputState = EInputState::TYPING;
					//NS_LogDebug(GUILog, "STOP selecting [keyboard]: [%s]", *Name);
				}
				else
				{
					//NS_LogDebug(GUILog, "UPDATE selecting [keyboard]: [%s]", *Name);

					if (context.IsKeyboardButtonPressed(nsEInputKey::KEYBOARD_ARROW_LEFT))
					{
						if (SelectStartCharIndex > 0 && CaretCharIndex > 0)
						{
							CaretCharIndex--;
							SelectedCharIndex = CaretCharIndex;
							SelectedCharCount = SelectStartCharIndex - CaretCharIndex;
							bComputeHighlightRect = true;
						}
					}
					else if (context.IsKeyboardButtonPressed(nsEInputKey::KEYBOARD_ARROW_RIGHT))
					{
						if (SelectStartCharIndex < currentCharLength && CaretCharIndex < currentCharLength)
						{
							CaretCharIndex++;
							SelectedCharIndex = SelectStartCharIndex;
							SelectedCharCount++;
							bComputeHighlightRect = true;
						}
					}
					else if (context.IsKeyboardButtonPressed(nsEInputKey::KEYBOARD_HOME))
					{
						if (SelectStartCharIndex > 0)
						{
							SelectedCharIndex = 0;
							SelectedCharCount = SelectStartCharIndex;
							CaretCharIndex = 0;
							bComputeHighlightRect = true;
						}
					}
					else if (context.IsKeyboardButtonPressed(nsEInputKey::KEYBOARD_END))
					{
						if (SelectStartCharIndex < currentCharLength)
						{
							SelectedCharIndex = SelectStartCharIndex;
							SelectedCharCount = currentCharLength - SelectedCharIndex;
							CaretCharIndex = currentCharLength;
							bComputeHighlightRect = true;
						}
					}
				}
			}
			else if (InputState == EInputState::DRAGGING)
			{
				const nsPointFloat deltaMousePosition = context.GetMouseDeltaPosition();

				nsPlatform::Mouse_SetCursorShape(nsEMouseCursorShape::SIZE_WE);
				DragValue = deltaMousePosition * DragSpeed;
				bCommitDrag = true;
				//NS_LogDebug(GUILog, "Commit drag value: [%s] [%f, %f]", *Name, DragValue.X, DragValue.Y);

				if (context.IsMouseButtonReleased(2))
				{
					DragValue = nsPointFloat();
					InputState = EInputState::TYPING;
					CaretCharIndex = TextValue.GetLength();
					bUnselectChars = true;
				}
			}

			if (context.IsKeyboardButtonPressed(nsEInputKey::KEYBOARD_ESCAPE))
			{
				InputState = EInputState::NONE;
				context.SetControlInputFocus(nullptr);
				SelectStartCharIndex = -1;
				bUnselectChars = true;
			}
			else if (context.IsKeyboardButtonPressed(nsEInputKey::KEYBOARD_BACKSPACE))
			{
				InputState = EInputState::TYPING;

				if (RemoveSelectedChars())
				{
					CaretCharIndex = nsMath::Min(CaretCharIndex, SelectStartCharIndex);
					SelectStartCharIndex = -1;
				}
				else if (CaretCharIndex > 0)
				{
					TextValue.RemoveAt(--CaretCharIndex);
				}
			}
			else if (context.IsKeyboardButtonPressed(nsEInputKey::KEYBOARD_DELETE))
			{
				InputState = EInputState::TYPING;

				if (RemoveSelectedChars())
				{
					CaretCharIndex = nsMath::Min(CaretCharIndex, SelectStartCharIndex);
					SelectStartCharIndex = -1;
				}
				else if (CaretCharIndex >= 0 && CaretCharIndex < currentCharLength)
				{
					TextValue.RemoveAt(CaretCharIndex);
				}
			}
			else if (context.IsKeyboardButtonPressed(nsEInputKey::KEYBOARD_ENTER))
			{
				InputState = EInputState::TYPING;
				SelectStartCharIndex = -1;
				bCommitEnter = true;
				bUnselectChars = true;
			}

			if (bUnselectChars)
			{
				SelectedCharIndex = -1;
				SelectedCharCount = 0;
			}
		}
		else
		{
			InputState = EInputState::NONE;
			SelectStartCharIndex = -1;
		}

	}

	CaretCharIndex = nsMath::Clamp(CaretCharIndex, 0, TextValue.GetLength());
	const bool bShowCaret = (!bDragEnabled && context.IsControlFocused(*Name)) || (InputState == EInputState::TYPING || InputState == EInputState::SELECTING);

	if (control.bIsVisible)
	{
		const nsFontID useFont = Font == nsFontID::INVALID ? context.GetDefaultFont() : Font;

		context.AddDrawRect(control.Rect, BackgroundColor);
		const nsGUIRect textRect = context.AddDrawTextOnRect(*TextValue, TextValue.GetLength(), control.Rect, nsEGUIAlignmentHorizontal::LEFT, nsEGUIAlignmentVertical::CENTER, nsPointFloat(2.0f, 0.0f), TextColor, useFont);

		if (bShowCaret)
		{
			if (SelectedCharCount > 0)
			{
				if (bComputeHighlightRect)
				{
					bComputeHighlightRect = false;

					const nsRectFloat selectedRect = nsFontManager::CalculateSelectedRect(useFont, nsPointFloat(textRect.Left, textRect.Top), *TextValue, TextValue.GetLength(), SelectedCharIndex, SelectedCharCount);
					HighlightRect.Left = selectedRect.X;
					HighlightRect.Top = selectedRect.Y;
					HighlightRect.Right = HighlightRect.Left + selectedRect.Width;
					HighlightRect.Bottom = HighlightRect.Top + selectedRect.Height;
				}

				context.AddDrawRect(HighlightRect, HighlightColor, nsTextureID::INVALID, nsMaterialID::INVALID, 1);
			}

			const nsPointFloat caretSize(1.0f, nsFontManager::GetFontSize(useFont) + 2.0f);
			nsGUIRect caretRect;
			caretRect.Left = control.Rect.Left + 1.0f + nsFontManager::CalculateCaretPositionX(useFont, TextValue, CaretCharIndex);
			caretRect.Top = control.Rect.Top + (control.Rect.GetHeight() * 0.5f) - (caretSize.Y * 0.5f);
			caretRect.Right = caretRect.Left + caretSize.X;
			caretRect.Bottom = caretRect.Top + caretSize.Y;

			context.AddDrawRect(caretRect, CaretColor, nsTextureID::INVALID, nsMaterialID::INVALID, 2);
		}
	}

	//NS_LogDebug(GUILog, "InputState-end: [%s] %i", *Name, static_cast<int>(InputState));

	return bCommitEnter || bCommitDrag;
}




// ================================================================================================================================================================================ //
// GUI - INPUT FLOAT
// ================================================================================================================================================================================ //
nsGUIInputFloat::nsGUIInputFloat() noexcept
{
	BackgroundColor = nsColor(50, 50, 50, 255);
	bDragEnabled = true;
	MinValue = -FLT_MAX;
	MaxValue = FLT_MAX;
	FloatValue = 0.0f;
}


bool nsGUIInputFloat::IsValidChar(char c) noexcept
{
	bool bValid = nsGUITextBox::IsValidChar(c);
	
	if (bValid)
	{
		bValid = (c >= '0' && c <= '9') || (c == '.') || (c == '-');
	}

	return bValid;
}


bool nsGUIInputFloat::Draw(nsGUIContext& context) noexcept
{
	if (!IsEditing())
	{
		const nsPointFloat dragValue = GetDragValue();
		FloatValue = nsMath::Clamp(FloatValue + dragValue.X, MinValue, MaxValue);
		TextValue = nsString::Format("%.3f", FloatValue);
		ClearDragValue();
	}

	const bool bCommitted = nsGUITextBox::Draw(context);

	if (bCommitted)
	{
		FloatValue = TextValue.ToFloat();
	}

	return bCommitted;
}




// ================================================================================================================================================================================ //
// GUI - INPUT VECTOR3
// ================================================================================================================================================================================ //
nsGUIInputVector3::nsGUIInputVector3() noexcept
{
	Size = nsPointFloat(0.0f, 22.0f);
	BackgroundColor = nsColor(50, 50, 50, 255);
	MinValue = -FLT_MAX;
	MaxValue = FLT_MAX;
	DragSpeed = 1.0f;
}


bool nsGUIInputVector3::Draw(nsGUIContext& context, nsVector3& outValue) noexcept
{
	if (Name.GetLength() == 0)
	{
		static int _counter = 0;
		Name = nsName::Format("#%i_gui_input_vector3", _counter++);
	}

	bool bInputCommitted = false;
	context.PushRegionElementLayout(nsEGUIElementLayout::HORIZONTAL);

	const nsGUIControl testControl = context.TestControlInCurrentRegion(Size);

	if (testControl.bIsVisible)
	{
		const nsPointFloat childElementSpace = context.GetCurrentRegionChildElementSpace();
		const float controlWidth = testControl.Rect.GetWidth() - (childElementSpace.X * 2.0f);
		const float widthPerInput = (controlWidth / 3.0f);
		bool bCommitteds[3];

		for (int i = 0; i < 3; ++i)
		{
			nsGUIInputFloat& inputFloat = InputFloats[i];
			inputFloat.Name = nsName::Format("%s_%i", *Name, i);
			inputFloat.Size = nsPointFloat(widthPerInput, Size.Y);
			inputFloat.BackgroundColor = BackgroundColor;
			inputFloat.MinValue = MinValue;
			inputFloat.MaxValue = MaxValue;
			inputFloat.DragSpeed = DragSpeed;

			/*
			if (!inputFloat.IsEditing())
			{
				const nsPointFloat dragValue = inputFloat.GetDragValue();
				const float floatValue = nsMath::Clamp(outValue[i] + dragValue.X, inputFloat.MinValue, inputFloat.MaxValue);
				inputFloat.Value = nsString::Format("%.3f", floatValue);
				//NS_LogDebug(GUILog, "InputFloats[%i] value: %f", i, InputFloats[i].Value);
				inputFloat.ClearDragValue();
			}
			*/

			if (!inputFloat.IsEditing())
			{
				inputFloat.FloatValue = outValue[i];
			}

			bCommitteds[i] = inputFloat.Draw(context);

			if (bCommitteds[i])
			{
				outValue[i] = inputFloat.FloatValue;
			}
		}

		bInputCommitted = bCommitteds[0] || bCommitteds[1] || bCommitteds[2];
	}

	context.PopRegionElementLayout();

	return bInputCommitted;
}




// ================================================================================================================================================================================ //
// GUI - TABLE
// ================================================================================================================================================================================ //
nsGUITable::nsGUITable() noexcept
{
	Size = nsPointFloat(400.0f, 300.0f);
	BackgroundColor = NS_GUI_DEFAULT_COLOR_BACKGROUND;
	SeparatorColor = NS_GUI_DEFAULT_COLOR_BORDER;
	MinColumnWidth = 32.0f;
	SeparatorWidth = 3.0f;
	bShowHeader = false;
	bIsFirstDraw = true;
	FocusSeparatorIndex = -1;
	bResizingColumn = false;
}


void nsGUITable::UpdateColumnSeparatorPosition(nsGUIContext& context) noexcept
{
	if (context.IsMouseButtonReleased(0) || nsPlatform::Mouse_IsCursorHidden() || bIsFirstDraw)
	{
		bResizingColumn = false;
		return;
	}

	if (bResizingColumn)
	{
		const nsPointFloat mouseDelta = context.GetMouseDeltaPosition();
		Column& column = Columns[FocusSeparatorIndex];
		const float prevRectRight = column.Rect.Right;
		column.Rect.Right += mouseDelta.X;

		// Drag to left
		if (mouseDelta.X < 0.0f)
		{
			// Constraint column width
			if (column.Rect.GetWidth() <= MinColumnWidth)
			{
				column.Rect.Right = column.Rect.Left + MinColumnWidth;
			}
		}
		// Drag to right
		else if (mouseDelta.X > 0.0f)
		{
			nsGUIRect separatorRect = ColumnSeparatorControls[FocusSeparatorIndex].Rect;
			separatorRect.Left = column.Rect.Right;
			separatorRect.Right = separatorRect.Left + SeparatorWidth;

			nsGUIRect nextColumnRect = Columns[FocusSeparatorIndex + 1].Rect;
			nextColumnRect.Left = separatorRect.Right;

			// If next column width equals MinColumnWidth, ignore resize
			if (nextColumnRect.GetWidth() <= MinColumnWidth)
			{
				column.Rect.Right = prevRectRight;
			}
		}
	}
	else
	{
		int checkSeparatorIndex = -1;

		for (int i = 0; i < ColumnSeparatorControls.GetCount(); ++i)
		{
			const nsGUIControl& control = ColumnSeparatorControls[i];

			if (control.Interactions & nsEGUIRectInteraction::Hovered)
			{
				checkSeparatorIndex = i;
				break;
			}
		}

		FocusSeparatorIndex = checkSeparatorIndex;
	}

	if (FocusSeparatorIndex != -1)
	{
		if (!bResizingColumn && (ColumnSeparatorControls[FocusSeparatorIndex].Interactions & nsEGUIRectInteraction::Pressed) )
		{
			bResizingColumn = true;
		}

		nsPlatform::Mouse_SetCursorShape(nsEMouseCursorShape::SIZE_WE);
	}
}


void nsGUITable::BeginDraw(nsGUIContext& context) noexcept
{
	if (Name.GetLength() == 0)
	{
		static int _counter = 0;
		Name = nsName::Format("#%i_gui_table", _counter++);
	}

	if (SeparatorWidth <= 0.0f)
	{
		SeparatorWidth = 1.0f;
	}

	if (Columns.GetCount() == 0)
	{
		Columns.Resize(1);
	}

	const int columnCount = Columns.GetCount();
	const int separatorCount = columnCount - 1;
	ColumnSeparatorControls.Resize(separatorCount);

	UpdateColumnSeparatorPosition(context);

	const nsGUIControl regionControl = context.TestControlInCurrentRegion(Size);
	const float regionWidth = regionControl.Rect.GetWidth();
	const float regionHeight = regionControl.Rect.GetHeight();

	context.BeginRegion(nullptr, nsGUIRect(0.0f, 0.0f, Size.X, Size.Y), nsPointFloat(), nsEGUIElementLayout::NONE, nsEGUIScrollOption::None, false);

	const nsPointFloat childElementSpace = context.GetCurrentRegionChildElementSpace();
	const float widthUsedByBorders = SeparatorWidth * separatorCount;
	const float remainingWidth = regionWidth - widthUsedByBorders - (childElementSpace.X * columnCount);
	float leftOffset = 0.0f;

	for (int i = 0; i < columnCount; ++i)
	{
		Column& col = Columns[i];
		col.Rect.Left = leftOffset;
		col.Rect.Top = 0.0f;

		if (bIsFirstDraw)
		{
			col.ActualWidth = remainingWidth * col.WidthPercentage - childElementSpace.X;
			col.Rect.Right = col.Rect.Left + col.ActualWidth;
		}
		else if (i == (columnCount - 1))
		{
			col.Rect.Right = col.Rect.Left + (regionWidth - col.Rect.Left);
		}

		col.Rect.Bottom = col.Rect.Top + regionHeight;

		leftOffset += col.Rect.GetWidth();

		if (i < separatorCount)
		{
			nsGUIControl& columnSeparatorControl = ColumnSeparatorControls[i];
			columnSeparatorControl.Rect.Left = leftOffset;
			columnSeparatorControl.Rect.Top = 4.0f;
			columnSeparatorControl.Rect.Right = columnSeparatorControl.Rect.Left + SeparatorWidth;
			columnSeparatorControl.Rect.Bottom = columnSeparatorControl.Rect.Top + regionHeight - 8.0f;

			context.UpdateControlInCurrentRegion(columnSeparatorControl, false);
			context.AddDrawRect(columnSeparatorControl.Rect, SeparatorColor);

		#ifdef _DEBUG
			if (context.bDrawDebugHoveredRect && context.IsCurrentRegionHovered() && (columnSeparatorControl.Interactions & nsEGUIRectInteraction::Hovered))
			{
				context.AddDrawDebugRectLine(columnSeparatorControl.Rect, nsColor::GREEN);
			}
		#endif // _DEBUG

			leftOffset += SeparatorWidth;
		}
	}
}


void nsGUITable::EndDraw(nsGUIContext& context) noexcept
{
	context.EndRegion();

	bIsFirstDraw = false;
}


void nsGUITable::BeginColumn(nsGUIContext& context, int columnIndex, const nsPointFloat& elementSpace) noexcept
{
	NS_AssertV(columnIndex < Columns.GetCount(), "columnIndex out of range!");

	Column& col = Columns[columnIndex];

	if (col.Name.GetLength() == 0)
	{
		col.Name = nsName::Format("%s.column_%i", *Name, columnIndex);
	}

	context.BeginRegion(*col.Name, Columns[columnIndex].Rect, elementSpace, nsEGUIElementLayout::VERTICAL, nsEGUIScrollOption::Scrollable_Y, false);
}


void nsGUITable::EndColumn(nsGUIContext& context)
{
	context.EndRegion();
}



// ================================================================================================================================================================================ //
// GUI - CONSOLE WINDOW
// ================================================================================================================================================================================ //
nsFontID nsGUIConsoleWindow::ConsoleFont;


nsGUIConsoleWindow::nsGUIConsoleWindow() noexcept
{
	BackgroundColor = NS_GUI_DEFAULT_COLOR_BACKGROUND;
	InputTextHeight = 22.0f;

	Window.InitialRect = nsGUIRect(16.0f, 16.0f, 416.0f, 316.0f);
	Window.BorderColorActive = Window.BorderColorDefault;
	Window.BorderWidth = 4.0f;
	Window.bTitleBar = false;
	Window.bResizable = false;
	Window.bMoveable = false;

	PrevLogEntryCount = 0;
	bOpened = false;
	bJustOpened = false;
	bJustClosed = false;
}


void nsGUIConsoleWindow::Draw(nsGUIContext& context) noexcept
{
	if (bOpened)
	{
		if (Name.GetLength() == 0)
		{
			static int _counter = 0;
			Name = nsName::Format("#%i_gui_console", _counter++);
		}
		
		if (bJustOpened)
		{
			Window.Name = nsName::Format("%s_window", *Name);
			LogAreaName = nsName::Format("%s_log_area", *Name);
			InputText.Name = nsName::Format("%s_input_text", *Name);
		}

		Window.BackgroundColor = BackgroundColor;
		Window.BorderColorActive = Window.BorderColorDefault;

		const nsGUIRect& canvasRect = context.GetCanvasRect();
		Window.SetRect(nsGUIRect(4.0f, 4.0f, canvasRect.GetWidth() * 0.5f, canvasRect.GetHeight() * 0.5f));

		Window.BeginDraw(context);
		{
			const nsGUIRect contentRect = Window.GetContentRect();

			context.BeginRegion(nullptr, contentRect, nsPointFloat(), nsEGUIElementLayout::VERTICAL, nsEGUIScrollOption::None, false);
			{
				nsConsoleManager& consoleManager = nsConsoleManager::Get();
				const nsTArray<nsConsoleLogEntry>& logEntries = consoleManager.GetLogEntries();
				const bool bScrollToEndLine = bJustOpened || PrevLogEntryCount < logEntries.GetCount();
				PrevLogEntryCount = logEntries.GetCount();

				// Log area
				const nsGUIRect logAreaRect(0.0f, 0.0f, contentRect.GetWidth(), contentRect.GetHeight() - InputTextHeight - 4.0f);
				const nsGUIScrollOptions scrollOptions = bScrollToEndLine ? (nsEGUIScrollOption::Scrollable_Y | nsEGUIScrollOption::AutoScroll_Y) : nsEGUIScrollOption::Scrollable_Y;
				context.BeginRegion(*LogAreaName, logAreaRect, nsPointFloat(2.0f), nsEGUIElementLayout::VERTICAL, scrollOptions, "console_logs");
				{
					const nsTArray<char>& logChars = consoleManager.GetLogChars();

					for (int i = 0; i < logEntries.GetCount(); ++i)
					{
						context.AddControlText(&logChars[logEntries[i].CharIndex], logEntries[i].Color, ConsoleFont);
					}
				}
				context.EndRegion();

				// Separator
				context.AddControlRect(logAreaRect.Right, 4.0f, Window.BorderColorDefault);

				// Input
				InputText.Size = nsPointFloat(logAreaRect.GetWidth(), InputTextHeight);
				InputText.Font = ConsoleFont;

				if (InputText.Draw(context))
				{
					const nsString command = InputText.TextValue.ToLower();

					if (command == "close")
					{
						Close();
					}
					else
					{
						consoleManager.ExecuteCommand(command);
					}

					InputText.Clear();
				}
			}
			context.EndRegion();
		}
		Window.EndDraw(context);

		if (bJustOpened)
		{
			bJustOpened = false;
			InputText.Focus(context);
		}
	}

	if (bJustClosed)
	{
		bJustClosed = false;
		InputText.Clear();
		context.SetControlInputFocus(nullptr);
	}
}
