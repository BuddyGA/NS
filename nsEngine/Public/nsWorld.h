#pragma once

#include "nsLevel.h"
#include "nsPhysicsTypes.h"



class NS_ENGINE_API nsWorld : public nsObject
{
	NS_DECLARE_OBJECT()

private:
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
	nsTArray<nsActor*> PostPhysicsTickUpdateActors;
	nsTArray<nsActor*> PendingDestroyActors;


public:
	nsWorld(nsString name, bool bInitPhysics);
	void Initialize();
	void Destroy();
	void CleanupPendingDestroyLevelsAndActors();
	void DispatchStartPlay();
	void DispatchStopPlay();
	void DispatchPrePhysicsTickUpdate(float deltaTime);
	void DispatchPhysicsTickUpdate(float fixedDeltaTime);
	void DispatchPostPhysicsTickUpdate();
	void SyncActorTransformsWithPhysics();
	bool PhysicsRayCast(nsPhysicsHitResult& hitResult, const nsVector3& origin, const nsVector3& direction, float distance, const nsPhysicsQueryParams& params = nsPhysicsQueryParams());

private:
	void RefreshActorList();

public:
	NS_NODISCARD nsLevel* FindLevel(const nsString& levelName) const;
	nsLevel* CreateLevel(nsString levelName);
	void DestroyLevel(nsString levelName);


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
	void InitActor(nsActor* actor, nsString name, bool bIsStatic, const nsTransform& optTransform = nsTransform(), nsActor* optParent = nullptr);

public:
	void DestroyActor(nsActor*& actor);
	void AddActorToLevel(nsActor* actor, nsLevel* level = nullptr);
	void RemoveActorFromLevel(nsActor* actor);


	template<typename TActor = nsActor>
	NS_NODISCARD_INLINE TActor* CreateActor(nsString name, bool bIsStatic, const nsTransform& optTransform = nsTransform(), nsActor* optParent = nullptr)
	{
		static_assert(std::is_base_of<nsActor, TActor>::value, "CreateActor type of <TActor> must be derived from type <nsActor>!");

		TActor* newActor = ActorMemory.AllocateConstruct<TActor>();
		InitActor(newActor, name, bIsStatic, optTransform, optParent);

		return newActor;
	}


	template<typename TActor = nsActor>
	NS_NODISCARD_INLINE TActor* CreateActor(nsString name, bool bIsStatic, const nsVector3& position, const nsQuaternion& rotation, const nsVector3& scale = nsVector3(1.0f))
	{
		return CreateActor<TActor>(name, bIsStatic, nsTransform(position, rotation, scale));
	}


	template<typename TActor>
	NS_NODISCARD_INLINE void FindActorsOfClass(nsTArray<TActor*>& outActors) const
	{
		static_assert(std::is_base_of<nsActor, TActor>::value, "FindActorsOfClass type of <TActor> must be derived from type <nsActor>!");

		const int actorCount = ActorList.GetCount();

		for (int i = 0; i < actorCount; ++i)
		{
			if (TActor* actor = ns_Cast<TActor>(ActorList[i]))
			{
				outActors.Add(actor);
			}
		}
	}


	// Get all actors
	NS_NODISCARD_INLINE const nsTArray<nsActor*>& GetAllActors() const noexcept
	{
		return ActorList;
	}


	NS_NODISCARD_INLINE physx::PxScene* GetPhysicsScene() const noexcept
	{
		return PhysicsScene;
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
