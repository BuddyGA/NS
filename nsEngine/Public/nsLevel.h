#pragma once

#include "nsActor.h"



class NS_ENGINE_API nsLevel : public nsObject
{
	NS_DECLARE_OBJECT(nsLevel)

private:
	nsWorld* World;
	nsTArray<nsActor*> Actors;
	uint8 bPersistent : 1;
	uint8 bLoading : 1;
	uint8 bLoaded : 1;


public:
	nsLevel();
	void Destroy();
	
	// Add actor into level
	bool AddActor(nsActor* actor) noexcept;

	// Remove actor from level
	bool RemoveActor(nsActor* actor) noexcept;


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
