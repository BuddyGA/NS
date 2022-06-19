#include "nsLevel.h"
#include "nsConsole.h"


static nsLogCategory LevelLog("nsLevelLog", nsELogVerbosity::LV_DEBUG);



nsLevel::nsLevel(nsName name)
{
	Name = name;
	World = nullptr;
	Actors.Reserve(64);
	bPersistent = 0;
	bLoading = 0;
	bLoaded = 0;
}


void nsLevel::Destroy()
{
	NS_ValidateV(0, "Not implemented yet!");
}


bool nsLevel::AddActor(nsActor* actor) noexcept
{
	if (actor == nullptr)
	{
		return false;
	}

	bool bAdded = Actors.AddUnique(actor);

	if (bAdded)
	{
		actor->OnAddedToLevel();
	}

	return bAdded;
}


bool nsLevel::RemoveActor(nsActor* actor) noexcept
{
	if (actor == nullptr)
	{
		return false;
	}

	NS_CONSOLE_Debug(LevelLog, "Remove actor [%s] from level [%s]", *actor->Name, *Name);

	bool bRemoved = Actors.Remove(actor);

	if (bRemoved)
	{
		actor->OnRemovedFromLevel();
	}

	return bRemoved;
}
