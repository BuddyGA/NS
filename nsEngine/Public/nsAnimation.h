#pragma once

#include "nsVulkan.h"



struct nsAnimationSkeletonData
{
	struct Bone
	{
		nsMatrix4 InverseBindPoseTransform;
		nsTransform LocalTransform;
		int ParentId;


		friend NS_INLINE void operator|(nsStream& stream, Bone& bone)
		{
			stream | bone.InverseBindPoseTransform;
			stream | bone.LocalTransform;
			stream | bone.ParentId;
		}

	};

	nsTArrayInline<nsName, NS_ENGINE_ANIMATION_SKELETON_MAX_BONE> BoneNames;
	nsTArrayInline<Bone, NS_ENGINE_ANIMATION_SKELETON_MAX_BONE> BoneDatas;


	friend NS_INLINE void operator|(nsStream& stream, nsAnimationSkeletonData& animationSkeletonData)
	{
		stream | animationSkeletonData.BoneNames;
		stream | animationSkeletonData.BoneDatas;
	}

};



struct nsAnimationKeyFrame
{
	nsQuaternion Rotation;
	nsVector3 Position;
	nsVector3 Scale;
	float RotationTime;
	float PositionTime;
	float ScaleTime;
	bool bHasRotation;
	bool bHasPosition;
	bool bHasScale;


public:
	nsAnimationKeyFrame()
		: Rotation(nsQuaternion::IDENTITY)
		, Position(0.0f)
		, Scale(1.0f)
		, RotationTime(0.0f)
		, PositionTime(0.0f)
		, ScaleTime(0.0f)
		, bHasRotation(false)
		, bHasPosition(false)
		, bHasScale(false)
	{
	}

};



struct nsAnimationSequenceData
{
	nsAnimationSkeletonID Skeleton;
	float Duration;
	int FramePerSecond;
	nsTArrayInline<nsAnimationKeyFrame, NS_ENGINE_ANIMATION_SEQUENCE_MAX_KEY_FRAME> KeyFrames;


public:
	nsAnimationSequenceData()
		: Skeleton(nsAnimationSkeletonID::INVALID)
		, Duration(0.0f)
		, FramePerSecond(0)
	{
	}

};



struct nsAnimationInstanceData
{
	// Skeleton Id
	nsAnimationSkeletonID Skeleton;

	// Transform for each bone in skeleton
	nsTArray<nsMatrix4> PoseTransforms;
};




class NS_ENGINE_API nsAnimationManager
{
	NS_DECLARE_SINGLETON(nsAnimationManager)

private:
	bool bInitialized;

	struct Frame
	{
		nsVulkanBuffer* AnimationPoseTransformStorageBuffer;
	};


	Frame FrameDatas[NS_ENGINE_FRAME_BUFFERING];
	int FrameIndex;


	enum Flag
	{
		Flag_None				= (0),
		Flag_Allocated			= (1 << 0),
		Flag_PendingDestroy		= (1 << 1),
	};


	// Skeleton
	nsTArrayFreeList<nsName> SkeletonNames;
	nsTArrayFreeList<uint32> SkeletonFlags;
	nsTArrayFreeList<nsAnimationSkeletonData> SkeletonDatas;
	

	// Animation Sequence
	nsTArrayFreeList<nsName> SequenceNames;
	nsTArrayFreeList<uint32> SequenceFlags;
	nsTArrayFreeList<nsAnimationSequenceData> SequenceDatas;


	// Animation Instance
	nsTArrayFreeList<nsName> InstanceNames;
	nsTArrayFreeList<uint32> InstanceFlags;
	nsTArrayFreeList<nsAnimationInstanceData> InstanceDatas;


public:
	void Initialize();


	NS_NODISCARD nsAnimationSkeletonID FindSkeleton(const nsName& name) const;
	NS_NODISCARD nsAnimationSkeletonID CreateSkeleton(nsName name);
	void DestroySkeleton(nsAnimationSkeletonID& skeleton);

	NS_NODISCARD_INLINE bool IsSkeletonValid(nsAnimationSkeletonID skeleton) const
	{
		return skeleton.IsValid() && SkeletonFlags.IsValid(skeleton.Id) && !(SkeletonFlags[skeleton.Id] & Flag_PendingDestroy);
	}

	NS_NODISCARD_INLINE nsAnimationSkeletonData& GetSkeletonData(nsAnimationSkeletonID skeleton)
	{
		NS_Assert(IsSkeletonValid(skeleton));
		return SkeletonDatas[skeleton.Id];
	}

	NS_NODISCARD_INLINE const nsAnimationSkeletonData& GetSkeletonData(nsAnimationSkeletonID skeleton) const
	{
		NS_Assert(IsSkeletonValid(skeleton));
		return SkeletonDatas[skeleton.Id];
	}

	NS_NODISCARD_INLINE nsName GetSkeletonName(nsAnimationSkeletonID skeleton) const
	{
		NS_Assert(IsSkeletonValid(skeleton));
		return SkeletonNames[skeleton.Id];
	}



	NS_NODISCARD nsAnimationSequenceID FindSequence(const nsName& name) const;
	NS_NODISCARD nsAnimationSequenceID CreateSequence(nsName name, nsAnimationSkeletonID skeleton);
	void DestroySequence(nsAnimationSequenceID& sequence);
	void UpdateSequenceData(nsAnimationSequenceID sequence, const nsAnimationKeyFrame* keyFrames, int keyFrameCount, float duration);

	NS_NODISCARD_INLINE bool IsAnimationSequenceValid(nsAnimationSequenceID sequence) const
	{
		return sequence.IsValid() && SequenceFlags.IsValid(sequence.Id) && !(SequenceFlags[sequence.Id] & Flag_PendingDestroy);
	}


	NS_NODISCARD nsAnimationInstanceID FindInstance(const nsName& name) const;
	NS_NODISCARD nsAnimationInstanceID CreateInstance(nsName name, nsAnimationSkeletonID skeleton);
	void DestroyInstance(nsAnimationInstanceID& instance);

	NS_NODISCARD_INLINE bool IsInstanceValid(nsAnimationInstanceID instance) const
	{
		return instance.IsValid() && InstanceFlags.IsValid(instance.Id) && !(InstanceFlags[instance.Id] & Flag_PendingDestroy);
	}


	void BeginFrame(int frameIndex);
	void BindAnimationInstances(const nsAnimationInstanceID* animationInstances, int count);
	void Update(float deltaTime);

};
