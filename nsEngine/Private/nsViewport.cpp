#include "nsViewport.h"



static const nsMatrix4 VULKAN_PERSPECTIVE_MATRIX(1.0f,  0.0f, 0.0f, 0.0f,
												 0.0f, -1.0f, 0.0f, 0.0f,
												 0.0f,  0.0f, 0.5f, 0.0f,
												 0.0f,  0.0f, 0.5f, 1.0f);



nsViewport::nsViewport() noexcept
{
	ViewMatrix = nsMatrix4::IDENTITY;
	ProjectionMatrix = nsMatrix4::IDENTITY;
	ViewProjectionMatrix = nsMatrix4::IDENTITY;
	ViewTransform = nsTransform();
	Dimension = nsPointFloat(1280.0f, 720.0f);
	FoV = 90.0f;
	NearClip = 10.0f;
	FarClip = 100000.0f;
	bIsOrthographic = false;
	bUpdateView = true;
	bUpdateProjection = true;
	bUpdateViewProjection = true;
	bUpdateFrustums = true;
}


const nsMatrix4& nsViewport::GetViewMatrix() noexcept
{
	if (bUpdateView)
	{
		ViewMatrix = ViewTransform.ToMatrixNoScale().GetInverseTransformNoScale();
		bUpdateView = false;
		bUpdateViewProjection = true;
	}

	return ViewMatrix;
}


const nsMatrix4& nsViewport::GetProjectionMatrix() noexcept
{
	if (bUpdateProjection)
	{
		if (bIsOrthographic)
		{
			ProjectionMatrix = nsMatrix4::Orthographic(-Dimension.X, Dimension.X, -Dimension.X, Dimension.X, -FarClip, FarClip);
		}
		else
		{
			ProjectionMatrix = VULKAN_PERSPECTIVE_MATRIX * nsMatrix4::Perspective(FoV, Dimension.X / Dimension.Y, NearClip, FarClip);
		}

		bUpdateProjection = false;
		bUpdateViewProjection = true;
	}

	return ProjectionMatrix;
}


const nsMatrix4& nsViewport::GetViewProjectionMatrix() noexcept
{
	if (bUpdateViewProjection)
	{
		ViewProjectionMatrix = GetViewMatrix() * GetProjectionMatrix();
		bUpdateViewProjection = false;
		bUpdateFrustums = true;
	}

	return ViewProjectionMatrix;
}


const nsTArrayInline<nsPlane, 6>& nsViewport::GetFrustums() noexcept
{
	const nsMatrix4& viewProj = GetViewProjectionMatrix();

	if (bUpdateFrustums)
	{
		Frustums.Resize(6);

		// TODO: Calculate frustum planes

		bUpdateFrustums = false;
	}

	return Frustums;
}


bool nsViewport::ProjectToWorld(const nsVector2& viewportPosition, nsVector3& outWorldPosition, nsVector3& outDirection) noexcept
{
	if (Dimension.X <= 0.0f || Dimension.Y <= 0.0f)
	{
		return false;
	}

	const float nx = nsMath::NormalizeMinusOneToOne(viewportPosition.X, 0.0f, Dimension.X);
	const float ny = nsMath::NormalizeMinusOneToOne(viewportPosition.Y, 0.0f, Dimension.Y);

	if (nx < -1.0f || nx > 1.0f || ny < -1.0f || ny > 1.0f)
	{
		return false;
	}

	nsVector4 rayStartClipSpace(nx, ny, 0.0f, 1.0f);
	nsVector4 rayEndClipSpace(nx, ny, 0.1f, 1.0f);

	const nsMatrix4 invProjectionMatrix = GetProjectionMatrix().GetInverse();
	nsVector4 rayStartViewSpace = rayStartClipSpace * invProjectionMatrix;
	nsVector4 rayEndViewSpace = rayEndClipSpace * invProjectionMatrix;

	if (rayStartViewSpace.W != 0.0f)
	{
		rayStartViewSpace /= rayStartViewSpace.W;
	}

	if (rayEndViewSpace.W != 0.0f)
	{
		rayEndViewSpace /= rayEndViewSpace.W;
	}

	nsVector4 rayDirViewSpace = rayEndViewSpace - rayStartViewSpace;
	rayDirViewSpace.Normalize();

	const nsMatrix4 invViewMatrix = GetViewMatrix().GetInverseTransformNoScale();
	const nsVector4 rayOriginWorldSpace = rayStartViewSpace * invViewMatrix;
	const nsVector4 rayDirWorldSpace = rayDirViewSpace * invViewMatrix;

	outWorldPosition = rayOriginWorldSpace.ToVector3();
	outDirection = rayDirWorldSpace.ToVector3();
	outDirection.Normalize();

	return true;
}


bool nsViewport::ProjectToViewport(const nsVector3& worldPosition, nsVector2& outViewportPosition) noexcept
{
	if (Dimension.X <= 0.0f || Dimension.Y <= 0.0f)
	{
		return false;
	}

	const nsMatrix4& viewProj = GetViewProjectionMatrix();
	nsVector4 hg = nsVector4(worldPosition, 1.0f) * viewProj;

	if (hg.W <= 0.0f)
	{
		return false;
	}

	hg.X /= hg.W;
	hg.Y /= hg.W;

	if (hg.X < -1.5f || hg.X > 1.5f || hg.Y < -1.5f || hg.Y > 1.5f)
	{
		return false;
	}

	const float nx = nsMath::NormalizeZeroToOne(hg.X, -1.0f, 1.0f);
	const float ny = nsMath::NormalizeZeroToOne(hg.Y, -1.0f, 1.0f);
	outViewportPosition.X = nx * Dimension.X;
	outViewportPosition.Y = ny * Dimension.Y;

	return true;
}
