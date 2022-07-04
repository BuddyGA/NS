#include "nsAnimation.h"
#include "nsConsole.h"


NS_ENGINE_DEFINE_HANDLE(nsAnimationSkeletonID);
NS_ENGINE_DEFINE_HANDLE(nsAnimationInstanceID);
NS_ENGINE_DEFINE_HANDLE(nsAnimationClipID);


static nsLogCategory AnimationLog("nsAnimationLog", nsELogVerbosity::LV_DEBUG);



nsAnimationManager::nsAnimationManager() noexcept
	: bInitialized(false)
	, FrameDatas()
	, FrameIndex(0)
{
	SkeletonNames.Reserve(4);
	SkeletonFlags.Reserve(4);
	SkeletonDatas.Reserve(4);

	ClipNames.Reserve(16);
	ClipFlags.Reserve(16);
	ClipDatas.Reserve(16);

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
		if (!(InstanceFlags[i] & Flag_Instance_UpdatePose))
		{
			continue;
		}

		nsAnimationInstanceData& animInstanceData = InstanceDatas[i];

		nsAnimationPlayState& state = InstancePlayStates[i];

		if (state.Clip == nsAnimationClipID::INVALID)
		{
			continue;
		}

		const nsAnimationClipData& clipData = ClipDatas[state.Clip.Id];
		state.Timestamp += deltaTime * state.PlayRate;
		
		if (state.bLooping)
		{
			state.Timestamp = nsMath::ModF(state.Timestamp, clipData.Duration);
		}
		else
		{
			state.Timestamp = nsMath::Clamp(state.Timestamp, 0.0f, clipData.Duration);
		}
		
		const int boneCount = animInstanceData.BoneTransforms.GetCount();
		NS_Assert(clipData.KeyFrames.GetCount() == boneCount);

		for (int j = 0; j < boneCount; ++j)
		{
			nsAnimationSkeletonData::Bone& bone = animInstanceData.BoneTransforms[j];
			bone.bUpdated = false;

			const nsTArray<nsAnimationKeyFrame::TChannel<nsVector3>>& pChannels = clipData.KeyFrames[j].PositionChannels;

			for (int p = 0; p < pChannels.GetCount() - 1; ++p)
			{
				if (state.Timestamp >= pChannels[p].Timestamp && state.Timestamp <= pChannels[p + 1].Timestamp)
				{
					const float pa = (state.Timestamp - pChannels[p].Timestamp) / (pChannels[p + 1].Timestamp - pChannels[p].Timestamp);
					bone.LocalTransform.Position = nsVector3::Lerp(pChannels[p].Value, pChannels[p + 1].Value, pa);
					break;
				}
			}


			const nsTArray<nsAnimationKeyFrame::TChannel<nsQuaternion>>& rChannels = clipData.KeyFrames[j].RotationChannels;
			for (int r = 0; r < rChannels.GetCount() - 1; ++r)
			{
				if (state.Timestamp >= rChannels[r].Timestamp && state.Timestamp <= rChannels[r + 1].Timestamp)
				{
					const float ra = (state.Timestamp - rChannels[r].Timestamp) / (rChannels[r + 1].Timestamp - rChannels[r].Timestamp);
					bone.LocalTransform.Rotation = nsQuaternion::Slerp(rChannels[r].Value, rChannels[r + 1].Value, ra);
					break;
				}
			}


			const nsTArray<nsAnimationKeyFrame::TChannel<nsVector3>>& sChannels = clipData.KeyFrames[j].ScaleChannels;
			for (int s = 0; s < sChannels.GetCount() - 1; ++s)
			{
				if (state.Timestamp >= sChannels[s].Timestamp && state.Timestamp <= sChannels[s + 1].Timestamp)
				{
					const float sa = (state.Timestamp - sChannels[s].Timestamp) / (sChannels[s + 1].Timestamp - sChannels[s].Timestamp);
					bone.LocalTransform.Scale = nsVector3::Lerp(sChannels[s].Value, sChannels[s + 1].Value, sa);
					break;
				}
			}
		}


		for (int j = 0; j < boneCount; ++j)
		{
			nsAnimationSkeletonData::Bone& bone = animInstanceData.BoneTransforms[j];
			const nsAnimationSkeletonData::Bone* parentBone = bone.ParentId == -1 ? nullptr : &animInstanceData.BoneTransforms[bone.ParentId];

			if (parentBone)
			{
				NS_Validate(parentBone->bUpdated);
			}

			bone.PoseTransform = parentBone ? bone.LocalTransform.ToMatrix() * parentBone->PoseTransform : bone.LocalTransform.ToMatrix();

			const int boneTransformId = animInstanceData.BoneTransformIndex + j;
			InstanceBoneTransforms[boneTransformId] = bone.InverseBindPoseTransform * bone.PoseTransform;
			bone.bUpdated = true;
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


nsAnimationClipID nsAnimationManager::FindClip(const nsName& name) const
{
	for (auto it = ClipNames.CreateConstIterator(); it; ++it)
	{
		if ((*it) == name)
		{
			return it.GetIndex();
		}
	}

	return nsAnimationClipID::INVALID;
}


nsAnimationClipID nsAnimationManager::CreateClip(nsName name)
{
	NS_Validate_IsMainThread();

	if (FindClip(name) != nsAnimationClipID::INVALID)
	{
		NS_LogWarning(AnimationLog, "Animation sequence with name [%s] already exists!", *name);
	}

	const int nameId = ClipNames.Add();
	const int flagId = ClipFlags.Add();
	const int dataId = ClipDatas.Add();
	NS_Assert(nameId == flagId && flagId == dataId);

	ClipNames[nameId] = name;
	ClipFlags[flagId] = Flag_Allocated;

	nsAnimationClipData& data = ClipDatas[dataId];
	data.SkeletonName = nsName::NONE;
	data.FrameCount = 0;
	data.Duration = 0.0f;
	data.KeyFrames.Clear(true);

	return nameId;
}


nsAnimationClipID nsAnimationManager::CreateClip(nsName name, nsAnimationSkeletonID skeleton)
{
	NS_Assert(IsSkeletonValid(skeleton));

	const nsAnimationClipID clip = CreateClip(name);

	nsAnimationClipData& data = ClipDatas[clip.Id];
	data.SkeletonName = SkeletonNames[skeleton.Id];

	return clip;
}


nsAnimationClipID nsAnimationManager::CreateClip(nsName name, nsName skeletonName)
{
	const nsAnimationSkeletonID skeleton = FindSkeleton(skeletonName);

	if (skeleton == nsAnimationSkeletonID::INVALID)
	{
		NS_LogError(AnimationLog, "Fail to create clip [%s]. Skeleton name [%s] not found!", *name, *skeletonName);
		return nsAnimationClipID::INVALID;
	}

	return CreateClip(name, skeleton);
}


void nsAnimationManager::DestroyClip(nsAnimationClipID& clip)
{
	NS_Validate_IsMainThread(); 

	if (IsClipValid(clip))
	{
		NS_ValidateV(0, "Not implemented yet!");
	}

	clip = nsAnimationClipID::INVALID;
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
	const int stateId = InstancePlayStates.Add();
	NS_Assert(nameId == flagId && flagId == dataId && dataId == stateId);

	InstanceNames[nameId] = name;
	InstanceFlags[flagId] = Flag_Allocated;

	nsAnimationSkeletonData& skeletonData = SkeletonDatas[skeleton.Id];
	const int boneCount = skeletonData.BoneNames.GetCount();

	nsAnimationInstanceData& data = InstanceDatas[dataId];
	data.BoneNames = skeletonData.BoneNames;
	data.BoneTransforms = skeletonData.BoneDatas;
	data.Skeleton = skeleton;
	data.BoneTransformIndex = InstanceBoneTransforms.GetCount();

	nsAnimationPlayState& state = InstancePlayStates[stateId];
	state.Clip = nsAnimationClipID::INVALID;
	state.PlayRate = 0.0f;
	state.Timestamp = 0.0f;
	state.bLooping = false;

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


void nsAnimationManager::SetInstanceUpdatePose(nsAnimationInstanceID instance, bool bUpdatePose)
{
	NS_Assert(IsInstanceValid(instance));

	if (bUpdatePose)
	{
		InstanceFlags[instance.Id] |= Flag_Instance_UpdatePose;
	}
	else
	{
		InstanceFlags[instance.Id] &= ~Flag_Instance_UpdatePose;
	}
}


void nsAnimationManager::PlayAnimation(nsAnimationInstanceID instance, nsAnimationClipID clip, float playRate, bool bLoop)
{
	NS_Assert(IsInstanceValid(instance));
	NS_Assert(IsClipValid(clip));

	const nsAnimationInstanceData& instanceData = InstanceDatas[instance.Id];
	const nsAnimationSkeletonID skeleton = instanceData.Skeleton;
	NS_Assert(IsSkeletonValid(skeleton));
	const nsName skeletonName = SkeletonNames[skeleton.Id];

	const nsAnimationClipData& clipData = ClipDatas[clip.Id];

	if (skeletonName != clipData.SkeletonName)
	{
		NS_CONSOLE_Warning(AnimationLog, "Fail to play animation [%s] with animation instance [%s]. Skeleton is not compatible! [AnimationInstanceSkeleton: %s, AnimationClipSkeleton: %s]",
			*ClipNames[clip.Id],
			*InstanceNames[instance.Id],
			*skeletonName,
			*clipData.SkeletonName
		);

		return;
	}

	nsAnimationPlayState& state = InstancePlayStates[instance.Id];
	
	if (state.Clip == clip && state.PlayRate == playRate)
	{
		return;
	}

	state.Clip = clip;
	state.PlayRate = playRate;
	state.Timestamp = 0.0f;
	state.bLooping = bLoop;
}


void nsAnimationManager::StopAnimation(nsAnimationInstanceID instance)
{
	NS_Assert(IsInstanceValid(instance));

}


void nsAnimationManager::BlendAnimation(nsAnimationInstanceID instance, nsEAnimationTransitionMode transitionMode, float blendFactor, nsAnimationClipID clip, float playRate, bool bLoop)
{
	NS_Assert(IsInstanceValid(instance));

}



void nsAnimationManager::BeginFrame(int frameIndex)
{
	FrameIndex = frameIndex;

	Frame& frame = FrameDatas[FrameIndex];
}


void nsAnimationManager::UpdateRenderResources()
{
	Frame& frame = FrameDatas[FrameIndex];

	const uint64 storageBufferSize = sizeof(nsMatrix4) * InstanceBoneTransforms.GetCount();
	frame.SkeletonPoseTransformStorageBuffer->Resize(storageBufferSize);

	void* map = frame.SkeletonPoseTransformStorageBuffer->MapMemory();
	nsPlatform::Memory_Copy(map, InstanceBoneTransforms.GetData(), storageBufferSize);
	frame.SkeletonPoseTransformStorageBuffer->UnmapMemory();
}
