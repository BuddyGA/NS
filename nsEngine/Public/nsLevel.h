#pragma once

#include "nsActor.h"



class NS_ENGINE_API nsLevel
{
private:
	nsName Name;
	nsWorld* World;
	nsTArray<nsActor*> Actors;
	uint8 bPersistent : 1;
	uint8 bLoading : 1;
	uint8 bLoaded : 1;


public:
	nsLevel(nsName name);
	void Destroy();
	
	// Add actor into level
	void AddActor(nsActor* actor) noexcept;

	// Remove actor from level
	void RemoveActor(nsActor* actor) noexcept;


	NS_NODISCARD_INLINE const nsName& GetName() const noexcept
	{
		return Name;
	}


	NS_NODISCARD_INLINE nsWorld* GetWorld() const noexcept
	{
		return World;
	}


	// Get all actors from this level
	NS_NODISCARD_INLINE const nsTArray<nsActor*>& GetActors() const noexcept
	{
		return Actors;
	}


	NS_NODISCARD_INLINE bool IsPersistent() const noexcept
	{
		return bPersistent;
	}


	NS_NODISCARD_INLINE bool IsLoaded() const noexcept
	{
		return bLoaded;
	}


	friend class nsWorld;

};