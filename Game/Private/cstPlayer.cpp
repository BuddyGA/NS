#include "cstPlayer.h"
#include "cstCharacter.h"



NS_CLASS_BEGIN(cstPlayerController, nsObject)
NS_CLASS_END(cstPlayerController)

cstPlayerController::cstPlayerController()
{
	PendingControlState = cstEPlayerControlState::NONE;
	CurrentControlState = cstEPlayerControlState::NONE;
	MovementAxis = nsVector3::ZERO;

	CameraTransform = nsTransform();
	CameraRotation = nsVector2::ZERO;
	CameraRotationSpeed = 90.0f;

	Viewport = nullptr;
	Character = nullptr;
}


void cstPlayerController::OnMouseMove(const nsMouseMoveEventArgs& args)
{
	CameraRotation.X += 0.25f * args.DeltaPosition.Y;
	CameraRotation.Y += 0.25f * args.DeltaPosition.X;
}


void cstPlayerController::OnMouseButton(const nsMouseButtonEventArgs& args)
{

}


void cstPlayerController::OnKeyboardButton(const nsKeyboardButtonEventArgs& args)
{
	if (args.ButtonState == nsEButtonState::PRESSED)
	{
		if (args.Key == nsEInputKey::KEYBOARD_A)
		{
			MovementAxis.X = -1.0f;
		}

		if (args.Key == nsEInputKey::KEYBOARD_D)
		{
			MovementAxis.X = 1.0f;
		}

		if (args.Key == nsEInputKey::KEYBOARD_S)
		{
			MovementAxis.Z = -1.0f;
		}

		if (args.Key == nsEInputKey::KEYBOARD_W)
		{
			MovementAxis.Z = 1.0f;
		}
	}
	else if (args.ButtonState == nsEButtonState::RELEASED)
	{
		if (args.Key == nsEInputKey::KEYBOARD_A || args.Key == nsEInputKey::KEYBOARD_D)
		{
			MovementAxis.X = 0.0f;
		}
		else if (args.Key == nsEInputKey::KEYBOARD_W || args.Key == nsEInputKey::KEYBOARD_S)
		{
			MovementAxis.Z = 0.0f;
		}
	}
}


void cstPlayerController::TickUpdate(float deltaTime)
{
	if (CurrentControlState != PendingControlState)
	{
		if (CurrentControlState == cstEPlayerControlState::FREE_CAMERA)
		{
			// TODO: End free camera state
		}
		else if (CurrentControlState == cstEPlayerControlState::CONTROLLING_CHARACTER)
		{
			// TODO: End controlling character state
		}
		
		if (PendingControlState == cstEPlayerControlState::FREE_CAMERA)
		{
			// TODO: Begin free camera state
		}
		else if (PendingControlState == cstEPlayerControlState::CONTROLLING_CHARACTER)
		{

		}

		CurrentControlState = PendingControlState;
	}

	
	if (CurrentControlState == cstEPlayerControlState::FREE_CAMERA)
	{
		// TODO: Update free camera
	}
	else if (CurrentControlState == cstEPlayerControlState::CONTROLLING_CHARACTER && Character)
	{
		// Update camera
		const nsTransform characterTransform = Character->GetWorldTransform();
		CameraTransform.Rotation = nsQuaternion::Slerp(CameraTransform.Rotation, nsQuaternion::FromRotation(CameraRotation.X, CameraRotation.Y, 0.0f), deltaTime);

		const nsVector3 cameraPivotPosition = characterTransform.Position + nsVector3(0.0f, 50.0f, 0.0f);
		CameraTransform.Position = cameraPivotPosition - CameraTransform.GetAxisForward() * 300.0f;

		if (Viewport)
		{
			Viewport->SetViewTransform(CameraTransform);
		}
	}
}


void cstPlayerController::PhysicsTickUpdate(float fixedDeltaTime)
{
	if (CurrentControlState != cstEPlayerControlState::CONTROLLING_CHARACTER || Character == nullptr)
	{
		return;
	}

	nsVector3 moveDirection;
	moveDirection += CameraTransform.GetAxisRight() * MovementAxis.X;
	moveDirection += CameraTransform.GetAxisForward() * MovementAxis.Z;
	moveDirection.Y = 0.0f;
	moveDirection.Normalize();

	Character->Move(fixedDeltaTime, moveDirection);
}
