#pragma once

#include "cstEditorTypes.h"



class cstEditorActorInspector
{
private:
	nsActor* InspectActor;
	nsVector3 CachePosition;
	nsVector3 CacheRotation;
	nsVector3 CacheScale;

	nsGUIWindow Window;
	nsGUIInputVector3 InputPosition;
	nsGUIInputVector3 InputRotation;
	nsGUIInputVector3 InputScale;

public:
	bool bIsLocalCoordSpace;


public:
	cstEditorActorInspector();
	void DrawGUI(nsGUIContext& context);


	NS_INLINE void SetInspectActor(nsActor* actor)
	{
		InspectActor = actor;
		UpdateCacheTransform();
	}


	NS_INLINE void UpdateCacheTransform()
	{
		if (InspectActor)
		{
			const nsTransform transform = bIsLocalCoordSpace ? InspectActor->GetLocalTransform() : InspectActor->GetWorldTransform();
			CachePosition = transform.Position;
			CacheRotation = transform.Rotation.GetPitchYawRoll();
			CacheScale = transform.Scale;
		}
	}

};
