#pragma once

#include "cstEditorTypes.h"



enum class cstEEditorGizmoAxis : uint8
{
	X = 0,
	Y,
	Z,
	XY,
	XZ,
	YZ
};



class cstEditorGizmoTranslate
{
public:
	// Axis types
	cstEEditorGizmoAxis Axis;

	// Line length
	float LineLength;

	// Arrow width
	float ArrowWidth;

	// Arrow tip height
	float ArrowTipHeight;

	// Default color
	nsColor Color;

private:
	//nsVector3 TranslationOffset;
	nsVector3 LastIntersectionPoint;
	bool bUpdating;


public:
	cstEditorGizmoTranslate();

private:
	nsVector3 GetIntersectionPoint(nsViewport* viewport, const nsVector2& mousePosition, const nsTransform& transform, float scale, bool bIsLocal) const;

public:
	bool UpdateTranslation(nsViewport* viewport, const nsVector2& mousePosition, nsTransform& transform, float snapValue, float scale, bool bIsLocal);
	void Render(nsRenderContextWorld& context, nsViewport* viewport, const nsTransform& transform, float scale, bool bIsLocal, bool bIsSelected, bool bDrawDebug);


	NS_INLINE void BeginTranslate(nsViewport* viewport, const nsVector2& mousePosition, const nsTransform& transform, float scale, bool bIsLocal)
	{
		if (!bUpdating)
		{
			LastIntersectionPoint = GetIntersectionPoint(viewport, mousePosition, transform, scale, bIsLocal);
			//TranslationOffset = LastIntersectionPoint - transform.Position;
			bUpdating = true;
		}
	}


	NS_INLINE void EndTranslate()
	{
		//TranslationOffset = nsVector3::ZERO;
		LastIntersectionPoint = nsVector3::ZERO;
		bUpdating = false;
	}


	NS_NODISCARD_INLINE nsLine GetAxisLine(const nsTransform& transform, float scale, bool bIsLocal) const
	{
		nsVector3 directionVector;

		if (Axis == cstEEditorGizmoAxis::X)
		{
			directionVector = bIsLocal ? transform.GetAxisRight() : nsVector3::RIGHT;
		}
		else if (Axis == cstEEditorGizmoAxis::Y)
		{
			directionVector = bIsLocal ? transform.GetAxisUp() : nsVector3::UP;
		}
		else if (Axis == cstEEditorGizmoAxis::Z)
		{
			directionVector = bIsLocal ? transform.GetAxisForward() : nsVector3::FORWARD;
		}

		directionVector.Normalize();

		nsLine axisLine;
		axisLine.A = transform.Position + directionVector * 16.0f * scale;
		axisLine.B = axisLine.A + directionVector * (LineLength + ArrowTipHeight * 0.5f) * scale;

		return axisLine;
	}

};




class cstEditorGizmoRotate
{
public:
	// Axis types
	cstEEditorGizmoAxis Axis;

	// Circle radius
	float CircleRadius;

	// Default color
	nsColor Color;

private:
	nsVector3 RotationAxis;
	nsVector3 RotationValue;
	nsQuaternion StartRotation;
	nsVector3 StartIntersectionDirection;
	nsVector3 CurrentIntersectionDirection;
	nsVector3 LastIntersectionDirection;
	bool bUpdating;


public:
	cstEditorGizmoRotate();

private:
	nsVector3 GetIntersectionDirection(nsViewport* viewport, const nsVector2& mousePosition, const nsTransform& transform, float scale, bool bIsLocal) const;

public:
	bool UpdateRotation(nsViewport* viewport, const nsVector2& mousePosition, nsTransform& transform, float snapValue, float scale, bool bIsLocal);
	void Render(nsRenderContextWorld& context, nsViewport* viewport, const nsTransform& transform, float scale, bool bIsLocal, bool bIsSelected, bool bDrawDebug);


	NS_INLINE void BeginRotate(nsViewport* viewport, const nsVector2& mousePosition, const nsTransform& transform, float scale, bool bIsLocal)
	{
		if (!bUpdating)
		{
			if (Axis == cstEEditorGizmoAxis::X)
			{
				RotationAxis = bIsLocal ? nsVector3::RIGHT : transform.GetAxisRight();
			}
			else if (Axis == cstEEditorGizmoAxis::Y)
			{
				RotationAxis = bIsLocal ? nsVector3::UP : transform.GetAxisUp();
			}
			else if (Axis == cstEEditorGizmoAxis::Z)
			{
				RotationAxis = bIsLocal ? nsVector3::FORWARD : transform.GetAxisForward();
			}

			RotationValue = transform.Rotation.GetPitchYawRoll();
			NS_CONSOLE_Debug(EditorLog, "BeginRotate: %.3f, %.3f, %.3f", RotationValue.X, RotationValue.Y, RotationValue.Z);
			StartRotation = transform.Rotation;
			StartIntersectionDirection = GetIntersectionDirection(viewport, mousePosition, transform, scale, bIsLocal);
			LastIntersectionDirection = StartIntersectionDirection;
			bUpdating = true;
		}
	}


	NS_INLINE void EndRotate()
	{
		RotationAxis = nsVector3::ZERO;
		StartRotation = nsQuaternion::IDENTITY;
		StartIntersectionDirection = nsVector3::ZERO;
		CurrentIntersectionDirection = nsVector3::ZERO;
		LastIntersectionDirection = nsVector3::ZERO;
		bUpdating = false;
	}


	NS_NODISCARD_INLINE nsLine GetAxisLine(const nsTransform& transform, bool bIsLocal) const
	{
		nsVector3 directionVector;

		if (Axis == cstEEditorGizmoAxis::X)
		{
			directionVector = bIsLocal ? transform.GetAxisRight() : nsVector3::RIGHT;
		}
		else if (Axis == cstEEditorGizmoAxis::Y)
		{
			directionVector = bIsLocal ? transform.GetAxisUp() : nsVector3::UP;
		}
		else if (Axis == cstEEditorGizmoAxis::Z)
		{
			directionVector = bIsLocal ? transform.GetAxisForward() : nsVector3::FORWARD;
		}

		directionVector.Normalize();

		nsLine axisLine;
		axisLine.A = transform.Position;
		axisLine.B = axisLine.A + directionVector * CircleRadius;

		return axisLine;
	}

};



class cstEditorGizmoScale
{
public:
	// Axis types
	cstEEditorGizmoAxis Axis;

	// Line length
	float LineLength;

	// Box width
	float BoxWidth;

	// Default color
	nsColor Color;

private:
	nsVector3 ScaleOffset;
	nsVector3 StartInteractionPoint;
	nsVector3 LastIntersectionPoint;
	nsVector3 CurrentIntersectionPoint;
	bool bUpdating;


public:
	cstEditorGizmoScale();

private:
	nsVector3 GetIntersectionPoint(nsViewport* viewport, const nsVector2& mousePosition, const nsTransform& transform, float scale) const;

public:
	bool UpdateScale(nsViewport* viewport, const nsVector2& mousePosition, nsTransform& transform, float snapValue, float scale);
	void Render(nsRenderContextWorld& context, nsViewport* viewport, const nsTransform& transform, float scale, bool bIsSelected, bool bDrawDebug);


	NS_INLINE void BeginScale(nsViewport* viewport, const nsVector2& mousePosition, const nsTransform& transform, float scale)
	{
		if (!bUpdating)
		{
			StartInteractionPoint = GetIntersectionPoint(viewport, mousePosition, transform, scale);
			LastIntersectionPoint = StartInteractionPoint;
			bUpdating = true;
		}
	}


	NS_INLINE void EndScale()
	{
		ScaleOffset = nsVector3::ZERO;
		StartInteractionPoint = nsVector3::ZERO;
		LastIntersectionPoint = nsVector3::ZERO;
		CurrentIntersectionPoint = nsVector3::ZERO;
		bUpdating = false;
	}


	NS_NODISCARD_INLINE nsLine GetAxisLine(const nsTransform& transform, float scale) const
	{
		nsVector3 directionVector;

		if (Axis == cstEEditorGizmoAxis::X)
		{
			directionVector = transform.GetAxisRight();
		}
		else if (Axis == cstEEditorGizmoAxis::Y)
		{
			directionVector = transform.GetAxisUp();
		}
		else if (Axis == cstEEditorGizmoAxis::Z)
		{
			directionVector = transform.GetAxisForward();
		}

		directionVector.Normalize();

		nsLine axisLine;
		axisLine.A = transform.Position + directionVector * 16.0f * scale;
		axisLine.B = axisLine.A + directionVector * (LineLength + BoxWidth * 0.5f) * scale;

		return axisLine;
	}

};



enum class cstEEditorGizmoTransformMode : uint8
{
	TRANSLATE = 0,
	ROTATE,
	SCALE
};



class cstEditorGizmoTransform
{
private:
	cstEditorGizmoTranslate Translates[3];
	cstEditorGizmoRotate Rotates[3];
	cstEditorGizmoScale Scales[3];
	int SelectedTranslate;
	int SelectedRotate;
	int SelectedScale;
	bool bUpdating;

public:
	cstEEditorGizmoTransformMode Mode;
	float SnapTranslationValue;
	float SnapRotationValue;
	float SnapScaleValue;


public:
	cstEditorGizmoTransform();
	void BeginTransform(nsViewport* viewport, const nsVector2& mousePosition, const nsTransform& transform, bool bIsLocal);
	void EndTransform();
	bool UpdateTransform(nsViewport* viewport, const nsVector2& mousePosition, cstEEditorViewMode viewMode, nsTransform& outTransform, bool bIsLocal);
	void Render(nsRenderContextWorld& context, nsViewport* viewport, const nsTransform& transform, bool bIsLocal, bool bDrawDebug);


	NS_INLINE void RemoveSelected()
	{
		SelectedTranslate = -1;
		SelectedRotate = -1;
		SelectedScale = -1;
	}


	NS_NODISCARD_INLINE bool IsUpdating() const
	{
		return bUpdating;
	}

};
