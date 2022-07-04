#include "cstEditorGizmo.h"
#include "nsRenderer.h"


#define CST_EDITOR_GIZMO_COLOR_AXIS_X		nsColor(200, 50, 50, 255);
#define CST_EDITOR_GIZMO_COLOR_AXIS_Y		nsColor(50, 200, 50, 255);
#define CST_EDITOR_GIZMO_COLOR_AXIS_Z		nsColor(50, 50, 200, 255);
#define CST_EDITOR_GIZMO_COLOR_SELECTED		nsColor(200, 150, 100, 255)



// ================================================================================================================================================================================== //
// EDITOR - GIZMO TRANSLATE
// ================================================================================================================================================================================== //
cstEditorGizmoTranslate::cstEditorGizmoTranslate()
{
	Axis = cstEEditorGizmoAxis::X;
	LineLength = 64.0f;
	ArrowWidth = 6.0f;
	ArrowTipHeight = 24.0f;
	Color = nsColor::WHITE;
	//TranslationOffset = nsVector3::ZERO;
	LastIntersectionPoint = nsVector3::ZERO;
	bUpdating = false;
}


nsVector3 cstEditorGizmoTranslate::GetIntersectionPoint(nsViewport* viewport, const nsVector2& mousePosition, const nsTransform& transform, float scale, bool bIsLocal) const
{
	NS_Assert(viewport);

	nsVector3 intersectionPoint = LastIntersectionPoint;

	nsVector3 rayOrigin;
	nsVector3 rayDirection;
	viewport->ProjectToWorld(mousePosition, rayOrigin, rayDirection);
	//NS_CONSOLE_Debug(EditorLog, "RayOrigin: %f, %f, %f", rayOrigin.X, rayOrigin.Y, rayOrigin.Z);

	const nsLine axisLine = GetAxisLine(transform, scale, bIsLocal);
	const nsPlane viewPlane(viewport->GetViewTransform().GetAxisForward(), transform.Position);
	nsVector3 planeIntersectionPoint;

	if (viewPlane.TestIntersectionRay(planeIntersectionPoint, rayOrigin, rayDirection))
	{
		intersectionPoint = axisLine.ProjectPoint(planeIntersectionPoint);
	}

	return intersectionPoint;
}


bool cstEditorGizmoTranslate::UpdateTranslation(nsViewport* viewport, const nsVector2& mousePosition, nsTransform& transform, float snapValue, float scale, bool bIsLocal)
{
	if (!bUpdating)
	{
		return false;
	}

	const nsVector3 currentIntersectionPoint = GetIntersectionPoint(viewport, mousePosition, transform, scale, bIsLocal);
	nsVector3 deltaPosition = currentIntersectionPoint - LastIntersectionPoint;
	bool bUpdatePosition = true;

	if (snapValue > 0.0f)
	{
		if (deltaPosition.GetMagnitude() >= snapValue)
		{
			deltaPosition.Normalize();
			deltaPosition *= snapValue;
		}
		else
		{
			bUpdatePosition = false;
		}
	}

	if (bUpdatePosition)
	{
		transform.Position += deltaPosition;
		LastIntersectionPoint = currentIntersectionPoint;
	}

	return true;
}


void cstEditorGizmoTranslate::Render(nsRenderer* renderer, nsViewport* viewport, const nsTransform& transform, float scale, bool bIsLocal, bool bIsSelected, bool bDrawDebug)
{
	const nsColor useColor = bIsSelected ? CST_EDITOR_GIZMO_COLOR_SELECTED : Color;
	const nsLine axisLine = GetAxisLine(transform, scale, bIsLocal);
	const nsVector3 directionAxis = axisLine.GetDirection();

	renderer->DebugDrawLine(axisLine.A, axisLine.B, useColor, 100, true);
	renderer->DebugDrawMeshPrism(axisLine.B, nsQuaternion::FromVectors(nsVector3::UP, directionAxis), ArrowWidth * scale, ArrowTipHeight * scale, useColor, true);

	// Draw debug
	if (bDrawDebug)
	{
		/*
		if (bUpdating)
		{
			const nsPlane viewPlane(viewport->GetViewTransform().GetAxisForward(), transform.Position);
			context.AddPrimitiveMesh_Plane(transform.Position, -viewPlane.Normal, 256.0f, nsColor::WHITE);
			context.AddPrimitiveMesh_AABB(LastIntersectionPoint - 8.0f, LastIntersectionPoint + 8.0f, nsColor::RED);
		}
		*/
	}
}




// ================================================================================================================================================================================== //
// EDITOR - GIZMO ROTATE
// ================================================================================================================================================================================== //
cstEditorGizmoRotate::cstEditorGizmoRotate()
{
	Axis = cstEEditorGizmoAxis::X;
	CircleRadius = 75.0f;
	Color = nsColor::WHITE;
	StartIntersectionDirection = nsVector3::ZERO;
	CurrentIntersectionDirection = nsVector3::ZERO;
	bUpdating = false;
}


nsVector3 cstEditorGizmoRotate::GetIntersectionDirection(nsViewport* viewport, const nsVector2& mousePosition, const nsTransform& transform, float scale, bool bIsLocal) const
{
	nsVector3 intersectionDirection = StartIntersectionDirection;

	nsVector3 rayOrigin;
	nsVector3 rayDirection;
	viewport->ProjectToWorld(mousePosition, rayOrigin, rayDirection);

	const nsPlane axisPlane(GetAxisLine(transform, bIsLocal).GetDirection(), transform.Position);
	nsVector3 planeIntersectionPoint;

	if (axisPlane.TestIntersectionRay(planeIntersectionPoint, rayOrigin, rayDirection))
	{
		// Get point on circle circumference
		const nsVector3 pointOnCircle = transform.Position + (planeIntersectionPoint - transform.Position).GetNormalized() * CircleRadius * scale;
		intersectionDirection = (pointOnCircle - transform.Position).GetNormalized();
	}

	return intersectionDirection;
}


bool cstEditorGizmoRotate::UpdateRotation(nsViewport* viewport, const nsVector2& mousePosition, nsTransform& transform, float snapValue, float scale, bool bIsLocal)
{
	if (!bUpdating)
	{
		return false;
	}

	const nsVector3 axisDirection = GetAxisLine(transform, bIsLocal).GetDirection();
	CurrentIntersectionDirection = GetIntersectionDirection(viewport, mousePosition, transform, scale, bIsLocal);
	float radian = nsVector3::AngleBetween(StartIntersectionDirection, CurrentIntersectionDirection);

	if (nsMath::IsZero(radian))
	{
		return false;
	}

	NS_Assert(!nsMath::IsNaN(radian));

	bool bUpdateRotation = true;

	if (snapValue > 0.0f)
	{
		const float degreeSnap = nsMath::RadToDeg(nsVector3::AngleBetween(LastIntersectionDirection, CurrentIntersectionDirection));

		if (nsMath::Abs(degreeSnap) >= snapValue)
		{
			const float radianSnapValue = nsMath::DegToRad(snapValue);
			radian = nsMath::Floor(radian / radianSnapValue) * radianSnapValue;
			LastIntersectionDirection = CurrentIntersectionDirection;
		}
		else
		{
			bUpdateRotation = false;
		}
	}

	if (bUpdateRotation)
	{
		const nsVector3 cross = nsVector3::CrossProduct(StartIntersectionDirection, CurrentIntersectionDirection);
		const float dot = nsVector3::DotProduct(cross.GetNormalized(), axisDirection);
		const float sign = dot > 0.0f ? 1.0f : -1.0f;

		/*
		NS_CONSOLE_Debug(EditorLog, "UpdateRotation: [Axis=%.3f,%.3f,%.3f], [radian=%.3f, dot=%.3f, sign=%.3f, Local=%d]",
			RotationAxis.X, RotationAxis.Y, RotationAxis.Z,
			radian, dot, sign, bIsLocal
		);
		*/

		const nsQuaternion deltaRotation(RotationAxis, radian * sign);
		transform.Rotation = StartRotation * deltaRotation;
	}

	return true;
}


void cstEditorGizmoRotate::Render(nsRenderer* renderer, nsViewport* viewport, const nsTransform& transform, float scale, bool bIsLocal, bool bIsSelected, bool bDrawDebug)
{
	const nsColor useColor = bIsSelected ? CST_EDITOR_GIZMO_COLOR_SELECTED : Color;
	const nsLine axisLine = GetAxisLine(transform, bIsLocal);
	const nsVector3 directionAxis = axisLine.GetDirection();

	renderer->DebugDrawLineCircleAroundAxis(transform.Position, directionAxis, CircleRadius * scale, NS_MATH_PI, useColor, 100, true);
	renderer->DebugDrawLine(transform.Position, transform.Position + directionAxis * CircleRadius * scale, useColor, 100, true);

	if (bUpdating)
	{
		const nsVector3 startIntersectionPoint = transform.Position + StartIntersectionDirection * CircleRadius * scale;
		renderer->DebugDrawMeshAABB(startIntersectionPoint - 2.0f * scale, startIntersectionPoint + 2.0f * scale, useColor, true);

		const nsVector3 currentIntersectionPoint = transform.Position + CurrentIntersectionDirection * CircleRadius * scale;
		renderer->DebugDrawMeshAABB(currentIntersectionPoint - 2.0f * scale, currentIntersectionPoint + 2.0f * scale, Color, true);
	}


	// Draw debug
	if (bDrawDebug)
	{
		//context.AddPrimitiveLine(axisLine.A, axisLine.B, useColor);
		/*
		const nsPlane viewPlane(axisLine.GetDirection(), transform.Position);
		context.AddPrimitiveMesh_Plane(transform.Position, viewPlane.Normal, 128.0f, useColor);

		nsVector3 rayOrigin, rayDirection;
		viewport->ProjectToWorld(mousePosition, rayOrigin, rayDirection);

		const nsPlane axisPlane(axisLine.GetDirection(), transform.Position);
		nsVector3 pointOnPlane;

		if (axisPlane.TestIntersectionRay(pointOnPlane, rayOrigin, rayDirection))
		{
			nsVector3 pointOnCircle = transform.Position + (pointOnPlane - transform.Position).GetNormalized() * CircleRadius * scale;
			const float dist = (pointOnPlane - pointOnCircle).GetMagnitude();

			context.AddPrimitiveMesh_AABB(pointOnPlane - 2.0f, pointOnPlane + 2.0f, useColor);
			context.AddPrimitiveMesh_AABB(pointOnCircle - 2.0f, pointOnCircle + 2.0f, nsColor::CYAN);
		}
		*/
	}
}




// ================================================================================================================================================================================== //
// EDITOR - GIZMO SCALE
// ================================================================================================================================================================================== //
cstEditorGizmoScale::cstEditorGizmoScale()
{
	Axis = cstEEditorGizmoAxis::X;
	LineLength = 64.0f;
	BoxWidth = 8.0f;
	ScaleOffset = nsVector3::ZERO;
	StartInteractionPoint = nsVector3::ZERO;
	LastIntersectionPoint = nsVector3::ZERO;
	CurrentIntersectionPoint = nsVector3::ZERO;
	bUpdating = false;
}


nsVector3 cstEditorGizmoScale::GetIntersectionPoint(nsViewport* viewport, const nsVector2& mousePosition, const nsTransform& transform, float scale) const
{
	nsVector3 intersectionPoint = LastIntersectionPoint;

	nsVector3 rayOrigin;
	nsVector3 rayDirection;
	viewport->ProjectToWorld(mousePosition, rayOrigin, rayDirection);
	//NS_CONSOLE_Debug(EditorLog, "RayOrigin: %f, %f, %f", rayOrigin.X, rayOrigin.Y, rayOrigin.Z);

	const nsLine axisLine = GetAxisLine(transform, scale);
	const nsPlane viewPlane(viewport->GetViewTransform().GetAxisForward(), transform.Position);
	nsVector3 planeIntersectionPoint;

	if (viewPlane.TestIntersectionRay(planeIntersectionPoint, rayOrigin, rayDirection))
	{
		intersectionPoint = axisLine.ProjectPoint(planeIntersectionPoint);
	}

	return intersectionPoint;
}


bool cstEditorGizmoScale::UpdateScale(nsViewport* viewport, const nsVector2& mousePosition, nsTransform& transform, float snapValue, float scale)
{
	if (!bUpdating)
	{
		return false;
	}

	const nsLine axisLine = GetAxisLine(transform, scale);
	CurrentIntersectionPoint = GetIntersectionPoint(viewport, mousePosition, transform, scale);
	const nsVector3 deltaIntersectionPoint = LastIntersectionPoint - CurrentIntersectionPoint;
	LastIntersectionPoint = CurrentIntersectionPoint;

	if (deltaIntersectionPoint.IsZero() || deltaIntersectionPoint.GetMagnitude() < snapValue)
	{
		return false;
	}

	const float sign = nsVector3::DotProduct(deltaIntersectionPoint.GetNormalized(), axisLine.GetDirection()) < 0.0f ? 1.0f : -1.0f;
	//NS_CONSOLE_Debug(EditorLog, "Sign: %f, Magnitude: %.3f", sign, deltaIntersectionPoint.GetMagnitude());

	nsVector3 scaleAxis;

	if (Axis == cstEEditorGizmoAxis::X)
	{
		scaleAxis = nsVector3::RIGHT;
	}
	else if (Axis == cstEEditorGizmoAxis::Y)
	{
		scaleAxis = nsVector3::UP;
	}
	else if (Axis == cstEEditorGizmoAxis::Z)
	{
		scaleAxis = nsVector3::FORWARD;
	}

	//const float dot = nsVector3::DotProduct(StartInteractionPoint, CurrentIntersectionPoint) / StartInteractionPoint.GetMagnitudeSqr();
	transform.Scale += scaleAxis * sign * snapValue;
	transform.Scale = nsMath::ClampVector3(transform.Scale, 0.01f, 1000.0f);

	return true;
}


void cstEditorGizmoScale::Render(nsRenderer* renderer, nsViewport* viewport, const nsTransform& transform, float scale, bool bIsSelected, bool bDrawDebug)
{
	const nsColor useColor = bIsSelected ? CST_EDITOR_GIZMO_COLOR_SELECTED : Color;
	const nsLine axisLine = GetAxisLine(transform, scale);
	const nsVector3 directionAxis = axisLine.GetDirection();

	renderer->DebugDrawLine(axisLine.A, axisLine.B, useColor, 100, true);
	renderer->DebugDrawMeshAABB(axisLine.B - BoxWidth * 0.5f * scale, axisLine.B + BoxWidth * 0.5f * scale, useColor, true);

	// Draw debug
	if (bDrawDebug)
	{
		/*
		if (bUpdating)
		{
			const nsPlane viewPlane(viewport->GetViewTransform().GetAxisForward(), transform.Position);
			context.AddPrimitiveMesh_Plane(transform.Position, -viewPlane.Normal, 256.0f, nsColor::WHITE);
			context.AddPrimitiveMesh_AABB(LastIntersectionPoint - 8.0f, LastIntersectionPoint + 8.0f, nsColor::RED);
			context.AddPrimitiveMesh_AABB(CurrentIntersectionPoint - 8.0f, CurrentIntersectionPoint + 8.0f, nsColor::CYAN);
		}
		*/
	}
}




// ================================================================================================================================================================================== //
// EDITOR - GIZMO TRANSFORM
// ================================================================================================================================================================================== //
cstEditorGizmoTransform::cstEditorGizmoTransform()
{
	// Translate X-Axis
	Translates[0].Axis = cstEEditorGizmoAxis::X;
	Translates[0].Color = CST_EDITOR_GIZMO_COLOR_AXIS_X;

	// Translate Y-Axis
	Translates[1].Axis = cstEEditorGizmoAxis::Y;
	Translates[1].Color = CST_EDITOR_GIZMO_COLOR_AXIS_Y;

	// Translate Z-Axis
	Translates[2].Axis = cstEEditorGizmoAxis::Z;
	Translates[2].Color = CST_EDITOR_GIZMO_COLOR_AXIS_Z;

	// Rotate X-Axis
	Rotates[0].Axis = cstEEditorGizmoAxis::X;
	Rotates[0].Color = CST_EDITOR_GIZMO_COLOR_AXIS_X;

	// Rotate Y-Axis
	Rotates[1].Axis = cstEEditorGizmoAxis::Y;
	Rotates[1].Color = CST_EDITOR_GIZMO_COLOR_AXIS_Y;

	// Rotate X-Axis
	Rotates[2].Axis = cstEEditorGizmoAxis::Z;
	Rotates[2].Color = CST_EDITOR_GIZMO_COLOR_AXIS_Z;

	// Scale X-Axis
	Scales[0].Axis = cstEEditorGizmoAxis::X;
	Scales[0].Color = CST_EDITOR_GIZMO_COLOR_AXIS_X;

	// Scale X-Axis
	Scales[1].Axis = cstEEditorGizmoAxis::Y;
	Scales[1].Color = CST_EDITOR_GIZMO_COLOR_AXIS_Y;

	// Scale Z-Axis
	Scales[2].Axis = cstEEditorGizmoAxis::Z;
	Scales[2].Color = CST_EDITOR_GIZMO_COLOR_AXIS_Z;

	SnapTranslationValue = 0.0f;
	SnapRotationValue = 0.0f;
	SnapScaleValue = 0.125f;
	SelectedTranslate = -1;
	SelectedRotate = -1;
	SelectedScale = -1;
	bUpdating = false;

	Mode = cstEEditorGizmoTransformMode::TRANSLATE;
}


void cstEditorGizmoTransform::BeginTransform(nsViewport* viewport, const nsVector2& mousePosition, const nsTransform& transform, bool bIsLocal)
{
	if (SelectedTranslate == -1 && SelectedRotate == -1 && SelectedScale == -1)
	{
		return;
	}

	if (!bUpdating)
	{
		const float distanceToView = nsVector3::Distance(viewport->GetViewTransform().Position, transform.Position);
		const float gizmoScale = nsMath::Lerp(1.0f, 10.0f, distanceToView / 10000.0f);

		if (Mode == cstEEditorGizmoTransformMode::TRANSLATE && SelectedTranslate != -1)
		{
			NS_Assert(SelectedTranslate >= 0 && SelectedTranslate < 3);
			Translates[SelectedTranslate].BeginTranslate(viewport, mousePosition, transform, gizmoScale, bIsLocal);
		}
		else if (Mode == cstEEditorGizmoTransformMode::ROTATE && SelectedRotate != -1)
		{
			NS_Assert(SelectedRotate >= 0 && SelectedRotate < 3);
			Rotates[SelectedRotate].BeginRotate(viewport, mousePosition, transform, gizmoScale, bIsLocal);
		}
		else if (Mode == cstEEditorGizmoTransformMode::SCALE && SelectedScale != -1)
		{
			NS_Assert(SelectedScale >= 0 && SelectedScale < 3);
			Scales[SelectedScale].BeginScale(viewport, mousePosition, transform, gizmoScale);
		}

		bUpdating = true;
	}
}


void cstEditorGizmoTransform::EndTransform()
{
	if (bUpdating)
	{
		if (Mode == cstEEditorGizmoTransformMode::TRANSLATE)
		{
			Translates[SelectedTranslate].EndTranslate();
		}
		else if (Mode == cstEEditorGizmoTransformMode::ROTATE)
		{
			Rotates[SelectedRotate].EndRotate();
		}
		else if (Mode == cstEEditorGizmoTransformMode::SCALE)
		{
			Scales[SelectedScale].EndScale();
		}

		SelectedTranslate = -1;
		SelectedRotate = -1;
		SelectedScale = -1;
		bUpdating = false;
	}
}


bool cstEditorGizmoTransform::UpdateTransform(nsViewport* viewport, const nsVector2& mousePosition, cstEEditorViewMode viewMode, nsTransform& outTransform, bool bIsLocal)
{
	NS_Assert(viewport);

	const float distanceToView = nsVector3::Distance(viewport->GetViewTransform().Position, outTransform.Position);
	const float gizmoScale = (viewMode == cstEEditorViewMode::PERSPECTIVE) ? nsMath::Lerp(1.0f, 10.0f, distanceToView / 10000.0f) : 2.0f;

	if (bUpdating)
	{
		if (Mode == cstEEditorGizmoTransformMode::TRANSLATE)
		{
			NS_Assert(SelectedTranslate >= 0 && SelectedTranslate < 3);
			return Translates[SelectedTranslate].UpdateTranslation(viewport, mousePosition, outTransform, SnapTranslationValue, gizmoScale, bIsLocal);
		}
		else if (Mode == cstEEditorGizmoTransformMode::ROTATE)
		{
			NS_Assert(SelectedRotate >= 0 && SelectedRotate < 3);
			return Rotates[SelectedRotate].UpdateRotation(viewport, mousePosition, outTransform, SnapRotationValue, gizmoScale, bIsLocal);
		}
		else // Mode == cstEEditorGizmoMode::SCALE
		{
			NS_Assert(SelectedScale >= 0 && SelectedScale < 3);
			return Scales[SelectedScale].UpdateScale(viewport, mousePosition, outTransform, SnapScaleValue, gizmoScale);
		}
	}
	else
	{
		nsVector3 rayOrigin, rayDirection;
		viewport->ProjectToWorld(mousePosition, rayOrigin, rayDirection);

		float nearClip, farClip;
		viewport->GetClip(nearClip, farClip);

		const nsLine rayLine(rayOrigin, rayOrigin + rayDirection * farClip);

		float closestDist = 999999999.0f;
		int closestIndex = -1;

		if (Mode == cstEEditorGizmoTransformMode::TRANSLATE)
		{
			for (int i = 0; i < 3; ++i)
			{
				const cstEditorGizmoTranslate& gizmo = Translates[i];
				const nsLine axisLine = gizmo.GetAxisLine(outTransform, gizmoScale, bIsLocal);
				const float dist = rayLine.GetShortestDistance(axisLine);

				if (dist < closestDist)
				{
					const nsPlane viewPlane(viewport->GetViewTransform().GetAxisForward(), outTransform.Position);
					nsVector3 planeIntersectionPoint;

					if (viewPlane.TestIntersectionRay(planeIntersectionPoint, rayOrigin, rayDirection))
					{
						float t = 100.0f;
						const nsVector3 projectedIntersectionPoint = axisLine.ProjectPoint(planeIntersectionPoint, &t);

						if (t >= 0.0f && t < 1.2f)
						{
							closestDist = dist;
							closestIndex = i;
						}
					}
				}
			}

			//NS_CONSOLE_Debug(EditorLog, "Closest gizmo translate [%i]: %f", closestIndex, closestDist);
			SelectedTranslate = closestDist < 16.0f ? closestIndex : -1;
		}
		else if (Mode == cstEEditorGizmoTransformMode::ROTATE)
		{
			for (int i = 0; i < 3; ++i)
			{
				const cstEditorGizmoRotate& gizmo = Rotates[i];
				const nsLine axisLine = gizmo.GetAxisLine(outTransform, bIsLocal);
				const nsPlane axisPlane(axisLine.GetDirection(), outTransform.Position);
				nsVector3 pointOnPlane;

				if (axisPlane.TestIntersectionRay(pointOnPlane, rayOrigin, rayDirection))
				{
					nsVector3 pointOnCircle = outTransform.Position + (pointOnPlane - outTransform.Position).GetNormalized() * gizmo.CircleRadius * gizmoScale;

					nsVector2 pointOnCircleSS;
					viewport->ProjectToViewport(pointOnCircle, pointOnCircleSS);
					const float dist = (pointOnCircleSS - mousePosition).GetMagnitude();

					if (dist < closestDist)
					{
						closestDist = dist;
						closestIndex = i;
					}

					//NS_CONSOLE_Debug(EditorLog, "[%i] dist: %f", i, dist);
				}
			}

			//NS_CONSOLE_Debug(EditorLog, "Closest gizmo rotate [%i]: %f", closestIndex, closestDist);
			SelectedRotate = closestDist < 8.0f ? closestIndex : -1;
		}
		else // Mode == cstEEditorGizmoMode::SCALE
		{
			for (int i = 0; i < 3; ++i)
			{
				const cstEditorGizmoScale& gizmo = Scales[i];
				const nsLine axisLine = gizmo.GetAxisLine(outTransform, gizmoScale);
				const float dist = rayLine.GetShortestDistance(axisLine);

				if (dist < closestDist)
				{
					closestDist = dist;
					closestIndex = i;
				}

				//NS_CONSOLE_Debug(EditorLog, "[%i] dist: %f", i, dist);
			}

			//NS_CONSOLE_Debug(EditorLog, "Closest gizmo scale [%i]: %f", closestIndex, closestDist);
			SelectedScale = closestDist < 16.0f ? closestIndex : -1;
		}
	}

	return false;
}


void cstEditorGizmoTransform::Render(nsRenderer* renderer, nsViewport* viewport, const nsTransform& transform, bool bIsLocal, bool bDrawDebug)
{
	if (renderer == nullptr || viewport == nullptr)
	{
		return;
	}

	const float distanceToView = nsVector3::Distance(viewport->GetViewTransform().Position, transform.Position);
	const float gizmoScale = nsMath::Lerp(1.0f, 10.0f, distanceToView / 10000.0f);

	if (Mode == cstEEditorGizmoTransformMode::TRANSLATE)
	{
		for (int i = 0; i < 3; ++i)
		{
			Translates[i].Render(renderer, viewport, transform, gizmoScale, bIsLocal, SelectedTranslate == i, bDrawDebug);
		}
	}
	else if (Mode == cstEEditorGizmoTransformMode::ROTATE)
	{
		for (int i = 0; i < 3; ++i)
		{
			if (bUpdating)
			{
				if (SelectedRotate == i)
				{
					Rotates[i].Render(renderer, viewport, transform, gizmoScale, bIsLocal, SelectedRotate == i, bDrawDebug);
				}
			}
			else
			{
				Rotates[i].Render(renderer, viewport, transform, gizmoScale, bIsLocal, SelectedRotate == i, bDrawDebug);
			}
		}
	}
	else if (Mode == cstEEditorGizmoTransformMode::SCALE)
	{
		for (int i = 0; i < 3; ++i)
		{
			Scales[i].Render(renderer, viewport, transform, gizmoScale, SelectedScale == i, bDrawDebug);
		}
	}
}
