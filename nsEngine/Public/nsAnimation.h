#pragma once

#include "nsVulkan.h"



struct nsAnimationSkeletonData
{
	struct Bone
	{
		nsMatrix4 InverseBindPoseTransform;
		nsMatrix4 PoseTransform;
		nsTransform LocalTransform;
		int ParentId;


		friend NS_INLINE void operator|(nsStream& stream, Bone& bone)
		{
			stream | bone.InverseBindPoseTransform;
			stream | bone.PoseTransform;
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
	template<typename T>
	struct TChannel
	{
		T Value;
		float Timestamp;
	};

	nsTArray<TChannel<nsVector3>> PositionChannels;
	nsTArray<TChannel<nsQuaternion>> RotationChannels;
	nsTArray<TChannel<nsVector3>> ScaleChannels;


	friend NS_INLINE void operator|(nsStream& stream, nsAnimationKeyFrame& animationKeyFrame)
	{
		stream | animationKeyFrame.PositionChannels;
		stream | animationKeyFrame.RotationChannels;
		stream | animationKeyFrame.ScaleChannels;
	}

};



struct nsAnimationClipData
{
	// Compatible skeleton
	nsName SkeletonName;

	// Frame count
	int FrameCount;

	// Duration (seconds)
	float Duration;

	// Key-frames for each bone
	nsTArray<nsAnimationKeyFrame> KeyFrames;


public:
	nsAnimationClipData()
		: SkeletonName()
		, FrameCount(0)
		, Duration(0.0f)
	{
	}


	friend NS_INLINE void operator|(nsStream& stream, nsAnimationClipData& animationSequenceData)
	{
		stream | animationSequenceData.SkeletonName;
		stream | animationSequenceData.FrameCount;
		stream | animationSequenceData.Duration;
		stream | animationSequenceData.KeyFrames;
	}

};



struct nsAnimationInstanceData
{
	// Skeleton bone names
	nsTArrayInline<nsName, NS_ENGINE_ANIMATION_SKELETON_MAX_BONE> BoneNames;

	// Skeleton bone transforms
	nsTArrayInline<nsAnimationSkeletonData::Bone, NS_ENGINE_ANIMATION_SKELETON_MAX_BONE> BoneTransforms;

	// Skeleton which this instanced from
	nsAnimationSkeletonID Skeleton;

	// Bone transform index
	int BoneTransformIndex;

	// Update/pause pose
	bool bUpdatePose;


public:
	nsAnimationInstanceData()
	{
		Skeleton = nsAnimationSkeletonID::INVALID;
		BoneTransformIndex = -1;
		bUpdatePose = false;
	}

};



enum class nsEAnimationTransitionMode : uint8
{
	SMOOTH = 0,
	FROZEN
};


struct nsAnimationPlayState
{
	nsAnimationClipID Clip;
	float PlayRate;
	float Timestamp;
	bool bLooping;


public:
	nsAnimationPlayState()
	{
		Clip = nsAnimationClipID::INVALID;
		PlayRate = 1.0f;
		Timestamp = 0.0f;
		bLooping = false;
	}

};


struct nsAnimationBlendState
{
	nsAnimationClipID Clip;
	nsEAnimationTransitionMode TransitionMode;
	float BlendFactor;


public:
	nsAnimationBlendState()
		: Clip(nsAnimationClipID::INVALID)
		, TransitionMode(nsEAnimationTransitionMode::SMOOTH)
		, BlendFactor(0.2f)
	{
	}

};



class NS_ENGINE_API nsAnimationManager
{
	NS_DECLARE_SINGLETON(nsAnimationManager)

private:
	bool bInitialized;

	struct Frame
	{
		nsVulkanBuffer* SkeletonPoseTransformStorageBuffer;
		nsTArray<nsAnimationInstanceID> AnimationInstanceToBinds;
	};


	Frame FrameDatas[NS_ENGINE_FRAME_BUFFERING];
	int FrameIndex;


	enum Flag
	{
		Flag_None				= (0),
		Flag_Allocated			= (1 << 0),
		Flag_PendingDestroy		= (1 << 1),
	};


	// Animation Skeleton
	nsTArrayFreeList<nsName> SkeletonNames;
	nsTArrayFreeList<uint32> SkeletonFlags;
	nsTArrayFreeList<nsAnimationSkeletonData> SkeletonDatas;
	

	// Animation Clip
	nsTArrayFreeList<nsName> ClipNames;
	nsTArrayFreeList<uint32> ClipFlags;
	nsTArrayFreeList<nsAnimationClipData> ClipDatas;


	// Animation Instance
	nsTArrayFreeList<nsName> InstanceNames;
	nsTArrayFreeList<uint32> InstanceFlags;
	nsTArrayFreeList<nsAnimationInstanceData> InstanceDatas;
	nsTArrayFreeList<nsAnimationPlayState> InstancePlayStates;


	nsTArray<nsMatrix4> InstanceBoneTransforms;


public:
	void Initialize();
	void UpdateAnimationPose(float deltaTime);


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



	NS_NODISCARD nsAnimationClipID FindClip(const nsName& name) const;
	NS_NODISCARD nsAnimationClipID CreateClip(nsName name);
	NS_NODISCARD nsAnimationClipID CreateClip(nsName name, nsAnimationSkeletonID skeleton);
	NS_NODISCARD nsAnimationClipID CreateClip(nsName name, nsName skeletonName);
	void DestroyClip(nsAnimationClipID& clip);

	NS_NODISCARD_INLINE bool IsClipValid(nsAnimationClipID clip) const
	{
		return clip.IsValid() && ClipFlags.IsValid(clip.Id) && !(ClipFlags[clip.Id] & Flag_PendingDestroy);
	}

	NS_NODISCARD_INLINE nsAnimationClipData& GetClipData(nsAnimationClipID clip)
	{
		NS_Assert(IsClipValid(clip));
		return ClipDatas[clip.Id];
	}

	NS_NODISCARD_INLINE const nsAnimationClipData& GetClipData(nsAnimationClipID clip) const
	{
		NS_Assert(IsClipValid(clip));
		return ClipDatas[clip.Id];
	}

	NS_NODISCARD_INLINE nsName GetClipName(nsAnimationClipID clip) const
	{
		NS_Assert(IsClipValid(clip));
		return ClipNames[clip.Id];
	}



	NS_NODISCARD nsAnimationInstanceID FindInstance(const nsName& name) const;
	NS_NODISCARD nsAnimationInstanceID CreateInstance(nsName name, nsAnimationSkeletonID skeleton);
	void DestroyInstance(nsAnimationInstanceID& instance);
	void PlayAnimation(nsAnimationInstanceID instance, nsAnimationClipID clip, float playRate, bool bLoop);
	void StopAnimation(nsAnimationInstanceID instance);
	void BlendAnimation(nsAnimationInstanceID instance, nsEAnimationTransitionMode transitionMode, float blendFactor, nsAnimationClipID clip, float playRate, bool bLoop);


	NS_NODISCARD_INLINE bool IsInstanceValid(nsAnimationInstanceID instance) const
	{
		return instance.IsValid() && InstanceFlags.IsValid(instance.Id) && !(InstanceFlags[instance.Id] & Flag_PendingDestroy);
	}

	NS_NODISCARD_INLINE int GetInstanceBoneTransformIndex(nsAnimationInstanceID animationInstance) const
	{
		NS_Assert(IsInstanceValid(animationInstance));
		return InstanceDatas[animationInstance.Id].BoneTransformIndex;
	}


	void BeginFrame(int frameIndex);
	void BindAnimationInstances(const nsAnimationInstanceID* animationInstances, int count);
	void UpdateRenderResources();


	NS_NODISCARD_INLINE nsVulkanBuffer* GetSkeletonPoseTransformStorageBuffer() const
	{
		return FrameDatas[FrameIndex].SkeletonPoseTransformStorageBuffer;
	}

};
