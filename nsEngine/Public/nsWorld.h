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

	physx::PxScene* PhysicsScene;
	nsTArray<nsLevel*> Levels;

	nsMemory ActorMemory;
	nsTArray<nsActor*> ActorList;
	nsTArray<nsActor*> StartStopPlayActors;
	nsTArray<nsActor*> PrePhysicsTickUpdateActors;
	nsTArray<nsActor*> PhysicsTickUpdateActors;
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
	void DispatchPhysicsTickUpdate(float fixedDeltaTime);
	void SyncActorTransformsWithPhysics();
	bool PhysicsRayCast(nsPhysicsHitResult& hitResult, const nsVector3& origin, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams());

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


private:
	void InitActor(nsActor* actor, nsName name, bool bIsStatic, const nsTransform& optTransform = nsTransform(), nsActor* optParent = nullptr);

public:
	void DestroyActor(nsActor*& actor);
	void AddActorToLevel(nsActor* actor, nsLevel* level = nullptr);
	void RemoveActorFromLevel(nsActor* actor);


	template<typename TActor = nsActor>
	NS_NODISCARD_INLINE TActor* CreateActor(nsName name, bool bIsStatic, const nsTransform& optTransform = nsTransform(), nsActor* optParent = nullptr)
	{
		static_assert(std::is_base_of<nsActor, TActor>::value, "CreateActor type of <TActor> must be derived from type <nsActor>!");

		TActor* newActor = ActorMemory.AllocateConstruct<TActor>();
		InitActor(newActor, name, bIsStatic, optTransform, optParent);

		return newActor;
	}


	template<typename TActor = nsActor>
	NS_NODISCARD_INLINE TActor* CreateActor(nsName name, bool bIsStatic, const nsVector3& position, const nsQuaternion& rotation, const nsVector3& scale = nsVector3(1.0f))
	{
		return CreateActor<TActor>(name, bIsStatic, nsTransform(position, rotation, scale));
	}


	NS_NODISCARD_INLINE physx::PxScene* GetPhysicsScene() const noexcept
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


	// Get time in seconds since start play has called
	NS_NODISCARD_INLINE float GetCurrentTimeSeconds() const noexcept
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


	NS_NODISCARD_INLINE bool HasStartedPlay() const
	{
		return bHasStartedPlay;
	}

};
