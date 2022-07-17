#pragma once

#include "nsEngineTypes.h"


NS_ENGINE_DECLARE_HANDLE(nsAnimationSkeletonID, nsAnimationManager)
NS_ENGINE_DECLARE_HANDLE(nsAnimationClipID, nsAnimationManager)
NS_ENGINE_DECLARE_HANDLE(nsAnimationInstanceID, nsAnimationManager)



struct nsAnimationSkeletonData
{
	struct Bone
	{
		nsMatrix4 InverseBindPoseTransform;
		nsMatrix4 PoseTransform;
		nsTransform LocalTransform;
		int ParentId;
		bool bUpdated;


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


public:
	nsAnimationInstanceData()
	{
		Skeleton = nsAnimationSkeletonID::INVALID;
		BoneTransformIndex = -1;
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



class nsAnimationGraph;
