#pragma once

#include "nsGUICore.h"


#define NS_GUI_DEFAULT_COLOR_BACKGROUND			nsColor(30, 30, 30, 230)
#define NS_GUI_DEFAULT_COLOR_BORDER				nsColor(10, 20, 30, 230)
#define NS_GUI_DEFAULT_COLOR_BORDER_ACTIVE		nsColor(30, 60, 80, 230)
#define NS_GUI_DEFAULT_COLOR_TEXT				nsColor(255, 255, 255, 255)
#define NS_GUI_DEFAULT_COLOR_TEXT_HIGHLIGHT		nsColor(50, 80, 200, 255)



class NS_ENGINE_API nsGUIWindow
{
private:
	enum class EResizeBorder : uint8
	{
		NONE = 0,
		LEFT,
		TOP,
		RIGHT,
		BOTTOM,
		BOTTOM_RIGHT
	};

public:
	nsName Name;

	nsGUIRect InitialRect;
	nsColor BackgroundColor;
	nsColor BorderColorDefault;
	nsColor BorderColorActive;
	float BorderWidth;

	nsName Title;
	nsColor TitleColor;
	nsFontID TitleFont;
	float TitleBarHeight;

	uint8 bTitleBar : 1;
	uint8 bResizable : 1;
	uint8 bMoveable : 1;

private:
	nsGUIRect Rect;
	nsGUIControl LeftBorderControl;
	nsGUIControl TopBorderControl;
	nsGUIControl RightBorderControl;
	nsGUIControl BottomBorderControl;
	nsGUIControl ResizeBorderControl;
	nsGUIControl TitleBarControl;
	bool bIsFirstDraw;
	bool bResizing;
	bool bMoving;
	EResizeBorder ResizeBorder;


public:
	nsGUIWindow() noexcept;

private:
	void UpdatePosition(nsGUIContext& context) noexcept;
	void UpdateSize(nsGUIContext& context) noexcept;


	NS_INLINE void DrawBorderControl(nsGUIContext& context, const nsGUIControl& borderControl, const nsColor& color) noexcept
	{
		if (borderControl.bIsVisible)
		{
			context.AddDrawRect(borderControl.Rect, color);

		#ifdef NS_ENGINE_DEBUG_DRAW
			if (bResizable && context.bDrawDebugHoveredRect && context.IsCurrentRegionHovered() && (borderControl.Interactions & nsEGUIRectInteraction::Hovered) )
			{
				context.AddDrawDebugRectLine(borderControl.Rect, nsColor::GREEN);
			}
		#endif // NS_ENGINE_DEBUG_DRAW
		}
	}


public:
	void BeginDraw(nsGUIContext& context) noexcept;
	void EndDraw(nsGUIContext& context) noexcept;


	NS_INLINE void SetRect(const nsGUIRect& rect) noexcept
	{
		InitialRect = rect;
		Rect = rect;
	}


	NS_NODISCARD_INLINE nsGUIRect GetContentRect() const noexcept
	{
		nsGUIRect contentRect;
		contentRect.Left = BorderWidth;
		contentRect.Top = bTitleBar ? BorderWidth + TitleBarHeight : BorderWidth;
		contentRect.Right = contentRect.Left + Rect.GetWidth() - LeftBorderControl.Rect.GetWidth() - RightBorderControl.Rect.GetWidth();
		contentRect.Bottom = contentRect.Top + Rect.GetHeight() - TopBorderControl.Rect.GetHeight() - BottomBorderControl.Rect.GetHeight();

		if (bTitleBar)
		{
			contentRect.Bottom -= TitleBarHeight;
		}

		return contentRect;
	}

};



class NS_ENGINE_API nsGUILabel
{
public:
	nsName Text;
	nsColor Color;
	nsFontID Font;
};



class NS_ENGINE_API nsGUIButton
{
public:
	nsPointFloat Size;
	nsColor DefaultColor;
	nsColor HoveredColor;
	nsColor PressedColor;
	nsName Text;
	nsColor TextColor;


public:
	nsGUIButton();
	bool Draw(nsGUIContext& context);

};



class NS_ENGINE_API nsGUITextBox
{
	enum class EInputState : uint8
	{
		NONE = 0,
		TYPING,
		SELECTING,
		BEGIN_DRAG,
		DRAGGING,
	};

private:
	nsGUIRect HighlightRect;
	nsPointFloat DragValue;
	int CaretCharIndex;
	int SelectStartCharIndex;
	int SelectedCharIndex;
	int SelectedCharCount;
	EInputState InputState;
	bool bComputeHighlightRect;

public:
	nsName Name;
	nsPointFloat Size;
	nsColor BackgroundColor;
	nsColor TextColor;
	nsColor HighlightColor;
	nsColor CaretColor;
	nsFontID Font;
	nsString TextValue;
	float DragSpeed;
	bool bEnabled;
	bool bDragEnabled;


public:
	nsGUITextBox() noexcept;
	
private:
	bool RemoveSelectedChars() noexcept;

protected:
	virtual bool IsValidChar(char c) noexcept;

public:
	virtual bool Draw(nsGUIContext& context) noexcept;


	NS_INLINE void Clear() noexcept
	{
		TextValue.Clear();
		CaretCharIndex = 0;
	}


	NS_INLINE void Focus(nsGUIContext& context) noexcept
	{
		context.SetControlInputFocus(*Name);
		InputState = EInputState::TYPING;
	}


	NS_NODISCARD_INLINE bool IsEditing() const noexcept
	{
		return InputState == EInputState::TYPING;
	}


	NS_NODISCARD_INLINE void ClearDragValue() noexcept
	{
		DragValue = nsPointFloat();
	}


	NS_NODISCARD_INLINE nsPointFloat GetDragValue() const noexcept
	{
		return DragValue;
	}

};



class NS_ENGINE_API nsGUIInputFloat : public nsGUITextBox
{
public:
	float MinValue;
	float MaxValue;
	float FloatValue;


public:
	nsGUIInputFloat() noexcept;
	virtual bool IsValidChar(char c) noexcept;
	virtual bool Draw(nsGUIContext& context) noexcept override;

};



class NS_ENGINE_API nsGUIInputVector3
{
public:
	nsName Name;
	nsPointFloat Size;
	nsColor BackgroundColor;
	float MinValue;
	float MaxValue;
	float DragSpeed;

private:
	nsGUIInputFloat InputFloats[3];


public:
	nsGUIInputVector3() noexcept;
	bool Draw(nsGUIContext& context, nsVector3& outValue) noexcept;

};



class NS_ENGINE_API nsGUITable
{
	struct Column
	{
		nsName Name;
		nsGUIRect Rect;
		float ActualWidth;
		float WidthPercentage;
	};


public:
	nsName Name;
	nsPointFloat Size;
	nsColor BackgroundColor;
	nsColor SeparatorColor;
	float MinColumnWidth;
	float SeparatorWidth;
	bool bShowHeader;
	bool bIsFirstDraw;


private:
	nsTArrayInline<Column, 8> Columns;
	nsTArrayInline<nsGUIControl, 7> ColumnSeparatorControls;
	nsTArray<float> RowHeights;
	int FocusSeparatorIndex;
	bool bResizingColumn;


public:
	nsGUITable() noexcept;

private:
	void UpdateColumnSeparatorPosition(nsGUIContext& context) noexcept;

public:
	void BeginDraw(nsGUIContext& context) noexcept;
	void EndDraw(nsGUIContext& context) noexcept;
	void BeginColumn(nsGUIContext& context, int columnIndex, const nsPointFloat& elementSpace = nsPointFloat()) noexcept;
	void EndColumn(nsGUIContext& context);

	NS_INLINE void AddColumn(nsName header, float widthPercentage) noexcept
	{
		Column& col = Columns.Add();
		col.Name = header;
		col.WidthPercentage = widthPercentage;
	}

};




class NS_ENGINE_API nsGUIConsoleWindow
{
public:
	nsName Name;
	nsColor BackgroundColor;
	float InputTextHeight;

private:
	static nsFontID ConsoleFont;

	nsGUIWindow Window;
	nsGUITextBox InputText;
	nsName LogAreaName;
	int PrevLogEntryCount;
	bool bOpened;
	bool bJustOpened;
	bool bJustClosed;


public:
	nsGUIConsoleWindow() noexcept;
	void Draw(nsGUIContext& context) noexcept;


	NS_INLINE void Open() noexcept
	{
		if (ConsoleFont == nsFontID::INVALID)
		{
			ConsoleFont = nsFontManager::CreateFontTTF("../../../Assets/Fonts/ShareTechMono_Regular.ttf", 15.0f);
		}

		if (!bOpened)
		{
			bOpened = true;
			bJustOpened = true;
		}
	}


	NS_INLINE void Close() noexcept
	{
		if (bOpened)
		{
			bOpened = false;
			bJustClosed = true;
		}
	}


	NS_INLINE void Toggle() noexcept
	{
		if (bOpened)
		{
			Close();
		}
		else
		{
			Open();
		}
	}


	NS_NODISCARD_INLINE bool IsOpened() const noexcept
	{
		return bOpened;
	}

};
