#pragma once

#include "nsVulkan.h"
#include "nsFont.h"



extern nsLogCategory GUILog;


namespace nsEGUIRectInteraction
{
	enum Flag
	{
		None = (0),
		Hovered = (1 << 0),
		Pressed = (1 << 1),
		Released = (1 << 2),
	};
};
typedef uint8 nsGUIRectInteractions;



namespace nsEGUIScrollOption
{
	enum Flag
	{
		None			= (0),
		Scrollable_X	= (1 << 0),
		Scrollable_Y	= (1 << 1),
		AutoScroll_X	= (1 << 2),
		AutoScroll_Y	= (1 << 3),
	};
};
typedef uint8 nsGUIScrollOptions;



enum class nsEGUIAlignmentHorizontal : uint8
{
	LEFT = 0,
	CENTER,
	RIGHT
};


enum class nsEGUIAlignmentVertical : uint8
{
	TOP = 0,
	CENTER,
	BOTTOM
};



enum class nsEGUIElementLayout : uint8
{
	NONE = 0,
	VERTICAL,
	HORIZONTAL,
};



struct nsGUIMouseState
{
	// Mouse cursor position relative to window
	nsPointFloat Position;

	// Mouse cursor position relative to window from previous frame
	nsPointFloat PrevPosition;

	// Drag start position
	nsPointFloat DragStartPosition;

	// Drag threshold. If mouse position went outside the threshold, begin drag operation
	nsPointFloat DragThreshold;

	// Mouse scroll value
	nsPointFloat ScrollValue;

	// Mouse scroll speed
	nsPointFloat ScrollSpeed;

	// Mouse button state on previous frame. [0]: Left, [1]: Middle, [2]: Right
	bool bWasPresseds[3];

	// Mouse button state on current frame. [0]: Left, [1]: Middle, [2]: Right
	bool bPresseds[3];

	// Mouse button double click
	bool bDoubleClicks[3];

	// Is dragging mouse while button is down? [0]: Left, [1]: Middle, [2]: Right
	bool bDraggings[3];



public:
	nsGUIMouseState() noexcept
		: Position()
		, PrevPosition()
		, DragStartPosition()
		, DragThreshold(4.0f)
		, ScrollValue()
		, ScrollSpeed(10.0f)
		, bWasPresseds()
		, bPresseds()
		, bDoubleClicks()
		, bDraggings()

	{
	}

};



struct nsGUIKeyboardState
{
	bool bPresseds[nsEInputKey::MAX_COUNT];
	bool bReleaseds[nsEInputKey::MAX_COUNT];

public:
	nsGUIKeyboardState() noexcept
		: bPresseds()
		, bReleaseds()
	{
	}

};



class nsGUIRect
{
public:
	float Left;
	float Top;
	float Right;
	float Bottom;


public:
	nsGUIRect() noexcept
		: Left(0.0f)
		, Top(0.0f)
		, Right(0.0f)
		, Bottom(0.0f)
	{
	}


	nsGUIRect(float l, float t, float r, float b) noexcept
		: Left(l)
		, Top(t)
		, Right(r)
		, Bottom(b)
	{
	}


	nsGUIRect(const nsRectFloat& r) noexcept
	{
		Left = r.X;
		Top = r.Y;
		Right = Left + r.Width;
		Bottom = Top + r.Height;
	}


	NS_NODISCARD_INLINE float GetWidth() const noexcept
	{
		return (Right - Left);
	}


	NS_NODISCARD_INLINE float GetHeight() const noexcept
	{
		return (Bottom - Top);
	}


	NS_NODISCARD_INLINE bool IsPointInside(float x, float y) const noexcept
	{
		return (x >= Left && x <= Right) && (y >= Top && y <= Bottom);
	}


	NS_NODISCARD_INLINE bool IsPointInside(const nsPointInt& point) const noexcept
	{
		return IsPointInside(static_cast<float>(point.X), static_cast<float>(point.Y));
	}


	NS_NODISCARD_INLINE bool IsPointInside(const nsPointFloat& point) const noexcept
	{
		return IsPointInside(point.X, point.Y);
	}


	NS_NODISCARD_INLINE bool IsRectInside(const nsGUIRect& rect) const noexcept
	{
		const bool bOutside = rect.Left > Right || rect.Top > Bottom || rect.Right < Left || rect.Bottom < Top;
		return !bOutside;
	}

};



class nsGUIControl
{
public:
	const char* Id;
	nsGUIRect Rect;
	bool bIsWindow;
	bool bIsVisible;
	nsGUIRectInteractions Interactions;
	nsGUIScrollOptions ScrollOptions;
	nsGUIRect ChildClipRect;
	nsEGUIElementLayout ChildElementLayout;
	nsPointFloat ChildElementSpace;
	nsPointFloat ChildElementOffset;
	nsGUIRect ChildContentRect;
	uint8 Order;

#ifdef _DEBUG
	nsName DebugName;
#endif // _DEBUG

public:
	nsGUIControl() noexcept
		: Id(nullptr)
		, Rect()
		, bIsWindow(false)
		, bIsVisible(false)
		, Interactions(nsEGUIRectInteraction::None)
		, ScrollOptions(nsEGUIScrollOption::None)
		, ChildClipRect()
		, ChildElementLayout(nsEGUIElementLayout::NONE)
		, ChildElementSpace()
		, ChildElementOffset()
		, ChildContentRect()
		, Order(0)
	{
	}


	nsGUIControl(float left, float top, float right, float bottom) noexcept
		: nsGUIControl()
	{
		Rect = nsGUIRect(left, top, right, bottom);
	}


	NS_NODISCARD_INLINE bool IsChildControlVisible(const nsGUIControl& child) const noexcept
	{
		return bIsVisible && ChildClipRect.IsRectInside(child.Rect);
	}

};



struct nsGUIDrawCallPerBatch
{
	int VertexCount;
	int BaseIndex;
	int IndexCount;
	int IndexVertexOffset;
	uint8 OrderZ;


public:
	nsGUIDrawCallPerBatch(uint8 order = 0) noexcept
		: OrderZ(order)
		, VertexCount(0)
		, BaseIndex(0)
		, IndexCount(0)
		, IndexVertexOffset(0)
	{
	}


	NS_INLINE bool operator==(const nsGUIDrawCallPerBatch& rhs) const noexcept
	{
		return OrderZ == rhs.OrderZ;
	}

};



struct nsGUIDrawCallPerMaterial
{
	nsMaterialID Material;
	nsTArrayInline<nsGUIDrawCallPerBatch, 4> Batches;


public:
	nsGUIDrawCallPerMaterial(nsMaterialID material = nsMaterialID::INVALID) noexcept
		: Material(material)
	{
	}


	NS_INLINE bool operator==(const nsGUIDrawCallPerMaterial& rhs) const noexcept
	{
		return Material == rhs.Material;
	}

};



struct nsGUIDrawCallPerRegion
{
	nsRectInt ClipRect;
	nsTArrayInline<nsGUIDrawCallPerMaterial, 16> Materials;
};



class NS_ENGINE_API nsGUIContext
{
private:
	struct Frame
	{
		nsVulkanBuffer* VertexBuffer;
		nsVulkanBuffer* IndexBuffer;
	};

	Frame FrameDatas[NS_ENGINE_FRAME_BUFFERING];
	int FrameIndex;

	nsGUIMouseState MouseState;
	nsGUIKeyboardState KeyboardState;
	char CharInput[8];

	nsPointFloat WindowDimension;
	nsFontID DefaultFont;
	nsMaterialID DefaultMaterialMesh;
	nsMaterialID DefaultMaterialFont;
	nsTextureID DefaultTextureWhite;
	nsTextureID DefaultTextureFont;


	struct MaterialInstance
	{
		nsMaterialID Material;
		nsTextureID Texture;
	};

	nsTMap<nsMaterialID, nsTArrayInline<MaterialInstance, 8>> MaterialInstanceTable;


	struct RegionData
	{
		nsGUIRect ChildContentRect;
		nsPointFloat ScrollValue;
	};

	nsTMap<const char*, RegionData> CachedRegionDatas;
	nsTArray<nsGUIControl> Controls;
	
	nsTArray<int> RegionStacks;
	int CurrentRegionId;
	int CurrentHoveredRegionId;
	int LastHoveredRegionId;
	int CurrentActiveWindowRegionId;

	int CurrentControlId;
	const char* CurrentControlInputFocus;
	nsTArrayInline<nsEGUIElementLayout, 16> ElementLayoutStacks;


	struct DrawData
	{
		int RegionId;
		nsMaterialID Material;
		int BaseVertex;
		int VertexCount;
		int BaseIndex;
		int IndexCount;
		uint8 OrderZ;
		uint8 bIsText;
	};

	nsTArray<nsVertexGUI> DrawVertices;
	nsTArray<uint32> DrawIndices;
	nsTArray<DrawData> DrawDatas;
	nsTArrayInline<nsMaterialID, 32> DrawBindMaterials;

	nsTArray<nsGUIDrawCallPerRegion> DrawCallData;


public:
	nsGUIContext() noexcept;
	~nsGUIContext() noexcept;
	void MouseMove(const nsMouseMoveEventArgs& e) noexcept;
	void MouseButton(const nsMouseButtonEventArgs& e) noexcept;
	void MouseWheel(const nsMouseWheelEventArgs& e) noexcept;
	void KeyboardButton(const nsKeyboardButtonEventArgs& e) noexcept;
	void AddCharInput(char c) noexcept;

private:
	NS_NODISCARD nsMaterialID GetOrCreateMaterialInstance(nsMaterialID material, nsTextureID texture) noexcept;
	NS_NODISCARD nsGUIRectInteractions TestInteraction(const nsGUIRect& rect) const noexcept;

public:
	void AddDrawShape(const nsVertexGUI* vertices, int vertexCount, const uint32* indices, int indexCount, nsTextureID texture = nsTextureID::INVALID, nsMaterialID material = nsMaterialID::INVALID, uint8 orderZ = 0) noexcept;
	void AddDrawTriangle(const nsPointFloat& center, float halfWidth, float rotationDegree, const nsColor& color, nsTextureID texture = nsTextureID::INVALID, nsMaterialID material = nsMaterialID::INVALID, uint8 orderZ = 0) noexcept;

	NS_INLINE void AddDrawTriangleLeft(const nsPointFloat& center, float halfWidth, const nsColor& color, nsTextureID texture = nsTextureID::INVALID, nsMaterialID material = nsMaterialID::INVALID, uint8 orderZ = 0) noexcept
	{
		AddDrawTriangle(center, halfWidth, -90.0f, color, texture, material, orderZ);
	}

	NS_INLINE void AddDrawTriangleRight(const nsPointFloat& center, float halfWidth, const nsColor& color, nsTextureID texture = nsTextureID::INVALID, nsMaterialID material = nsMaterialID::INVALID, uint8 orderZ = 0) noexcept
	{
		AddDrawTriangle(center, halfWidth, 90.0f, color, texture, material, orderZ);
	}

	NS_INLINE void AddDrawTriangleDown(const nsPointFloat& center, float halfWidth, const nsColor& color, nsTextureID texture = nsTextureID::INVALID, nsMaterialID material = nsMaterialID::INVALID, uint8 orderZ = 0) noexcept
	{
		AddDrawTriangle(center, halfWidth, 180.0f, color, texture, material, orderZ);
	}


	void AddDrawRect(const nsGUIRect& rect, const nsColor& color, nsTextureID texture = nsTextureID::INVALID, nsMaterialID material = nsMaterialID::INVALID, uint8 orderZ = 0, const nsVector2& uv0 = nsVector2(0.0f), const nsVector2& uv1 = nsVector2(1.0f)) noexcept;
	void AddDrawText(const char* text, int charLength, const nsPointFloat& position, const nsColor& color = nsColor::WHITE, nsFontID font = nsFontID::INVALID, nsMaterialID material = nsMaterialID::INVALID, uint8 orderZ = 0) noexcept;
	nsGUIRect AddDrawTextOnRect(const char* text, int charLength, const nsGUIRect& rect, nsEGUIAlignmentHorizontal hAlign, nsEGUIAlignmentVertical vAlign, const nsPointFloat& offsetAlignment = nsPointFloat(), const nsColor& color = nsColor::WHITE, nsFontID font = nsFontID::INVALID, nsMaterialID material = nsMaterialID::INVALID) noexcept;


	void BeginRegion(const char* uniqueId, const nsGUIRect& rect, const nsPointFloat& childElementSpace, nsEGUIElementLayout childElementLayout, nsGUIScrollOptions scrollOptions, bool bIsWindow, nsName debugName = "") noexcept;
	void EndRegion() noexcept;
	nsGUIControl TestControlInCurrentRegion(const nsPointFloat& size) const noexcept;
	void UpdateControlInCurrentRegion(nsGUIControl& control, bool bIsRegion) noexcept;

	nsGUIControl AddControlText(const char* text, nsColor color = nsColor::WHITE, nsFontID font = nsFontID::INVALID, nsMaterialID material = nsMaterialID::INVALID) noexcept;
	nsGUIControl AddControlTextOnRect(const char* text, const nsGUIRect& rect, nsEGUIAlignmentHorizontal hAlign, nsEGUIAlignmentVertical vAlign, const nsPointFloat& offsetAlignment = nsPointFloat(), const nsColor& color = nsColor::WHITE, nsFontID font = nsFontID::INVALID, nsMaterialID material = nsMaterialID::INVALID) noexcept;
	nsGUIControl AddControlRect(float width, float height, nsColor color = nsColor::WHITE, nsTextureID texture = nsTextureID::INVALID, nsMaterialID material = nsMaterialID::INVALID) noexcept;

	// Add empty control. UniqueId must be valid 
	NS_NODISCARD nsGUIControl& AddControlUnique(const char* uniqueId, const nsPointFloat& size) noexcept;

	void SetControlInputFocus(const char* controlUniqueId) noexcept;

	void BeginRender(const nsPointFloat& windowDimension, const nsGUIRect& canvasRect) noexcept;
	void EndRender() noexcept;
	void UpdateResourcesAndBuildDrawCalls(int frameIndex) noexcept;
	void ExecuteDrawCalls(VkCommandBuffer commandBuffer) noexcept;


	// Is mouse button pressed? [0]: Left, [1]: Middle, [2]: Right
	NS_NODISCARD_INLINE bool IsMouseButtonPressed(int buttonIndex) const noexcept
	{
		NS_Assert(buttonIndex >= 0 && buttonIndex <= 2);
		return !MouseState.bWasPresseds[buttonIndex] && MouseState.bPresseds[buttonIndex];
	}

	// Is mouse button held down? [0]: Left, [1]: Middle, [2]: Right
	NS_NODISCARD_INLINE bool IsMouseButtonDown(int buttonIndex) const noexcept
	{
		NS_Assert(buttonIndex >= 0 && buttonIndex <= 2);
		return MouseState.bWasPresseds[buttonIndex] && MouseState.bPresseds[buttonIndex];
	}

	// Is mouse button released? [0]: Left, [1]: Middle, [2]: Right
	NS_NODISCARD_INLINE bool IsMouseButtonReleased(int buttonIndex) const noexcept
	{
		NS_Assert(buttonIndex >= 0 && buttonIndex <= 2);
		return MouseState.bWasPresseds[buttonIndex] && !MouseState.bPresseds[buttonIndex];
	}

	// Is mouse button double clicked? [0]: Left, [1]: Middle, [2]: Right
	NS_NODISCARD_INLINE bool IsMouseButtonDoubleClicked(int buttonIndex) const noexcept
	{
		NS_Assert(buttonIndex >= 0 && buttonIndex <= 2);
		return MouseState.bDoubleClicks[buttonIndex];
	}

	// Is dragging mouse while button is down?
	NS_NODISCARD_INLINE bool IsMouseDragging(int buttonIndex) const noexcept
	{
		NS_Assert(buttonIndex >= 0 && buttonIndex <= 2);
		return MouseState.bDraggings[buttonIndex];
	}

	// Get mouse position
	NS_NODISCARD_INLINE nsPointFloat GetMousePosition() const noexcept
	{
		return MouseState.Position;
	}

	// Get mouse delta position
	NS_NODISCARD_INLINE nsPointFloat GetMouseDeltaPosition() const noexcept
	{
		return MouseState.Position - MouseState.PrevPosition;
	}


	// Is keyboard button pressed?
	NS_NODISCARD_INLINE bool IsKeyboardButtonPressed(nsInputKey key) const noexcept
	{
		return KeyboardState.bPresseds[key];
	}

	// Is keyboard button released?
	NS_NODISCARD_INLINE bool IsKeyboardButtonReleased(nsInputKey key) const noexcept
	{
		return KeyboardState.bReleaseds[key];
	}


	/*
	// Is keyboard button held down?
	NS_NODISCARD_INLINE bool IsKeyboardButtonDown(nsInputKey key) const noexcept
	{
		return KeyboardState.bWasPresseds[key] && KeyboardState.bPresseds[key];
	}

	// Is keyboard button released
	NS_NODISCARD_INLINE bool IsKeyboardButtonReleased(nsInputKey key) const noexcept
	{
		return KeyboardState.bWasPresseds[key] && !KeyboardState.bPresseds[key];
	}
	*/

	
	// Get char input
	NS_NODISCARD_INLINE const char* GetCharInput() const noexcept
	{
		return CharInput;
	}


	NS_NODISCARD_INLINE bool IsCurrentRegionHovered() const noexcept
	{
		return LastHoveredRegionId == CurrentRegionId;
	}


	// Returns true if current region is window and is active
	NS_NODISCARD_INLINE bool IsCurrentWindowRegionActive() const noexcept
	{
		return CurrentActiveWindowRegionId == CurrentRegionId;
	}


	NS_INLINE void PushRegionElementLayout(nsEGUIElementLayout layout) noexcept
	{
		ElementLayoutStacks.Add(layout);
		Controls[CurrentRegionId].ChildElementLayout = layout;
	}


	NS_INLINE void PopRegionElementLayout() noexcept
	{
		ElementLayoutStacks.RemoveAt(NS_ARRAY_INDEX_LAST);

		nsGUIControl& currentRegion = Controls[CurrentRegionId];
		const nsEGUIElementLayout layout = ElementLayoutStacks[ElementLayoutStacks.GetCount() - 1];

		if (layout == nsEGUIElementLayout::HORIZONTAL)
		{
			currentRegion.ChildElementOffset.X = currentRegion.ChildContentRect.Right + currentRegion.ChildElementSpace.X;
			currentRegion.ChildElementOffset.Y = currentRegion.Rect.Top + currentRegion.ChildElementSpace.Y;
		}
		else if (layout == nsEGUIElementLayout::VERTICAL)
		{
			currentRegion.ChildElementOffset.X = currentRegion.Rect.Left + currentRegion.ChildElementSpace.X;
			currentRegion.ChildElementOffset.Y = currentRegion.ChildContentRect.Bottom + currentRegion.ChildElementSpace.Y;
		}

		currentRegion.ChildElementLayout = layout;
	}


	NS_NODISCARD_INLINE nsPointFloat GetCurrentRegionChildElementSpace() const noexcept
	{
		return Controls[CurrentRegionId].ChildElementSpace;
	}


	NS_NODISCARD_INLINE bool IsControlFocused(const char* controlUniqueId) const noexcept
	{
		return CurrentControlInputFocus == controlUniqueId;
	}


	NS_NODISCARD_INLINE bool IsAnyControlFocused() const noexcept
	{
		return CurrentControlInputFocus != nullptr;
	}


	NS_NODISCARD_INLINE const nsPointFloat& GetWindowDimension() const noexcept
	{
		return WindowDimension;
	}


	NS_NODISCARD_INLINE const nsGUIRect& GetCanvasRect() const noexcept
	{
		return Controls[0].Rect;
	}


	NS_NODISCARD_INLINE const nsTArray<nsGUIDrawCallPerRegion>& GetDrawCallData() const noexcept
	{
		return DrawCallData;
	}


	NS_NODISCARD_INLINE const nsFontID GetDefaultFont() const noexcept
	{
		return DefaultFont;
	}


#ifdef _DEBUG
private:
	struct FrameDebug
	{
		nsVulkanBuffer* VertexBuffer;
		nsVulkanBuffer* IndexBuffer;
	};

	FrameDebug FrameDebugDatas[NS_ENGINE_FRAME_BUFFERING];
	nsMaterialID DefaultDebugMaterial;
	nsTArray<nsVertexPrimitive2D> DrawDebugVertices;
	nsTArray<uint32> DrawDebugIndices;

public:
	bool bDrawDebugRect;
	bool bDrawDebugHoveredRect;


public:
	NS_INLINE void AddDrawDebugLine(nsPointFloat start, nsPointFloat end, nsColor color) noexcept
	{
		const uint32 baseVertex = static_cast<uint32>(DrawDebugVertices.GetCount());

		DrawDebugVertices.Add(nsVertexPrimitive2D({ nsVector2(start.X, start.Y), color }));
		DrawDebugVertices.Add(nsVertexPrimitive2D({ nsVector2(end.X, end.Y), color }));

		DrawDebugIndices.Add(baseVertex);
		DrawDebugIndices.Add(baseVertex + 1);
	}


	NS_INLINE void AddDrawDebugRectLine(nsGUIRect rect, nsColor color) noexcept
	{
		if (rect.GetWidth() > 0.0f && rect.GetHeight() > 0.0f)
		{
			const uint32 baseVertex = static_cast<uint32>(DrawDebugVertices.GetCount());

			DrawDebugVertices.Add(nsVertexPrimitive2D({ nsVector2(rect.Left, rect.Top), color }));
			DrawDebugVertices.Add(nsVertexPrimitive2D({ nsVector2(rect.Right, rect.Top), color }));
			DrawDebugVertices.Add(nsVertexPrimitive2D({ nsVector2(rect.Right, rect.Bottom), color }));
			DrawDebugVertices.Add(nsVertexPrimitive2D({ nsVector2(rect.Left, rect.Bottom), color }));

			DrawDebugIndices.Add(baseVertex);
			DrawDebugIndices.Add(baseVertex + 1);
			DrawDebugIndices.Add(baseVertex + 1);
			DrawDebugIndices.Add(baseVertex + 2);
			DrawDebugIndices.Add(baseVertex + 2);
			DrawDebugIndices.Add(baseVertex + 3);
			DrawDebugIndices.Add(baseVertex + 3);
			DrawDebugIndices.Add(baseVertex);
		}
	}


	NS_NODISCARD_INLINE const nsTArray<nsVertexPrimitive2D>& GetDrawDebugVertices() const noexcept
	{
		return DrawDebugVertices;
	}


	NS_NODISCARD_INLINE const nsTArray<uint32>& GetDrawDebugIndices() const noexcept
	{
		return DrawDebugIndices;
	}

#endif // _DEBUG


	NS_DECLARE_NOCOPY(nsGUIContext)

};
