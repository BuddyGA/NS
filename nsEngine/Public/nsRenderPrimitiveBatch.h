#pragma once

#include "nsEngineTypes.h"



class NS_ENGINE_API nsRenderPrimitiveBatch
{
	NS_DECLARE_NOCOPY(nsRenderPrimitiveBatch)

protected:
	nsTArray<nsVertexPrimitive> Vertices;
	nsTArray<uint32> Indices;


public:
	nsRenderPrimitiveBatch();
	void UpdateIndicesAtVertexOffset(int vertexOffset);


	NS_INLINE void Clear(bool bFreeMemory = false)
	{
		Vertices.Clear(bFreeMemory);
		Indices.Clear(bFreeMemory);
	}


	NS_NODISCARD_INLINE const nsTArray<nsVertexPrimitive>& GetVertices() const
	{
		return Vertices;
	}


	NS_NODISCARD_INLINE const nsTArray<uint32>& GetIndices() const
	{
		return Indices;
	}


	NS_NODISCARD_INLINE uint64 GetVertexSizeBytes() const
	{
		return sizeof(nsVertexPrimitive) * Vertices.GetCount();
	}


	NS_NODISCARD_INLINE uint64 GetIndexSizeBytes() const
	{
		return sizeof(uint32) * Indices.GetCount();
	}


	NS_NODISCARD_INLINE bool IsEmpty() const
	{
		return Vertices.IsEmpty();
	}

};



class NS_ENGINE_API nsRenderPrimitiveBatchLine : public nsRenderPrimitiveBatch
{
public:
	nsRenderPrimitiveBatchLine();
	void AddLine(const nsVector3& start, const nsVector3& end, nsColor color);
	void AddBoxAABB(const nsVector3& boxMin, const nsVector3& boxMax, nsColor color);
	void AddCircle(const nsVector3& center, float radius, float halfArcRadian, nsEAxisType arcAxis, nsColor color);
	void AddCircleAroundAxis(const nsVector3& center, float radius, const nsVector3& axis, float halfArcRadian, nsColor color);


	NS_INLINE void AddSphere(const nsVector3& center, float radius, nsColor color)
	{
		AddCircle(center, radius, NS_MATH_PI, nsEAxisType::X_Axis, color);
		AddCircle(center, radius, NS_MATH_PI, nsEAxisType::Y_Axis, color);
		AddCircle(center, radius, NS_MATH_PI, nsEAxisType::Z_Axis, color);
	}

};




class NS_ENGINE_API nsRenderPrimitiveBatchMesh : public nsRenderPrimitiveBatch
{
public:
	nsRenderPrimitiveBatchMesh();
	void AddTriangle(const nsVector3& a, const nsVector3& b, const nsVector3& c, nsColor color);
	void AddBoxAABB(const nsVector3& boxMin, const nsVector3& boxMax, nsColor color);
	void AddBoxOBB(const nsVector3& center, const nsVector3& halfExtent, const nsQuaternion& rotation, nsColor color);
	void AddPrism(const nsVector3& position, const nsQuaternion& rotation, float width, float height, nsColor color);

};
