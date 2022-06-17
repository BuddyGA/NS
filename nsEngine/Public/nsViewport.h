#pragma once

#include "nsEngineTypes.h"



class NS_ENGINE_API nsViewport
{
private:
	nsMatrix4 ViewMatrix;
	nsMatrix4 ProjectionMatrix;
	nsMatrix4 ViewProjectionMatrix;
	nsTArrayInline<nsPlane, 6> Frustums;
	nsTransform ViewTransform;
	nsPointFloat Dimension;
	float FoV;
	float NearClip;
	float FarClip;
	bool bIsOrthographic;
	bool bUpdateView;
	bool bUpdateProjection;
	bool bUpdateViewProjection;
	bool bUpdateFrustums;


public:
	nsViewport() noexcept;
	const nsMatrix4& GetViewMatrix() noexcept;
	const nsMatrix4& GetProjectionMatrix() noexcept;
	const nsMatrix4& GetViewProjectionMatrix() noexcept;
	const nsTArrayInline<nsPlane, 6>& GetFrustums() noexcept;
	bool ProjectToWorld(const nsVector2& viewportPosition, nsVector3& outWorldPosition, nsVector3& outDirection) noexcept;
	bool ProjectToViewport(const nsVector3& worldPosition, nsVector2& outViewportPosition) noexcept;


	NS_INLINE void SetViewTransform(const nsVector3& worldPosition, const nsQuaternion& worldRotation) noexcept
	{
		ViewTransform.Position = worldPosition;
		ViewTransform.Rotation = worldRotation;
		bUpdateView = true;
	}


	NS_INLINE void SetViewTransform(const nsTransform& worldTransform) noexcept
	{
		ViewTransform.Position = worldTransform.Position;
		ViewTransform.Rotation = worldTransform.Rotation;
		bUpdateView = true;
	}

	
	NS_INLINE void SetDimension(float width, float height) noexcept
	{
		Dimension.X = width;
		Dimension.Y = height;
		bUpdateProjection = true;
	}


	NS_INLINE void SetFoV(float degree) noexcept
	{
		FoV = degree;
		bUpdateProjection = true;
	}


	NS_INLINE void SetClip(float nearClip, float farClip) noexcept
	{
		NearClip = nearClip;
		FarClip = farClip;
		bUpdateProjection = true;
	}


	NS_INLINE void SetProjectionMode(bool bOrthograpic) noexcept
	{
		if (bIsOrthographic != bOrthograpic)
		{
			bIsOrthographic = bOrthograpic;
			bUpdateProjection = true;
		}
	}


	NS_NODISCARD_INLINE const nsTransform& GetViewTransform() const noexcept
	{
		return ViewTransform;
	}


	NS_NODISCARD_INLINE const nsPointFloat& GetDimension() const noexcept
	{
		return Dimension;
	}


	NS_NODISCARD_INLINE float GetAspect() const noexcept
	{
		return Dimension.X / Dimension.Y;
	}


	NS_NODISCARD_INLINE float GetFoV() const noexcept
	{
		return FoV;
	}


	NS_INLINE void GetClip(float& outNearClip, float& outFarClip) const noexcept
	{
		outNearClip = NearClip;
		outFarClip = FarClip;
	}


	NS_NODISCARD_INLINE bool IsOrthograpic() const noexcept
	{
		return bIsOrthographic;
	}

};
