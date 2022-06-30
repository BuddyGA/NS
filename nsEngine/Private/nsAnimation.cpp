#include "nsAnimation.h"


NS_ENGINE_DEFINE_HANDLE(nsAnimationSkeletonID);
NS_ENGINE_DEFINE_HANDLE(nsAnimationInstanceID);
NS_ENGINE_DEFINE_HANDLE(nsAnimationSequenceID);


static nsLogCategory AnimationLog("nsAnimationLog", nsELogVerbosity::LV_DEBUG);



nsAnimationManager::nsAnimationManager() noexcept
	: bInitialized(false)
	, FrameDatas()
	, FrameIndex(0)
{
	SkeletonNames.Reserve(4);
	SkeletonFlags.Reserve(4);
	SkeletonDatas.Reserve(4);

	SequenceNames.Reserve(16);
	SequenceFlags.Reserve(16);
	SequenceDatas.Reserve(16);

	InstanceNames.Reserve(16);
	InstanceFlags.Reserve(16);
	InstanceDatas.Reserve(16);
}


void nsAnimationManager::Initialize()
{
	if (bInitialized)
	{
		return;
	}

	NS_LogInfo(AnimationLog, "Initialize animation manager");

	for (int i = 0; i < NS_ENGINE_FRAME_BUFFERING; ++i)
	{
		Frame& frame = FrameDatas[i];
		frame.SkeletonPoseTransformStorageBuffer = nsVulkan::CreateStorageBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, NS_MEMORY_SIZE_MiB(1), nsName::Format("skel_pose_transform_ssbo_%i", i));
	}

	bInitialized = true;
}


void nsAnimationManager::UpdateAnimationPose(float deltaTime)
{
	if (InstanceDatas.IsEmpty())
	{
		return;
	}

	const int count = InstanceDatas.GetCount();

	for (int i = 0; i < count; ++i)
	{
		nsAnimationInstanceData& animInstanceData = InstanceDatas[i];
		
		if (!animInstanceData.bUpdatePose)
		{
			continue;
		}

		const int boneCount = animInstanceData.BoneTransforms.GetCount();

		for (int j = 0; j < boneCount; ++j)
		{
			nsAnimationSkeletonData::Bone& bone = animInstanceData.BoneTransforms[j];
			const nsAnimationSkeletonData::Bone* parentBone = bone.ParentId == -1 ? nullptr : &animInstanceData.BoneTransforms[bone.ParentId];
			bone.PoseTransform = parentBone ? bone.LocalTransform.ToMatrixNoScale() * parentBone->PoseTransform : bone.LocalTransform.ToMatrixNoScale();

			const int boneTransformId = animInstanceData.BoneTransformIndex + j;
			InstanceBoneTransforms[boneTransformId] = bone.InverseBindPoseTransform * bone.PoseTransform;
		}
	}
}


nsAnimationSkeletonID nsAnimationManager::FindSkeleton(const nsName& name) const
{
	for (auto it = SkeletonNames.CreateConstIterator(); it; ++it)
	{
		if ((*it) == name)
		{
			return it.GetIndex();
		}
	}

	return nsAnimationSkeletonID::INVALID;
}


nsAnimationSkeletonID nsAnimationManager::CreateSkeleton(nsName name)
{
	NS_Validate_IsMainThread();

	if (FindSkeleton(name) != nsAnimationSkeletonID::INVALID)
	{
		NS_LogWarning(AnimationLog, "Skeleton with name [%s] already exists!", *name);
	}

	const int nameId = SkeletonNames.Add();
	const int flagId = SkeletonFlags.Add();
	const int dataId = SkeletonDatas.Add();
	NS_Assert(nameId == flagId && flagId == dataId);

	SkeletonNames[nameId] = name;
	SkeletonFlags[flagId] = Flag_Allocated;

	nsAnimationSkeletonData& data = SkeletonDatas[dataId];
	data.BoneNames.Clear();
	data.BoneDatas.Clear();

	return nameId;
}


void nsAnimationManager::DestroySkeleton(nsAnimationSkeletonID& skeleton)
{
	NS_Validate_IsMainThread();

	if (IsSkeletonValid(skeleton))
	{
		NS_ValidateV(0, "Not implemented yet!");
	}

	skeleton = nsAnimationSkeletonID::INVALID;
}


nsAnimationSequenceID nsAnimationManager::FindSequence(const nsName& name) const
{
	for (auto it = SequenceNames.CreateConstIterator(); it; ++it)
	{
		if ((*it) == name)
		{
			return it.GetIndex();
		}
	}

	return nsAnimationSequenceID::INVALID;
}


nsAnimationSequenceID nsAnimationManager::CreateSequence(nsName name, nsAnimationSkeletonID skeleton)
{
	NS_Validate_IsMainThread();
	NS_Assert(IsSkeletonValid(skeleton));

	if (FindSequence(name) != nsAnimationSequenceID::INVALID)
	{
		NS_LogWarning(AnimationLog, "Animation sequence with name [%s] already exists!", *name);
	}

	const int nameId = SequenceNames.Add();
	const int flagId = SequenceFlags.Add();
	const int dataId = SequenceDatas.Add();
	NS_Assert(nameId == flagId && flagId == dataId);

	SequenceNames[nameId] = name;
	SequenceFlags[flagId] = Flag_Allocated;

	nsAnimationSequenceData& data = SequenceDatas[dataId];
	data.Skeleton = skeleton;
	data.Duration = 0.0f;
	data.FramePerSecond = 0;
	data.KeyFrames.Clear();

	return nameId;
}


void nsAnimationManager::DestroySequence(nsAnimationSequenceID& sequence)
{
	NS_Validate_IsMainThread(); 

	if (IsSequenceValid(sequence))
	{
		NS_ValidateV(0, "Not implemented yet!");
	}

	sequence = nsAnimationSequenceID::INVALID;
}


void nsAnimationManager::UpdateSequenceData(nsAnimationSequenceID sequence, const nsAnimationKeyFrame* keyFrames, int keyFrameCount, float duration)
{
}


nsAnimationInstanceID nsAnimationManager::FindInstance(const nsName& name) const
{
	for (auto it = InstanceNames.CreateConstIterator(); it; ++it)
	{
		if ((*it) == name)
		{
			return it.GetIndex();
		}
	}

	return nsAnimationInstanceID::INVALID;
}


nsAnimationInstanceID nsAnimationManager::CreateInstance(nsName name, nsAnimationSkeletonID skeleton)
{
	NS_Validate_IsMainThread();
	NS_Assert(IsSkeletonValid(skeleton));

	if (FindInstance(name) != nsAnimationInstanceID::INVALID)
	{
		NS_LogWarning(AnimationLog, "Animation skeleton with name [%s] already exists!", *name);
	}

	const int nameId = InstanceNames.Add();
	const int flagId = InstanceFlags.Add();
	const int dataId = InstanceDatas.Add();
	NS_Assert(nameId == flagId && flagId == dataId);

	InstanceNames[nameId] = name;
	InstanceFlags[flagId] = Flag_Allocated;

	nsAnimationSkeletonData& skeletonData = SkeletonDatas[skeleton.Id];
	const int boneCount = skeletonData.BoneNames.GetCount();

	nsAnimationInstanceData& data = InstanceDatas[dataId];
	data.BoneNames = skeletonData.BoneNames;
	data.BoneTransforms = skeletonData.BoneDatas;
	data.BoneTransformIndex = InstanceBoneTransforms.GetCount();
	data.bUpdatePose = true;

	InstanceBoneTransforms.ResizeConstructs(data.BoneTransformIndex + boneCount, nsMatrix4::IDENTITY);

	NS_LogDebug(AnimationLog, "Create animation instance [%s]", *name);

	return nameId;
}


void nsAnimationManager::DestroyInstance(nsAnimationInstanceID& instance)
{
	NS_Validate_IsMainThread();

	if (IsInstanceValid(instance))
	{
		NS_ValidateV(0, "Not implemented yet!");
	}

	instance = nsAnimationInstanceID::INVALID;
}


void nsAnimationManager::BeginFrame(int frameIndex)
{
	FrameIndex = frameIndex;

	Frame& frame = FrameDatas[FrameIndex];
	frame.AnimationInstanceToBinds.Clear();
}


void nsAnimationManager::BindAnimationInstances(const nsAnimationInstanceID* animationInstances, int count)
{
	if (animationInstances == nullptr || count <= 0)
	{
		return;
	}

	Frame& frame = FrameDatas[FrameIndex];

	for (int i = 0; i < count; ++i)
	{
		const nsAnimationInstanceID& animInstance = animationInstances[i];
		NS_Assert(IsInstanceValid(animInstance));

		const uint32& flags = InstanceFlags[animInstance.Id];
		NS_AssertV(!(flags & Flag_PendingDestroy), "Cannot bind animation instance that has marked pending destroy!");

		frame.AnimationInstanceToBinds.Add(animInstance);
	}
}


void nsAnimationManager::UpdateRenderResources()
{
	Frame& frame = FrameDatas[FrameIndex];

	if (frame.AnimationInstanceToBinds.GetCount() == 0)
	{
		return;
	}

	const uint64 storageBufferSize = sizeof(nsMatrix4) * InstanceBoneTransforms.GetCount();
	frame.SkeletonPoseTransformStorageBuffer->Resize(storageBufferSize);

	void* map = frame.SkeletonPoseTransformStorageBuffer->MapMemory();
	nsPlatform::Memory_Copy(map, InstanceBoneTransforms.GetData(), storageBufferSize);
	frame.SkeletonPoseTransformStorageBuffer->UnmapMemory();
}
