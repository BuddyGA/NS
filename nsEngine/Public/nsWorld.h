#pragma once

#include "nsLevel.h"



class NS_ENGINE_API nsWorld : public nsObject
{
	NS_DECLARE_OBJECT()

private:
	nsName Name;
	float StartTimeSeconds;
	float DeltaTimeSeconds;
	bool bHasPhysics;
	bool bHasStartedPlay;

	nsPhysicsSceneID PhysicsScene;
	nsTArray<nsLevel*> Levels;

	nsMemory ActorMemory;
	nsTArray<nsActor*> ActorList;
	nsTArray<nsActor*> StartStopPlayActors;
	nsTArray<nsActor*> TickUpdateActors;
	nsTArray<nsActor*> PendingDestroyActors;

public:
	bool bOnlyTickAfterStartedPlay;


public:
	nsWorld(nsName name, bool bInitPhysics);
	void Initialize();
	void Destroy();
	void CleanupPendingDestroyLevelsAndActors();
	void DispatchStartPlay();
	void DispatchStopPlay();
	void DispatchTickUpdate(float deltaTime);
	void SyncActorTransformsWithPhysics();

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


	nsActor* CreateActor(nsName name, const nsTransform& optTransform = nsTransform(), nsActor* optParent = nullptr);
	nsActor* CreateActor(nsName name, const nsVector3& position, const nsQuaternion& rotation = nsQuaternion::IDENTITY, const nsVector3& scale = 1.0f);
	void DestroyActor(nsActor*& actor);
	void AddActorToLevel(nsActor* actor, nsLevel* level = nullptr);
	void RemoveActorFromLevel(nsActor* actor);


	NS_NODISCARD_INLINE nsPhysicsSceneID GetPhysicsScene() const noexcept
	{
		return PhysicsScene;
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
