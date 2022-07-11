#pragma once

#include "nsAnimationTypes.h"
#include "nsVulkan.h"



class NS_ENGINE_API nsAnimationManager
{
	NS_DECLARE_SINGLETON(nsAnimationManager)

private:
	bool bInitialized;

	struct Frame
	{
		nsVulkanBuffer* SkeletonPoseTransformStorageBuffer;
	};


	Frame FrameDatas[NS_ENGINE_FRAME_BUFFERING];
	int FrameIndex;


	enum Flag
	{
		Flag_None					= (0),
		Flag_Allocated				= (1 << 0),
		Flag_PendingDestroy			= (1 << 1),
		Flag_Instance_UpdatePose	= (1 << 2),
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
	void UpdateAnimationPoses(float deltaTime);


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
	void SetInstanceUpdatePose(nsAnimationInstanceID instance, bool bUpdatePose);
	void PlayAnimation(nsAnimationInstanceID instance, nsAnimationClipID clip, float playRate, bool bLoop);
	void StopAnimation(nsAnimationInstanceID instance);
	void BlendAnimation(nsAnimationInstanceID instance, nsEAnimationTransitionMode transitionMode, float blendFactor, nsAnimationClipID clip, float playRate, bool bLoop);


	NS_NODISCARD_INLINE bool IsInstanceValid(nsAnimationInstanceID instance) const
	{
		return instance.IsValid() && InstanceFlags.IsValid(instance.Id) && !(InstanceFlags[instance.Id] & Flag_PendingDestroy);
	}

	NS_NODISCARD_INLINE int GetInstanceBoneTransformIndex(nsAnimationInstanceID instance) const
	{
		NS_Assert(IsInstanceValid(instance));
		return InstanceDatas[instance.Id].BoneTransformIndex;
	}


	void BeginFrame(int frameIndex);
	void UpdateRenderResources();


	NS_NODISCARD_INLINE nsVulkanBuffer* GetSkeletonPoseTransformStorageBuffer() const
	{
		return FrameDatas[FrameIndex].SkeletonPoseTransformStorageBuffer;
	}


#ifdef NS_ENGINE_DEBUG_DRAW
private:
	nsTMap<int, nsTransform> InstanceDebugDraws;

public:
	void SetInstanceDebugDraw(nsAnimationInstanceID instance, bool bDebugDraw, nsTransform rootWorldTransform);
	void DebugDraw(class nsRenderer* renderer);

#endif // NS_ENGINE_DEBUG_DRAW

};
