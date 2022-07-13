#include "nsRenderManager.h"
#include "nsCommandLines.h"
#include "nsShaderManager.h"
#include "nsMesh.h"
#include "nsTextureManager.h"
#include "nsMaterial.h"
#include "nsAnimationManager.h"
#include "nsFont.h"
#include "nsRenderer.h"



nsRenderManager::nsRenderManager() noexcept
	: FrameIndex(0)
	, bInitialized(false)
{

}


void nsRenderManager::Initialize() noexcept
{
	if (bInitialized)
	{
		return;
	}

	const nsCommandLines& commandLines = nsCommandLines::Get();
	
	nsVulkan::Initialize(commandLines.HasCommand("vulkandebug"));
	nsShaderManager::Get().Initialize();
	nsTextureManager::Get().Initialize();
	nsMaterialManager::Get().Initialize();
	nsMeshManager::Get().Initialize();
	nsFontManager::Initialize(TEXT("../../../Assets/Fonts/Roboto_Regular.ttf"), 15.0f);

	bInitialized = true;
}


void nsRenderManager::Render(float deltaTime) noexcept
{
	nsVulkan::BeginFrame(FrameIndex);

	nsShaderManager::Get().CompileShaders(false);

	nsTextureManager::Get().BeginFrame(FrameIndex);
	nsMaterialManager::Get().BeginFrame(FrameIndex);
	nsMeshManager::Get().BeginFrame(FrameIndex);
	nsAnimationManager::Get().BeginFrame(FrameIndex);

	for (int i = 0; i < RegisteredRenderers.GetCount(); ++i)
	{
		RegisteredRenderers[i]->BeginRender(FrameIndex, deltaTime);
	}

	nsMaterialManager::Get().UpdateRenderResources();
	nsTextureManager::Get().UpdateRenderResources();
	nsMeshManager::Get().UpdateRenderResources();
	nsAnimationManager::Get().UpdateRenderResources();

	for (int i = 0; i < RegisteredRenderers.GetCount(); ++i)
	{
		RegisteredRenderers[i]->ExecuteDrawCalls();
	}

	nsVulkan::Execute();

	FrameIndex = (FrameIndex + 1) % NS_ENGINE_FRAME_BUFFERING;
}
