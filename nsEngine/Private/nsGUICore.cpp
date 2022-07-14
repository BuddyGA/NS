#include "nsGUICore.h"
#include "nsLogger.h"
#include "nsAlgorithm.h"
#include "nsTextureManager.h"
#include "nsMaterial.h"
#include "API_VK/nsVulkanFunctions.h"



#ifdef NS_ENGINE_DEBUG_DRAW
#define NS_GUI_DRAW_DEBUG_COLOR_REGION_RECT_CLIP		nsColor::CYAN
#define NS_GUI_DRAW_DEBUG_COLOR_REGION_RECT_CONTENT		nsColor::YELLOW
#define NS_GUI_DRAW_DEBUG_COLOR_RECT					nsColor::GREEN
#define NS_GUI_DRAW_DEBUG_COLOR_RECT_HOVERED			nsColor::WHITE
#endif // NS_ENGINE_DEBUG_DRAW


nsLogCategory GUILog(TEXT("nsGUILog"), nsELogVerbosity::LV_DEBUG);



nsGUIContext::nsGUIContext() noexcept
	: FrameDatas()
	, FrameIndex(0)
	, CharInput()
{
	CachedRegionDatas.Reserve(32);
	Controls.Reserve(64);
	RegionStacks.Reserve(32);
	CurrentRegionId = -1;
	CurrentHoveredRegionId = -1;
	LastHoveredRegionId = -1;
	CurrentActiveWindowRegionId = -1;
	CurrentControlInputFocus = nullptr;

	DrawVertices.Reserve(1024);
	DrawIndices.Reserve(2048);
	DrawDatas.Reserve(32);

	for (int i = 0; i < NS_ENGINE_FRAME_BUFFERING; ++i)
	{
		Frame& frame = FrameDatas[i];
		frame.VertexBuffer = nsVulkan::CreateVertexBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, NS_MEMORY_SIZE_KiB(256), nsName::Format("gui_vertex_buffer_%i", i));
		frame.IndexBuffer = nsVulkan::CreateIndexBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, NS_MEMORY_SIZE_KiB(64), nsName::Format("gui_index_buffer_%i", i));
	}

	DefaultFont = nsFontManager::GetDefaultFont();
	DefaultTextureFont = nsFontManager::GetFontTexture(DefaultFont);
	DefaultMaterialMesh = nsMaterialManager::Get().GetDefaultMaterial_GUI();
	DefaultMaterialFont = nsMaterialManager::Get().GetDefaultMaterial_Font();
	DefaultTextureWhite = nsTextureManager::Get().GetDefaultTexture2D_White();

	nsPlatform::Memory_Zero(MouseState.bWasPresseds, sizeof(bool) * 3);
	nsPlatform::Memory_Zero(MouseState.bPresseds, sizeof(bool) * 3);

#ifdef NS_ENGINE_DEBUG_DRAW
	DefaultDebugMaterial = nsMaterialManager::Get().GetDefaultMaterial_PrimitiveLine_2D();

	for (int i = 0; i < NS_ENGINE_FRAME_BUFFERING; ++i)
	{
		FrameDebug& debug = FrameDebugDatas[i];
		debug.VertexBuffer = nsVulkan::CreateVertexBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, NS_MEMORY_SIZE_KiB(16), nsName::Format("gui_vertex_buffer_debug_%i", i));
		debug.IndexBuffer = nsVulkan::CreateIndexBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, NS_MEMORY_SIZE_KiB(8), nsName::Format("gui_index_buffer_debug_%i", i));
	}

	DrawDebugVertices.Reserve(64);
	DrawDebugIndices.Reserve(32);
	bDrawDebugRect = false;
	bDrawDebugHoveredRect = true;
#endif // NS_ENGINE_DEBUG_DRAW

}


nsGUIContext::~nsGUIContext() noexcept
{
	NS_Validate(0);
}


void nsGUIContext::MouseMove(const nsMouseMoveEventArgs& e) noexcept
{
	MouseState.Position.X = static_cast<float>(e.Position.X);
	MouseState.Position.Y = static_cast<float>(e.Position.Y);

	for (int i = 0; i < 3; ++i)
	{
		if (MouseState.bPresseds[i] && !MouseState.bDraggings[i])
		{
			const float deltaAbsX = nsMath::Abs(MouseState.Position.X - MouseState.DragStartPosition.X);
			const float deltaAbsY = nsMath::Abs(MouseState.Position.Y - MouseState.DragStartPosition.Y);
			MouseState.bDraggings[i] = deltaAbsX > MouseState.DragThreshold.X || deltaAbsY > MouseState.DragThreshold.Y;
		}
	}
}


void nsGUIContext::MouseButton(const nsMouseButtonEventArgs& e) noexcept
{
	if (e.ButtonState == nsEButtonState::PRESSED)
	{
		if (e.Key == nsEInputKey::MOUSE_LEFT) MouseState.bPresseds[0] = true;
		if (e.Key == nsEInputKey::MOUSE_MIDDLE) MouseState.bPresseds[1] = true;
		if (e.Key == nsEInputKey::MOUSE_RIGHT) MouseState.bPresseds[2] = true;

		if (e.Key == nsEInputKey::MOUSE_LEFT || e.Key == nsEInputKey::MOUSE_MIDDLE || e.Key == nsEInputKey::MOUSE_RIGHT)
		{
			MouseState.DragStartPosition = nsPointFloat(static_cast<float>(e.Position.X), static_cast<float>(e.Position.Y));
		}
	}
	else if (e.ButtonState == nsEButtonState::RELEASED)
	{
		if (e.Key == nsEInputKey::MOUSE_LEFT) MouseState.bPresseds[0] = false;
		if (e.Key == nsEInputKey::MOUSE_MIDDLE) MouseState.bPresseds[1] = false;
		if (e.Key == nsEInputKey::MOUSE_RIGHT) MouseState.bPresseds[2] = false;

		nsPlatform::Memory_Set(MouseState.bDraggings, 0, sizeof(bool) * 3);
	}
	else if (e.ButtonState == nsEButtonState::REPEAT)
	{
		if (e.Key == nsEInputKey::MOUSE_LEFT) MouseState.bDoubleClicks[0] = true;
		if (e.Key == nsEInputKey::MOUSE_MIDDLE) MouseState.bDoubleClicks[1] = true;
		if (e.Key == nsEInputKey::MOUSE_RIGHT) MouseState.bDoubleClicks[2] = true;
	}
}


void nsGUIContext::MouseWheel(const nsMouseWheelEventArgs& e) noexcept
{
	MouseState.ScrollValue.X = static_cast<float>(e.ScrollValue.X);
	MouseState.ScrollValue.Y = static_cast<float>(e.ScrollValue.Y);
}


void nsGUIContext::KeyboardButton(const nsKeyboardButtonEventArgs& e) noexcept
{
	if (e.ButtonState == nsEButtonState::PRESSED)
	{
		KeyboardState.bPresseds[e.Key] = true;
	}
	else if (e.ButtonState == nsEButtonState::RELEASED)
	{
		KeyboardState.bReleaseds[e.Key] = true;
	}
}


void nsGUIContext::AddCharInput(char c) noexcept
{
	if (c == '\b' || c == '\r' || c == '\n' || c == '\0')
	{
		return;
	}

	CharInput[0] = c;
}


nsMaterialID nsGUIContext::GetOrCreateMaterialInstance(nsMaterialID material, nsTextureID texture) noexcept
{
	nsTArrayInline<MaterialInstance, 8>& materialInstances = MaterialInstanceTable[material];

	for (int i = 0; i < materialInstances.GetCount(); ++i)
	{
		if (materialInstances[i].Texture == texture)
		{
			return materialInstances[i].Material;
		}
	}

	nsMaterialManager& materialManager = nsMaterialManager::Get();
	MaterialInstance& instance = materialInstances.Add();
	instance.Material = materialManager.CreateMaterialInstance(nsName::Format("%s_inst_%i", *materialManager.GetMaterialName(material), materialInstances.GetCount()), material);
	materialManager.SetMaterialParameterTextureValue(instance.Material, "texture", texture);
	instance.Texture = texture;

	return instance.Material;
}


nsGUIRectInteractions nsGUIContext::TestInteraction(const nsGUIRect& rect) const noexcept
{
	nsGUIRectInteractions interactions = nsEGUIRectInteraction::None;

	if (rect.IsPointInside(MouseState.Position))
	{
		interactions |= nsEGUIRectInteraction::Hovered;
		
		if (MouseState.bPresseds[0] && !MouseState.bWasPresseds[0])
		{
			interactions |= nsEGUIRectInteraction::Pressed;
		}
		else if (!MouseState.bPresseds[0] && MouseState.bWasPresseds[0])
		{
			interactions |= nsEGUIRectInteraction::Released;
		}
	}

	return interactions;
}


void nsGUIContext::AddDrawShape(const nsVertexGUI* vertices, int vertexCount, const uint32* indices, int indexCount, nsTextureID texture, nsMaterialID material, uint8 orderZ) noexcept
{
	if (vertices == nullptr || vertexCount <= 0 || indices == nullptr || indexCount <= 0)
	{
		return;
	}

	DrawData& draw = DrawDatas.Add();
	draw.RegionId = CurrentRegionId;
	
	draw.Material = GetOrCreateMaterialInstance(
		material == nsMaterialID::INVALID ? DefaultMaterialMesh : material,
		texture == nsTextureID::INVALID ? DefaultTextureWhite : texture
	);

	draw.BaseVertex = DrawVertices.GetCount();
	draw.VertexCount = vertexCount;
	draw.BaseIndex = DrawIndices.GetCount();
	draw.IndexCount = indexCount;
	draw.OrderZ = orderZ;
	draw.bIsText = 0;

	DrawVertices.InsertAt(vertices, vertexCount);
	DrawIndices.InsertAt(indices, indexCount);
	DrawBindMaterials.AddUnique(draw.Material);
}


void nsGUIContext::AddDrawTriangle(const nsPointFloat& center, float halfWidth, float rotationDegree, nsColor color, nsTextureID texture, nsMaterialID material, uint8 orderZ) noexcept
{
	const int VERTEX_COUNT = 3;
	nsVertexGUI vertices[VERTEX_COUNT] =
	{
		{ nsVector2(0.0f, -halfWidth), nsVector2(0.5f, 0.0f), color },
		{ nsVector2(-halfWidth, halfWidth), nsVector2(1.0f, 0.0f), color },
		{ nsVector2(halfWidth, halfWidth), nsVector2(1.0f, 1.0f), color },
	};

	const float theta = nsMath::DegToRad(rotationDegree);
	const float sinTheta = nsMath::Sin(theta);
	const float cosTheta = nsMath::Cos(theta);

	for (int i = 0; i < VERTEX_COUNT; ++i)
	{
		nsVertexGUI& vertex = vertices[i];

		const nsVector2 rotatedVertex(
			vertex.Position.X * cosTheta - vertex.Position.Y * sinTheta,
			vertex.Position.X * sinTheta + vertex.Position.Y * cosTheta
		);

		vertex.Position.X = center.X + rotatedVertex.X;
		vertex.Position.Y = center.Y + rotatedVertex.Y;
	}
	
	const int INDEX_COUNT = 3;
	const uint32 indices[INDEX_COUNT] = { 0, 2, 1 };

	AddDrawShape(vertices, VERTEX_COUNT, indices, INDEX_COUNT, texture, material, orderZ);
}


void nsGUIContext::AddDrawRect(const nsGUIRect& rect, nsColor color, nsTextureID texture, nsMaterialID material, uint8 orderZ, const nsVector2& uv0, const nsVector2& uv1) noexcept
{
	const int VERTEX_COUNT = 4;
	const nsVertexGUI vertices[VERTEX_COUNT] =
	{
		{ nsVector2(rect.Left, rect.Top), nsVector2(uv0.X, uv0.Y), color },
		{ nsVector2(rect.Right, rect.Top), nsVector2(uv1.X, uv0.Y), color },
		{ nsVector2(rect.Right, rect.Bottom), nsVector2(uv1.X, uv1.Y), color },
		{ nsVector2(rect.Left, rect.Bottom), nsVector2(uv0.X, uv1.Y), color }
	};

	const int INDEX_COUNT = 6;
	const uint32 indices[INDEX_COUNT] = { 0, 1, 2, 2, 3, 0 };

	AddDrawShape(vertices, VERTEX_COUNT, indices, INDEX_COUNT, texture, material, orderZ);
}


void nsGUIContext::AddDrawText(const wchar_t* text, int charLength, const nsPointFloat& position, nsColor color, nsFontID font, nsMaterialID material, uint8 orderZ) noexcept
{
	if (text == nullptr || charLength <= 0)
	{
		return;
	}

	const nsFontID useFont = (font == nsFontID::INVALID) ? DefaultFont : font;

	DrawData& draw = DrawDatas.Add();
	draw.RegionId = CurrentRegionId;

	draw.Material = GetOrCreateMaterialInstance(
		material == nsMaterialID::INVALID ? DefaultMaterialFont : material,
		useFont == DefaultFont ? DefaultTextureFont : nsFontManager::GetFontTexture(useFont)
	);

	draw.BaseVertex = DrawVertices.GetCount();
	draw.BaseIndex = DrawIndices.GetCount();

	nsPointFloat drawPosition = position;
	const int charCount = nsFontManager::GenerateVertices(useFont, drawPosition, text, charLength, color, DrawVertices, DrawIndices);

	draw.VertexCount = charCount * 4;
	draw.IndexCount = charCount * 6;
	draw.OrderZ = orderZ;
	draw.bIsText = 1;

	
	DrawBindMaterials.AddUnique(draw.Material);
}


nsGUIRect nsGUIContext::AddDrawTextOnRect(const wchar_t* text, int charLength, const nsGUIRect& rect, nsEGUIAlignmentHorizontal hAlign, nsEGUIAlignmentVertical vAlign, const nsPointFloat& offsetAlignment, nsColor color, nsFontID font, nsMaterialID material) noexcept
{
	if (text == nullptr || charLength <= 0)
	{
		return nsGUIRect();
	}

	const nsFontID useFont = (font == nsFontID::INVALID) ? DefaultFont : font;
	nsGUIRect textRect = nsFontManager::CalculateRect(useFont, nsPointFloat(), text, charLength);
	const float width = textRect.GetWidth();
	const float height = textRect.GetHeight();

	switch (hAlign)
	{
		case nsEGUIAlignmentHorizontal::LEFT: textRect.Left = rect.Left; break;
		case nsEGUIAlignmentHorizontal::CENTER: textRect.Left = rect.Left + (rect.GetWidth() * 0.5f) - (textRect.GetWidth() * 0.5f); break;
		case nsEGUIAlignmentHorizontal::RIGHT: textRect.Left = rect.Right - textRect.GetWidth(); break;
		default: break;
	}

	switch (vAlign)
	{
		case nsEGUIAlignmentVertical::TOP: textRect.Top = rect.Top; break;
		case nsEGUIAlignmentVertical::CENTER: textRect.Top = rect.Top + (rect.GetHeight() * 0.5f) - (textRect.GetHeight() * 0.5f); break;
		case nsEGUIAlignmentVertical::BOTTOM: textRect.Top = rect.Bottom - textRect.GetHeight(); break;
		default: break;
	}

	textRect.Left += offsetAlignment.X;
	textRect.Top += offsetAlignment.Y;
	textRect.Right = textRect.Left + width;
	textRect.Bottom = textRect.Top + height;

	AddDrawText(text, charLength, nsPointFloat(textRect.Left, textRect.Top), color, font, material);
	
	return textRect;
}


void nsGUIContext::BeginRegion(const char* uniqueId, const nsGUIRect& rect, const nsPointFloat& childElementSpace, nsEGUIElementLayout childElementLayout, nsGUIScrollOptions scrollOptions, bool bIsWindow, nsName debugName) noexcept
{
	if (uniqueId == nullptr && scrollOptions != nsEGUIScrollOption::None)
	{
		NS_LogWarning(GUILog, TEXT("Cannot cache scroll value data. Region <id> must not nullptr if <scrollOptions> is not None! [DebugName: %s]"), *debugName.ToString());
	}

	const int parentRegionId = CurrentRegionId;
	nsGUIControl& newRegion = Controls.Add();
	newRegion.Id = uniqueId;
	newRegion.Rect = rect;
	newRegion.bIsWindow = bIsWindow;

	// Root region (canvas)
	if (parentRegionId == -1)
	{
		newRegion.bIsVisible = true;
		newRegion.Interactions = TestInteraction(newRegion.Rect);
	}
	// Child region
	else
	{
		UpdateControlInCurrentRegion(newRegion, true);
	}

	newRegion.ScrollOptions = scrollOptions;
	
	//NS_Assert(newRegion.Rect.Left > 0.0f && newRegion.Rect.Top > 0.0f);
	newRegion.ChildClipRect = newRegion.Rect;

	if (newRegion.ChildClipRect.Left < 0.0f) newRegion.ChildClipRect.Left = 0.0f;
	if (newRegion.ChildClipRect.Top < 0.0f) newRegion.ChildClipRect.Top = 0.0f;

	newRegion.ChildElementLayout = childElementLayout;
	newRegion.ChildElementSpace = childElementSpace;
	newRegion.ChildElementOffset = nsPointFloat(newRegion.Rect.Left + newRegion.ChildElementSpace.X, newRegion.Rect.Top + newRegion.ChildElementSpace.Y);
	newRegion.ChildContentRect = nsGUIRect(newRegion.Rect.Left + newRegion.ChildElementSpace.X, newRegion.Rect.Top + newRegion.ChildElementSpace.Y, 0.0f, 0.0f);

	CurrentRegionId = Controls.GetCount() - 1;
	RegionStacks.Add(CurrentRegionId);

	if (newRegion.Interactions & nsEGUIRectInteraction::Hovered)
	{
		CurrentHoveredRegionId = CurrentRegionId;
	}
	else if (CurrentRegionId == LastHoveredRegionId)
	{
		CurrentHoveredRegionId = -1;
	}

	const bool bScrollableX = newRegion.ScrollOptions & nsEGUIScrollOption::Scrollable_X;
	const bool bScrollableY = newRegion.ScrollOptions & nsEGUIScrollOption::Scrollable_Y;

	if (bScrollableX || bScrollableY)
	{
		NS_AssertV(newRegion.Id, TEXT("Must provide region id!"));
		RegionData& data = CachedRegionDatas[newRegion.Id];

		// Scroll-X
		if (data.ChildContentRect.Right > newRegion.Rect.Right)
		{
			if (bScrollableX && LastHoveredRegionId == CurrentRegionId)
			{
				data.ScrollValue.X -= MouseState.ScrollValue.X * MouseState.ScrollSpeed.X;
			}

			data.ScrollValue.X = nsMath::Clamp(data.ScrollValue.X, 0.0f, data.ChildContentRect.Right - newRegion.Rect.Right);
		}

		// Scroll-Y
		if (data.ChildContentRect.Bottom > newRegion.Rect.Bottom)
		{
			if (bScrollableY && LastHoveredRegionId == CurrentRegionId)
			{
				data.ScrollValue.Y -= MouseState.ScrollValue.Y * MouseState.ScrollSpeed.Y;
			}

			data.ScrollValue.Y = nsMath::Clamp(data.ScrollValue.Y, 0.0f, data.ChildContentRect.Bottom - newRegion.Rect.Bottom);
		}
	}

	if (newRegion.Interactions & nsEGUIRectInteraction::Pressed)
	{
		if (newRegion.bIsWindow)
		{
			CurrentActiveWindowRegionId = CurrentRegionId;
		}

		if (LastHoveredRegionId == CurrentRegionId)
		{
			CurrentControlInputFocus = nullptr;
		}
	}

	ElementLayoutStacks.Add(newRegion.ChildElementLayout);

#ifdef NS_ENGINE_DEBUG_DRAW
	newRegion.DebugName = debugName;

	if (bDrawDebugRect && LastHoveredRegionId == CurrentRegionId)
	{
		AddDrawDebugRectLine(newRegion.Rect, NS_GUI_DRAW_DEBUG_COLOR_REGION_RECT_CLIP);
	}
#endif // NS_ENGINE_DEBUG_DRAW

}


void nsGUIContext::EndRegion() noexcept
{
	nsGUIControl& currentRegion = Controls[CurrentRegionId];

	if (currentRegion.Id)
	{
		RegionData& data = CachedRegionDatas[currentRegion.Id];
		data.ChildContentRect = currentRegion.ChildContentRect;

		if (data.ChildContentRect.Right > currentRegion.Rect.Right && (currentRegion.ScrollOptions & nsEGUIScrollOption::AutoScroll_X) )
		{
			data.ScrollValue.X = 999999.0f;
		}

		if (data.ChildContentRect.Bottom > currentRegion.Rect.Bottom && (currentRegion.ScrollOptions & nsEGUIScrollOption::AutoScroll_Y) )
		{
			data.ScrollValue.Y = 999999.0f;
		}
	}

#ifdef NS_ENGINE_DEBUG_DRAW
	if (bDrawDebugRect && CurrentRegionId == LastHoveredRegionId)
	{
		AddDrawDebugRectLine(currentRegion.ChildContentRect, NS_GUI_DRAW_DEBUG_COLOR_REGION_RECT_CONTENT);
	}
#endif // NS_ENGINE_DEBUG_DRAW

	ElementLayoutStacks.RemoveAt(NS_ARRAY_INDEX_LAST);
	RegionStacks.RemoveAt(NS_ARRAY_INDEX_LAST);
	CurrentRegionId = RegionStacks.IsEmpty() ? -1 : RegionStacks[RegionStacks.GetCount() - 1];
}


nsGUIControl nsGUIContext::TestControlInCurrentRegion(const nsPointFloat& size) const noexcept
{
	const nsGUIControl& currentRegion = Controls[CurrentRegionId];

	nsGUIControl testControl;
	testControl.Rect = nsGUIRect(0.0f, 0.0f, size.X, size.Y);
	testControl.Rect.Left += currentRegion.ChildElementOffset.X;
	testControl.Rect.Top += currentRegion.ChildElementOffset.Y;

	float width = size.X;
	float height = size.Y;

	if (currentRegion.ScrollOptions & nsEGUIScrollOption::Scrollable_X)
	{
		NS_AssertV(size.X > 0.0f, TEXT("size.X must be greater than 0.0f if scroll-x is enabled!"));
	}
	else if (size.X <= 0.0f)
	{
		width = (currentRegion.Rect.Right - currentRegion.ChildElementSpace.X) - testControl.Rect.Left;
	}

	if (currentRegion.ScrollOptions & nsEGUIScrollOption::Scrollable_Y)
	{
		NS_AssertV(size.Y > 0.0f, TEXT("size.Y must be greater than 0.0f if scroll-Y is enabled!"));
	}
	else if (size.Y <= 0.0f)
	{
		height = (currentRegion.Rect.Bottom - currentRegion.ChildElementSpace.Y) - testControl.Rect.Top;
	}

	testControl.Rect.Right = testControl.Rect.Left + width;
	testControl.Rect.Bottom = testControl.Rect.Top + height;

	if (currentRegion.Id)
	{
		const RegionData& data = CachedRegionDatas[currentRegion.Id];
		testControl.Rect.Left -= data.ScrollValue.X;
		testControl.Rect.Top -= data.ScrollValue.Y;
		testControl.Rect.Right = testControl.Rect.Left + width;
		testControl.Rect.Bottom = testControl.Rect.Top + height;
	}

	NS_Assert(testControl.Rect.Left <= testControl.Rect.Right);
	NS_Assert(testControl.Rect.Top <= testControl.Rect.Bottom);

	testControl.bIsVisible = currentRegion.IsChildControlVisible(testControl);

	// Check if clipped by parent region
	if (testControl.bIsVisible && RegionStacks.GetCount() > 1)
	{
		const int parentRegionId = RegionStacks[RegionStacks.GetCount() - 2];
		const nsGUIControl& parentRegion = Controls[parentRegionId];
		testControl.bIsVisible = parentRegion.IsChildControlVisible(testControl);
	}

	testControl.Interactions = testControl.bIsVisible && (currentRegion.Interactions & nsEGUIRectInteraction::Hovered) ? TestInteraction(testControl.Rect) : nsEGUIRectInteraction::None;

	return testControl;
}


void nsGUIContext::UpdateControlInCurrentRegion(nsGUIControl& control, bool bIsRegion) noexcept
{
	float width = control.Rect.GetWidth();
	float height = control.Rect.GetHeight();

	nsGUIControl& currentRegion = Controls[CurrentRegionId];
	control.Rect.Left += currentRegion.ChildElementOffset.X;
	control.Rect.Top += currentRegion.ChildElementOffset.Y;

	if (currentRegion.ScrollOptions & nsEGUIScrollOption::Scrollable_X)
	{
		NS_AssertV(width > 0.0f, TEXT("width must be greater than 0.0f if scroll-X is enabled!"));
	}
	else if (width <= 0.0f)
	{
		width = (currentRegion.Rect.Right - currentRegion.ChildElementSpace.X) - control.Rect.Left;
	}

	if (currentRegion.ScrollOptions & nsEGUIScrollOption::Scrollable_Y)
	{
		NS_AssertV(height > 0.0f, TEXT("height must be greater than 0.0f if scroll-Y is enabled!"));
	}
	else if (height <= 0.0f)
	{
		height = (currentRegion.Rect.Bottom - currentRegion.ChildElementSpace.Y) - control.Rect.Top;
	}

	control.Rect.Right = control.Rect.Left + width;
	control.Rect.Bottom = control.Rect.Top + height;

	currentRegion.ChildContentRect.Right = nsMath::Max(currentRegion.ChildContentRect.Right, control.Rect.Right);
	currentRegion.ChildContentRect.Bottom = nsMath::Max(currentRegion.ChildContentRect.Bottom, control.Rect.Bottom);

	if (currentRegion.Id)
	{
		const RegionData& data = CachedRegionDatas[currentRegion.Id];
		control.Rect.Left -= data.ScrollValue.X;
		control.Rect.Top -= data.ScrollValue.Y;
		control.Rect.Right = control.Rect.Left + width;
		control.Rect.Bottom = control.Rect.Top + height;
	}

	if (bIsRegion)
	{
		control.Rect.Right = nsMath::Clamp(control.Rect.Right, control.Rect.Left, currentRegion.Rect.Right);

		if (control.Rect.Right < control.Rect.Left)
		{
			control.Rect.Right = control.Rect.Left;
		}

		control.Rect.Bottom = nsMath::Clamp(control.Rect.Bottom, control.Rect.Top, currentRegion.Rect.Bottom);

		if (control.Rect.Bottom < control.Rect.Top)
		{
			control.Rect.Bottom = control.Rect.Top;
		}
	}

	NS_Assert(control.Rect.Left <= control.Rect.Right);
	NS_Assert(control.Rect.Top <= control.Rect.Bottom);

	control.bIsVisible = currentRegion.IsChildControlVisible(control);

	// Check if clipped by parent region
	if (control.bIsVisible && RegionStacks.GetCount() > 1)
	{
		const int parentRegionId = RegionStacks[RegionStacks.GetCount() - 2];
		const nsGUIControl& parentRegion = Controls[parentRegionId];
		control.bIsVisible = parentRegion.IsChildControlVisible(control);
	}

	control.Interactions = control.bIsVisible && (currentRegion.Interactions & nsEGUIRectInteraction::Hovered) ? TestInteraction(control.Rect) : nsEGUIRectInteraction::None;

	if (currentRegion.ChildElementLayout == nsEGUIElementLayout::HORIZONTAL)
	{
		currentRegion.ChildElementOffset.X += control.Rect.GetWidth() + currentRegion.ChildElementSpace.X;
	}
	else if (currentRegion.ChildElementLayout == nsEGUIElementLayout::VERTICAL)
	{
		currentRegion.ChildElementOffset.Y += control.Rect.GetHeight() + currentRegion.ChildElementSpace.Y;
	}
}


nsGUIControl nsGUIContext::AddControlText(const wchar_t* text, nsColor color, nsFontID font, nsMaterialID material) noexcept
{
	nsGUIControl textControl;
	const int charLength = nsPlatform::String_Length(text);

	if (charLength == 0)
	{
		return textControl;
	}

	const nsFontID useFont = (font == nsFontID::INVALID) ? DefaultFont : font;

	textControl.Rect = nsFontManager::CalculateRect(useFont, nsPointFloat(), text, charLength);

	UpdateControlInCurrentRegion(textControl, false);

	if (textControl.bIsVisible)
	{
		AddDrawText(text, charLength, nsPointFloat(textControl.Rect.Left, textControl.Rect.Top), color, font, material);
	}

#ifdef NS_ENGINE_DEBUG_DRAW
	if (bDrawDebugRect)
	{
		AddDrawDebugRectLine(textControl.Rect, NS_GUI_DRAW_DEBUG_COLOR_RECT);
	}
#endif // NS_ENGINE_DEBUG_DRAW

	return textControl;
}


nsGUIControl nsGUIContext::AddControlTextOnRect(const wchar_t* text, const nsGUIRect& rect, nsEGUIAlignmentHorizontal hAlign, nsEGUIAlignmentVertical vAlign, const nsPointFloat& offsetAlignment, const nsColor& color, nsFontID font, nsMaterialID material) noexcept
{
	const int charLength = nsPlatform::String_Length(text);

	if (charLength == 0)
	{
		return nsGUIControl();
	}

	const nsFontID useFont = (font == nsFontID::INVALID) ? DefaultFont : font;

	nsGUIControl textControl;
	textControl.Rect = rect;

	UpdateControlInCurrentRegion(textControl, false);

	if (textControl.bIsVisible)
	{
		AddDrawTextOnRect(text, charLength, textControl.Rect, hAlign, vAlign, offsetAlignment, color, font, material);
	}

#ifdef NS_ENGINE_DEBUG_DRAW
	if (bDrawDebugHoveredRect && IsCurrentRegionHovered() && (textControl.Interactions & nsEGUIRectInteraction::Hovered) )
	{
		AddDrawDebugRectLine(textControl.Rect, NS_GUI_DRAW_DEBUG_COLOR_RECT_HOVERED);
	}
#endif // NS_ENGINE_DEBUG_DRAW

	return textControl;
}


nsGUIControl nsGUIContext::AddControlRect(float width, float height, nsColor color, nsTextureID texture, nsMaterialID material) noexcept
{
	nsGUIControl newControl;
	newControl.Rect = nsGUIRect(0.0f, 0.0f, width, height);

	UpdateControlInCurrentRegion(newControl, false);

	if (newControl.bIsVisible)
	{
		AddDrawRect(newControl.Rect, color, texture, material);
	}

#ifdef NS_ENGINE_DEBUG_DRAW
	if (bDrawDebugRect)
	{
		AddDrawDebugRectLine(newControl.Rect, NS_GUI_DRAW_DEBUG_COLOR_RECT);
	}

	if (bDrawDebugHoveredRect && IsCurrentRegionHovered() && (newControl.Interactions & nsEGUIRectInteraction::Hovered)	)
	{
		AddDrawDebugRectLine(newControl.Rect, NS_GUI_DRAW_DEBUG_COLOR_RECT_HOVERED);
	}
#endif // NS_ENGINE_DEBUG_DRAW

	return newControl;
}


nsGUIControl& nsGUIContext::AddControlUnique(const char* uniqueId, const nsPointFloat& size) noexcept
{
	NS_Assert(uniqueId);
	CurrentControlId = Controls.GetCount();

	nsGUIControl& newControl = Controls.Add();
	newControl.Id = uniqueId;
	newControl.Rect = nsGUIRect(0.0f, 0.0f, size.X, size.Y);

	UpdateControlInCurrentRegion(newControl, false);

#ifdef NS_ENGINE_DEBUG_DRAW
	if (bDrawDebugRect)
	{
		AddDrawDebugRectLine(newControl.Rect, NS_GUI_DRAW_DEBUG_COLOR_RECT);
	}

	if (bDrawDebugHoveredRect && IsCurrentRegionHovered() && (newControl.Interactions & nsEGUIRectInteraction::Hovered))
	{
		AddDrawDebugRectLine(newControl.Rect, NS_GUI_DRAW_DEBUG_COLOR_RECT_HOVERED);
	}
#endif // NS_ENGINE_DEBUG_DRAW

	return newControl;
}


void nsGUIContext::SetControlInputFocus(const char* controlUniqueId) noexcept
{
#ifdef _DEBUG
	if (controlUniqueId && controlUniqueId != CurrentControlInputFocus)
	{
		const int index = Controls.Find(controlUniqueId, [](const nsGUIControl& check, const char* value) { return check.Id == value; });
		NS_Assert(index != NS_ARRAY_INDEX_INVALID);
	}
#endif // _DEBUG

	CurrentControlInputFocus = controlUniqueId;
}


void nsGUIContext::BeginRender(const nsPointFloat& windowDimension, const nsGUIRect& canvasRect) noexcept
{
	WindowDimension = windowDimension;

	Controls.Clear();
	RegionStacks.Clear();
	CurrentRegionId = -1;
	CurrentControlId = -1;
	DrawVertices.Clear();
	DrawIndices.Clear();
	DrawDatas.Clear();
	DrawBindMaterials.Clear();
	DrawCallData.Clear();

#ifdef NS_ENGINE_DEBUG_DRAW
	DrawDebugVertices.Clear();
	DrawDebugIndices.Clear();
#endif // NS_ENGINE_DEBUG_DRAW
	
	NS_Assert(WindowDimension.X > 0.0f && WindowDimension.Y > 0.0f);
	BeginRegion("canvas", canvasRect, nsPointFloat(), nsEGUIElementLayout::NONE, nsEGUIScrollOption::None, true);
}


void nsGUIContext::EndRender() noexcept
{
	EndRegion();

	NS_Assert(ElementLayoutStacks.IsEmpty());
	NS_Assert(RegionStacks.IsEmpty());
	NS_Assert(CurrentRegionId == -1);

	MouseState.PrevPosition = MouseState.Position;
	MouseState.ScrollValue = nsPointFloat();
	nsPlatform::Memory_Copy(MouseState.bWasPresseds, MouseState.bPresseds, sizeof(bool) * 3);
	nsPlatform::Memory_Zero(MouseState.bDoubleClicks, sizeof(bool) * 3);
	nsPlatform::Memory_Zero(KeyboardState.bPresseds, sizeof(bool) * nsEInputKey::MAX_COUNT);
	nsPlatform::Memory_Zero(KeyboardState.bReleaseds, sizeof(bool) * nsEInputKey::MAX_COUNT);

	LastHoveredRegionId = CurrentHoveredRegionId;
	nsPlatform::Memory_Zero(CharInput, sizeof(CharInput));
}


void nsGUIContext::UpdateResourcesAndBuildDrawCalls(int frameIndex) noexcept
{
	FrameIndex = frameIndex;

	const uint64 vertexBufferSize = sizeof(nsVertexGUI) * DrawVertices.GetCount();
	const uint64 indexBufferSize = sizeof(uint32) * DrawIndices.GetCount();

	if (vertexBufferSize > 0)
	{
		NS_Assert(indexBufferSize > 0);

		Frame& frame = FrameDatas[FrameIndex];

		// Sort draws based on regionId, material, orderZ, bIsText
		nsAlgorithm::Sort(DrawDatas.GetData(), DrawDatas.GetCount(),
			[](const DrawData& a, const DrawData& b)
			{
				const bool bSameRegion = a.RegionId == b.RegionId;
				const bool bIsText = a.bIsText == b.bIsText;
				const bool bSameMaterial = a.Material == b.Material;
				const bool bSameOrder = a.OrderZ == b.OrderZ;

				return (a.RegionId < b.RegionId) ||
					(bSameRegion && a.bIsText < b.bIsText) ||
					(bSameRegion && bIsText && a.Material < b.Material) ||
					(bSameRegion && bIsText && bSameMaterial && a.OrderZ < b.OrderZ);
			}
		);

		frame.VertexBuffer->Resize(vertexBufferSize);
		frame.IndexBuffer->Resize(indexBufferSize);
		nsVertexGUI* vtxMap = reinterpret_cast<nsVertexGUI*>(frame.VertexBuffer->MapMemory());
		uint32* idxMap = reinterpret_cast<uint32*>(frame.IndexBuffer->MapMemory());

		int vtxCount = 0;
		int idxCount = 0;

		for (int i = 0; i < DrawDatas.GetCount(); ++i)
		{
			const DrawData& data = DrawDatas[i];
			DrawCallData.Resize(data.RegionId + 1);

			const nsGUIControl& region = Controls[data.RegionId];

			nsGUIDrawCallPerRegion& perRegion = DrawCallData[data.RegionId];
			perRegion.ClipRect.X = static_cast<int>(region.ChildClipRect.Left);
			perRegion.ClipRect.Y = static_cast<int>(region.ChildClipRect.Top);
			perRegion.ClipRect.Width = static_cast<int>(region.ChildClipRect.GetWidth());
			perRegion.ClipRect.Height = static_cast<int>(region.ChildClipRect.GetHeight());

			int perMaterialIndex = -1;
			perRegion.Materials.AddUnique(data.Material, &perMaterialIndex);
			nsGUIDrawCallPerMaterial& perMaterialDraw = perRegion.Materials[perMaterialIndex];

			int perOrderIndex = -1;
			perMaterialDraw.Batches.AddUnique(data.OrderZ, &perOrderIndex);
			nsGUIDrawCallPerBatch& draw = perMaterialDraw.Batches[perOrderIndex];

			if (draw.IndexCount == 0)
			{
				draw.BaseIndex = idxCount;
				draw.IndexVertexOffset = vtxCount;
			}

			for (int j = data.BaseIndex; j < (data.BaseIndex + data.IndexCount); ++j)
			{
				DrawIndices[j] += draw.VertexCount;
			}

			draw.VertexCount += data.VertexCount;
			draw.IndexCount += data.IndexCount;

			nsPlatform::Memory_Copy(vtxMap + vtxCount, &DrawVertices[data.BaseVertex], sizeof(nsVertexGUI) * data.VertexCount);
			vtxCount += data.VertexCount;

			nsPlatform::Memory_Copy(idxMap + idxCount, &DrawIndices[data.BaseIndex], sizeof(uint32) * data.IndexCount);
			idxCount += data.IndexCount;
		}

		frame.VertexBuffer->UnmapMemory();
		frame.IndexBuffer->UnmapMemory();
	}

	nsMaterialManager::Get().BindMaterials(DrawBindMaterials.GetData(), DrawBindMaterials.GetCount());


#ifdef NS_ENGINE_DEBUG_DRAW
	if (!DrawDebugVertices.IsEmpty())
	{
		NS_Assert(!DrawDebugIndices.IsEmpty());

		FrameDebug& frameDebug = FrameDebugDatas[FrameIndex];

		const uint64 drawDebugVertexBufferSize = sizeof(nsVertexPrimitive2D) * DrawDebugVertices.GetCount();
		frameDebug.VertexBuffer->Resize(drawDebugVertexBufferSize);
		void* vtxMap = frameDebug.VertexBuffer->MapMemory();
		nsPlatform::Memory_Copy(vtxMap, DrawDebugVertices.GetData(), drawDebugVertexBufferSize);
		frameDebug.VertexBuffer->UnmapMemory();

		const uint64 drawDebugIndexBufferSize = sizeof(uint32) * DrawDebugIndices.GetCount();
		frameDebug.IndexBuffer->Resize(drawDebugIndexBufferSize);
		void* idxMap = frameDebug.IndexBuffer->MapMemory();
		nsPlatform::Memory_Copy(idxMap, DrawDebugIndices.GetData(), drawDebugIndexBufferSize);
		frameDebug.IndexBuffer->UnmapMemory();
	}
#endif // NS_ENGINE_DEBUG_DRAW
}


void nsGUIContext::ExecuteDrawCalls(VkCommandBuffer commandBuffer) noexcept
{
	nsMaterialManager& materialManager = nsMaterialManager::Get();

	struct VertexPushConstant
	{
		nsVector2 Scale;
		nsVector2 Translate;
	} vpc;

	vpc.Scale = nsVector2(2.0f / WindowDimension.X, 2.0f / WindowDimension.Y);
	vpc.Translate = nsVector2(-1.0f);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = WindowDimension.X;
	viewport.height = WindowDimension.Y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	const nsVulkanShaderResourceLayout* defaultShaderResourceLayout = materialManager.GetDefaultShaderResourceLayout_GUI();

	if (!DrawCallData.IsEmpty())
	{
		const VkDescriptorSet textureDescriptorSet = nsTextureManager::Get().GetDescriptorSet();
		Frame& frame = FrameDatas[FrameIndex];

		VkBuffer vkVertexBuffer = frame.VertexBuffer->GetVkBuffer();
		VkDeviceSize vertexOffset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vkVertexBuffer, &vertexOffset);

		vkCmdBindIndexBuffer(commandBuffer, frame.IndexBuffer->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultShaderResourceLayout->GetVkPipelineLayout(), 0, 1, &textureDescriptorSet, 0, nullptr);
		vkCmdPushConstants(commandBuffer, defaultShaderResourceLayout->GetVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VertexPushConstant), &vpc);

		nsVulkanShaderPipeline* boundShaderPipeline = nullptr;
		nsMaterialID boundMaterial = nsMaterialID::INVALID;

		for (int i = 0; i < DrawCallData.GetCount(); ++i)
		{
			const nsGUIDrawCallPerRegion& perRegion = DrawCallData[i];

			VkRect2D scissor{};
			scissor.offset = { perRegion.ClipRect.X, perRegion.ClipRect.Y };
			scissor.extent = { static_cast<uint32>(perRegion.ClipRect.Width), static_cast<uint32>(perRegion.ClipRect.Height) };
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			for (int j = 0; j < perRegion.Materials.GetCount(); ++j)
			{
				const nsGUIDrawCallPerMaterial& perMaterial = perRegion.Materials[j];
				const nsMaterialID material = perMaterial.Material;

				if (boundMaterial != material)
				{
					boundMaterial = material;
					const nsMaterialResource& materialResource = materialManager.GetMaterialResource(boundMaterial);

					if (boundShaderPipeline != materialResource.ShaderPipeline)
					{
						boundShaderPipeline = materialResource.ShaderPipeline;
						vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, boundShaderPipeline->GetVkPipeline());
					}

					nsTextureID texture = materialManager.GetMaterialParameterTextureValue(boundMaterial, "texture");
					vkCmdPushConstants(commandBuffer, defaultShaderResourceLayout->GetVkPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(VertexPushConstant), sizeof(int), &texture);
				}

				for (int k = 0; k < perMaterial.Batches.GetCount(); ++k)
				{
					const nsGUIDrawCallPerBatch& perBatch = perMaterial.Batches[k];
					vkCmdDrawIndexed(commandBuffer, static_cast<uint32>(perBatch.IndexCount), 1, static_cast<uint32>(perBatch.BaseIndex), perBatch.IndexVertexOffset, 0);
				}
			}
		}
	}

#ifdef NS_ENGINE_DEBUG_DRAW
	if (!DrawDebugVertices.IsEmpty())
	{
		NS_Assert(!DrawDebugIndices.IsEmpty());

		FrameDebug& frameDebug = FrameDebugDatas[FrameIndex];

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { static_cast<uint32>(WindowDimension.X), static_cast<uint32>(WindowDimension.Y) };
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VkBuffer vkVertexBuffer = frameDebug.VertexBuffer->GetVkBuffer();
		VkDeviceSize vertexOffset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vkVertexBuffer, &vertexOffset);

		vkCmdBindIndexBuffer(commandBuffer, frameDebug.IndexBuffer->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdPushConstants(commandBuffer, defaultShaderResourceLayout->GetVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VertexPushConstant), &vpc);

		const nsMaterialResource& materialResource = materialManager.GetMaterialResource(DefaultDebugMaterial);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, materialResource.ShaderPipeline->GetVkPipeline());

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32>(DrawDebugIndices.GetCount()), 1, 0, 0, 0);
	}
#endif // NS_ENGINE_DEBUG_DRAW

}
