#include "nsAnimationManager.h"
#include "nsConsole.h"


NS_ENGINE_DEFINE_HANDLE(nsAnimationSkeletonID);
NS_ENGINE_DEFINE_HANDLE(nsAnimationInstanceID);
NS_ENGINE_DEFINE_HANDLE(nsAnimationClipID);


static nsLogCategory AnimationLog(TEXT("nsAnimationLog"), nsELogVerbosity::LV_DEBUG);



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

	NS_CONSOLE_Log(AnimationLog, TEXT("Initialize animation manager"));

	for (int i = 0; i < NS_ENGINE_FRAME_BUFFERING; ++i)
	{
		Frame& frame = FrameDatas[i];
		frame.SkeletonPoseTransformStorageBuffer = nsVulkan::CreateStorageBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, NS_MEMORY_SIZE_MiB(1), nsName::Format("skel_pose_transform_ssbo_%i", i));
	}

	bInitialized = true;
}


void nsAnimationManager::UpdateAnimationPoses(float deltaTime)
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

			const nsTArray<nsAnimationKeyFrame::TChannel<nsVector3>>& positionChannels = clipData.KeyFrames[j].PositionChannels;
			for (int p = 0; p < positionChannels.GetCount() - 1; ++p)
			{
				if (state.Timestamp >= positionChannels[p].Timestamp && state.Timestamp <= positionChannels[p + 1].Timestamp)
				{
					bone.LocalTransform.Position = nsVector3::Lerp(positionChannels[p].Value, positionChannels[p + 1].Value, (state.Timestamp - positionChannels[p].Timestamp) / (positionChannels[p + 1].Timestamp - positionChannels[p].Timestamp));
					break;
				}
			}

			const nsTArray<nsAnimationKeyFrame::TChannel<nsQuaternion>>& rotationChannels = clipData.KeyFrames[j].RotationChannels;
			for (int r = 0; r < rotationChannels.GetCount() - 1; ++r)
			{
				if (state.Timestamp >= rotationChannels[r].Timestamp && state.Timestamp <= rotationChannels[r + 1].Timestamp)
				{
					bone.LocalTransform.Rotation = nsQuaternion::Slerp(rotationChannels[r].Value, rotationChannels[r + 1].Value, (state.Timestamp - rotationChannels[r].Timestamp) / (rotationChannels[r + 1].Timestamp - rotationChannels[r].Timestamp));
					break;
				}
			}

			const nsTArray<nsAnimationKeyFrame::TChannel<nsVector3>>& scaleChannels = clipData.KeyFrames[j].ScaleChannels;
			for (int s = 0; s < scaleChannels.GetCount() - 1; ++s)
			{
				if (state.Timestamp >= scaleChannels[s].Timestamp && state.Timestamp <= scaleChannels[s + 1].Timestamp)
				{
					bone.LocalTransform.Scale = nsVector3::Lerp(scaleChannels[s].Value, scaleChannels[s + 1].Value, (state.Timestamp - scaleChannels[s].Timestamp) / (scaleChannels[s + 1].Timestamp - scaleChannels[s].Timestamp));
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
		NS_LogWarning(AnimationLog, TEXT("Skeleton with name [%s] already exists!"), *name.ToString());
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
		NS_ValidateV(0, TEXT("Not implemented yet!"));
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
		NS_LogWarning(AnimationLog, TEXT("Animation sequence with name [%s] already exists!"), *name.ToString());
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
		NS_LogError(AnimationLog, TEXT("Fail to create clip [%s]. Skeleton name [%s] not found!"), *name.ToString(), *skeletonName.ToString());
		return nsAnimationClipID::INVALID;
	}

	return CreateClip(name, skeleton);
}


void nsAnimationManager::DestroyClip(nsAnimationClipID& clip)
{
	NS_Validate_IsMainThread(); 

	if (IsClipValid(clip))
	{
		NS_ValidateV(0, TEXT("Not implemented yet!"));
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
		NS_LogWarning(AnimationLog, TEXT("Animation skeleton with name [%s] already exists!"), *name.ToString());
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

	NS_LogDebug(AnimationLog, TEXT("Create animation instance [%s]"), *name.ToString());

	return nameId;
}


void nsAnimationManager::DestroyInstance(nsAnimationInstanceID& instance)
{
	NS_Validate_IsMainThread();

	if (IsInstanceValid(instance))
	{
		NS_ValidateV(0, TEXT("Not implemented yet!"));
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
		NS_CONSOLE_Warning(AnimationLog, TEXT("Play animation [%s] with animation instance [%s]. Skeleton is not compatible! [AnimationInstanceSkeleton: %s, AnimationClipSkeleton: %s]"),
			*ClipNames[clip.Id].ToString(),
			*InstanceNames[instance.Id].ToString(),
			*skeletonName.ToString(),
			*clipData.SkeletonName.ToString()
		);
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

	nsAnimationPlayState& state = InstancePlayStates[instance.Id];
	state.Clip = nsAnimationClipID::INVALID;
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



#ifdef NS_ENGINE_DEBUG_DRAW
#include "nsRenderer.h"


void nsAnimationManager::SetInstanceDebugDraw(nsAnimationInstanceID instance, bool bDebugDraw, nsTransform rootWorldTransform)
{
	NS_Assert(IsInstanceValid(instance));

	if (bDebugDraw)
	{
		InstanceDebugDraws[instance.Id] = rootWorldTransform;
	}
	else
	{
		InstanceDebugDraws.Remove(instance.Id);
	}
}


void nsAnimationManager::DebugDraw(nsRenderer* renderer)
{
	nsTArray<nsMatrix4> cachedBoneWorldMatrices;

	for (int i = 0; i < InstanceDebugDraws.GetCount(); ++i)
	{
		const int id = InstanceDebugDraws.GetKeyByIndex(i);
		const nsMatrix4 rootWorldTransformMatrix = InstanceDebugDraws.GetValueByIndex(i).ToMatrixNoScale();
		const nsTArrayInline<nsAnimationSkeletonData::Bone, NS_ENGINE_ANIMATION_SKELETON_MAX_BONE>& instanceBones = InstanceDatas[id].BoneTransforms;
		cachedBoneWorldMatrices.Clear();
		cachedBoneWorldMatrices.ResizeConstructs(instanceBones.GetCount(), nsMatrix4::IDENTITY);

		for (int j = 0; j < instanceBones.GetCount(); ++j)
		{
			const nsAnimationSkeletonData::Bone& bone = instanceBones[j];

			if (bone.ParentId == -1)
			{
				cachedBoneWorldMatrices[j] = bone.LocalTransform.ToMatrix() * rootWorldTransformMatrix;
			}
			else
			{
				cachedBoneWorldMatrices[j] = bone.LocalTransform.ToMatrix() * cachedBoneWorldMatrices[bone.ParentId];
			}
		}

		for (int j = 0; j < instanceBones.GetCount(); ++j)
		{
			const nsAnimationSkeletonData::Bone& bone = instanceBones[j];
			const nsVector3 boneWorldPosition = cachedBoneWorldMatrices[j].GetPosition();
			renderer->DebugDrawMeshAABB(boneWorldPosition - 0.5f, boneWorldPosition + 0.5f, nsColor::GRAY, true);

			if (bone.ParentId != -1)
			{
				const nsVector3 parentBoneWorldPosition = cachedBoneWorldMatrices[bone.ParentId].GetPosition();
				renderer->DebugDrawLine(boneWorldPosition, parentBoneWorldPosition, nsColor::WHITE, 100, true);
			}
		}
	}
}

#endif // NS_ENGINE_DEBUG_DRAW