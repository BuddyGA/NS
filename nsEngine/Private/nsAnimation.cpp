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

	bInitialized = true;
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

	if (IsAnimationSequenceValid(sequence))
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

	nsAnimationInstanceData& data = InstanceDatas[dataId];
	data.Skeleton = skeleton;


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
}


void nsAnimationManager::BindAnimationInstances(const nsAnimationInstanceID* animationInstances, int count)
{

}


void nsAnimationManager::Update(float deltaTime)
{

}
