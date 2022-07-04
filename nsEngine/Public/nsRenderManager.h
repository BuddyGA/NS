#pragma once

#include "nsRenderer.h"



class NS_ENGINE_API nsRenderManager
{
	NS_DECLARE_SINGLETON(nsRenderManager)

private:
	int FrameIndex;
	bool bInitialized;

	nsTArrayInline<nsRenderer*, 8> RegisteredRenderers;
	nsTMap<nsWorld*, nsRenderContextWorld> WorldRenderContexts;


public:
	void Initialize() noexcept;
	void Render(float deltaTime) noexcept;


	NS_INLINE void RegisterRenderer(nsRenderer* renderer) noexcept
	{
		RegisteredRenderers.AddUnique(renderer);
	}


	NS_INLINE void UnregisterRenderer(nsRenderer* renderer) noexcept
	{
		RegisteredRenderers.Remove(renderer, false);
	}


	NS_INLINE void AddWorldRenderContext(nsWorld* world) noexcept
	{
		if (world == nullptr)
		{
			return;
		}

		WorldRenderContexts.Add(world);
	}


	NS_INLINE void RemoveWorldRenderContext(nsWorld* world) noexcept
	{
		if (world == nullptr)
		{
			return;
		}

		WorldRenderContexts.Remove(world);
	}


	NS_NODISCARD_INLINE nsRenderContextWorld& GetWorldRenderContext(nsWorld* world) noexcept
	{
		NS_Validate(world);

		return WorldRenderContexts[world];
	}

};
