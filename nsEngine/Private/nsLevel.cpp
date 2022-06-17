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


void nsLevel::AddActor(nsActor* actor) noexcept
{
	if (actor == nullptr)
	{
		return;
	}

	Actors.AddUnique(actor);
	actor->OnAddedToLevel();
}


void nsLevel::RemoveActor(nsActor* actor) noexcept
{
	if (actor == nullptr)
	{
		return;
	}

	NS_CONSOLE_Debug(LevelLog, "Remove actor [%s] from level [%s]", *actor->Name, *Name);

	Actors.Remove(actor);
	actor->OnRemovedFromLevel();
}
