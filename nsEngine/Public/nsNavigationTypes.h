#pragma once

#include "nsEngineTypes.h"



class nsNavigationInputGeometry
{
private:
	nsTArray<nsVector3> Vertices;
	nsTArray<uint32> Indices;


public:
	nsNavigationInputGeometry()
	{
		Vertices.Reserve(300);
		Indices.Reserve(900);
	}


	NS_INLINE void Clear()
	{
		Vertices.Clear();
		Indices.Clear();
	}


	NS_INLINE void AddTriangle(const nsVector3& v0, const nsVector3& v1, const nsVector3& v2)
	{
		const int baseVertex = Vertices.GetCount();
		Vertices.Resize(baseVertex + 3);
		Vertices[baseVertex] = v0;
		Vertices[baseVertex + 1] = v1;
		Vertices[baseVertex + 2] = v2;

		const int baseIndex = Indices.GetCount();
		Indices.Resize(baseIndex + 3);
		Indices[baseIndex] = baseIndex;
		Indices[baseIndex + 1] = baseIndex + 1;
		Indices[baseIndex + 2] = baseIndex + 2;
	}


	NS_INLINE void AddTriangle(const nsVector3* triVertices)
	{
		AddTriangle(triVertices[0], triVertices[1], triVertices[2]);
	}


	NS_NODISCARD_INLINE const nsTArray<nsVector3>& GetVertices() const
	{
		return Vertices;
	}


	NS_NODISCARD_INLINE const nsVector3* GetVertexData() const
	{
		return Vertices.GetData();
	}


	NS_NODISCARD_INLINE int GetVertexCount() const
	{
		return Vertices.GetCount();
	}


	NS_NODISCARD_INLINE const nsTArray<uint32>& GetIndices() const
	{
		return Indices;
	}


	NS_NODISCARD_INLINE const uint32* GetIndexData() const
	{
		return Indices.GetData();
	}


	NS_NODISCARD_INLINE int GetTriangleCount() const
	{
		NS_Assert(Indices.GetCount() % 3 == 0);
		return Indices.GetCount() / 3;
	}

};



struct nsNavigationBuildSettings
{
	nsAABB AreaBound;
	float CellSize;
	float CellHeight;
	float MaxWalkableHeight;
	float MaxWalkableClimb;
	float MaxWalkableSlopeDegree;
	int MinRegionArea;


public:
	nsNavigationBuildSettings()
	{
		CellSize = 16.0f;
		CellHeight = 8.0f;
		MaxWalkableHeight = 200.0f;
		MaxWalkableClimb = 30.0f;
		MaxWalkableSlopeDegree = 45.0f;
		MinRegionArea = 16;
	}

};
