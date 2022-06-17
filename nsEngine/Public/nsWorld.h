#pragma once

#include "nsLevel.h"



class NS_ENGINE_API nsWorld
{
private:
	nsName Name;
	float StartTimeSeconds;
	float DeltaTimeSeconds;
	bool bHasStartedPlay;

	nsTArray<nsLevel*> Levels;

	nsMemory ActorMemory;
	nsTArray<nsActor*> ActorList;
	nsTArray<nsActor*> StartStopPlayActors;
	nsTArray<nsActor*> TickUpdateActors;
	nsTArray<nsActor*> PendingDestroyActors;

public:
	bool bOnlyTickAfterStartedPlay;


public:
	nsWorld(nsName name, bool bHasPhysics);
	void Initialize();
	void Destroy();
	void CleanupActors();
	void DispatchStartPlay();
	void DispatchStopPlay();
	void DispatchTickUpdate(float deltaTime);

private:
	void RefreshActorList();

public:
	NS_NODISCARD nsLevel* FindLevel(const nsName& levelName) const;
	nsLevel* CreateLevel(nsName levelName);
	void DestroyLevel(nsName levelName);


	// Get persistent level
	NS_NODISCARD_INLINE nsLevel* GetPersistentLevel() const noexcept
	{
		return Levels[0];
	}


	// Get all levels
	NS_NODISCARD_INLINE const nsTArray<nsLevel*>& GetLevels() const noexcept
	{
		return Levels;
	}


	void DestroyActor(nsActor* actor);


	template<typename TActor = nsActor>
	NS_INLINE TActor* CreateActor(nsName name, const nsTransform& optTransform = nsTransform(), nsActor* optParent = nullptr, nsLevel* optLevel = nullptr)
	{
		static_assert(std::is_base_of<nsActor, TActor>::value, "CreateActor type of <TActor> must be derived from type <nsActor>!");
		NS_Validate_IsMainThread();

		nsLevel* level = optLevel ? optLevel : GetPersistentLevel();

		TActor* newActor = ActorMemory.AllocateConstruct<TActor>();
		newActor->Name = name;
		newActor->Level = level;

		uint32& flags = newActor->Flags;
		if (flags & nsEActorFlag::CallStartStopPlay) StartStopPlayActors.Add(newActor);
		if (flags & nsEActorFlag::CallTickUpdate) TickUpdateActors.Add(newActor);
		
		newActor->SetWorldTransform(optTransform);
		newActor->OnInitialize();

		if (bHasStartedPlay && (flags & nsEActorFlag::CallStartStopPlay) )
		{
			newActor->OnStartPlay();
		}

		ActorList.Add(newActor);
		level->AddActor(newActor);

		return newActor;
	}


	template<typename TActor = nsActor>
	NS_INLINE TActor* CreateActor(nsName name, const nsVector3& position, const nsQuaternion& rotation = nsQuaternion::IDENTITY, const nsVector3& scale = 1.0f) noexcept
	{
		return CreateActor<TActor>(name, nsTransform(position, rotation, scale));
	}


	// Get all actors
	NS_NODISCARD_INLINE const nsTArray<nsActor*>& GetAllActors() const noexcept
	{
		return ActorList;
	}


	NS_NODISCARD_INLINE const nsName& GetName() const noexcept
	{
		return Name;
	}


	// Get start time in seconds when start play 
	NS_NODISCARD_INLINE float GetStartTimeSeconds() const noexcept
	{
		return StartTimeSeconds;
	}


	// Get elapsed time in seconds since start play has called
	NS_NODISCARD_INLINE float GetElapsedTimeSeconds() const noexcept
	{
		if (!bHasStartedPlay)
		{
			return 0.0f;
		}

		const float currentTime = static_cast<float>(nsPlatform::PerformanceQuery_Counter()) / static_cast<float>(nsPlatform::PerformanceQuery_Frequency());
		return currentTime - StartTimeSeconds;
	}


	// Get delta time in seconds
	NS_NODISCARD_INLINE float GetDeltaTimeSeconds() const noexcept
	{
		return DeltaTimeSeconds;
	}

};
