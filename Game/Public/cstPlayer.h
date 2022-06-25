#pragma once

#include "cstTypes.h"



enum class cstEPlayerControlState : uint8
{
	NONE = 0,
	FREE_CAMERA,
	CONTROLLING_CHARACTER
};


class cstPlayerController : public nsObject
{
	NS_DECLARE_OBJECT()

private:
	cstEPlayerControlState PendingControlState;
	cstEPlayerControlState CurrentControlState;
	nsVector3 MovementAxis;

	// Camera
	nsTransform CameraTransform;
	nsVector2 CameraRotation;
	float CameraRotationSpeed;

public:
	// Viewport
	nsViewport* Viewport;

	// Character
	cstCharacter* Character;


public:
	cstPlayerController();
	void OnMouseMove(const nsMouseMoveEventArgs& args);
	void OnMouseButton(const nsMouseButtonEventArgs& args);
	void OnKeyboardButton(const nsKeyboardButtonEventArgs& args);
	void TickUpdate(float deltaTime);
	void PhysicsTickUpdate(float fixedDeltaTime);


	NS_INLINE void SetControlState(cstEPlayerControlState newControlState)
	{
		if (CurrentControlState == newControlState)
		{
			return;
		}

		PendingControlState = newControlState;
	}

};
