#pragma once

#include "nsEngineTypes.h"



namespace nsGeometryFactory
{
	extern NS_ENGINE_API void AddMeshBox(nsTArray<nsVertexMeshPosition>& outPositions, nsTArray<nsVertexMeshAttribute>& outAttributes, nsTArray<uint32>& outIndices, const nsVector3& boxMin, const nsVector3& boxMax, const nsVector2& uvScale) noexcept;

	extern NS_ENGINE_API void AddPrimitiveLine(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 start, nsVector3 end, nsColor color) noexcept;
	extern NS_ENGINE_API void AddPrimitiveLine_Circle(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 center, float radius, float halfArcRadian, nsEAxisType arcAxis, nsColor color) noexcept;
	extern NS_ENGINE_API void AddPrimitiveLine_Sphere(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 center, float radius, nsColor color) noexcept;
	extern NS_ENGINE_API void AddPrimitiveLine_AABB(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 boxMin, nsVector3 boxMax, nsColor color) noexcept;
	extern NS_ENGINE_API void AddPrimitiveLine_OBB(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 center, nsQuaternion rotation, nsVector3 halfExtents, nsColor color) noexcept;
	extern NS_ENGINE_API void AddPrimitiveLine_Capsule(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 position, nsQuaternion rotation, float radius, float height, nsColor color) noexcept;
	extern NS_ENGINE_API void AddPrimitiveLine_Convex(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, const nsVector3* points, int count, nsColor color) noexcept;

	extern NS_ENGINE_API void AddPrimitiveTriangle(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 a, nsVector3 b, nsVector3 c, nsColor color, bool bIsLine) noexcept;
	extern NS_ENGINE_API void AddPrimitiveTriangle_Plane(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 normal, float width, nsColor color) noexcept;
	extern NS_ENGINE_API void AddPrimitiveTriangle_AABB(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 boxMin, nsVector3 boxMax, nsColor color) noexcept;
	extern NS_ENGINE_API void AddPrimitiveTriangle_OBB(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 center, nsQuaternion rotation, nsVector3 halfExtents, nsColor color) noexcept;
	extern NS_ENGINE_API void AddPrimitiveTriangle_Prism(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, float width, float height, nsColor color) noexcept;
	extern NS_ENGINE_API void AddPrimitiveTriangle_Arrow(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, float shaftLength, float width, float tipHeight, nsColor shaftColor, nsColor arrowColor) noexcept;
};
