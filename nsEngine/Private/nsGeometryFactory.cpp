#include "nsGeometryFactory.h"



void nsGeometryFactory::AddMeshBox(nsTArray<nsVertexMeshPosition>& outPositions, nsTArray<nsVertexMeshAttribute>& outAttributes, nsTArray<uint32>& outIndices, const nsVector3& boxMin, const nsVector3& boxMax, const nsVector2& uvScale) noexcept
{
	const int VERTEX_COUNT = 24;
	const int baseVertex = outPositions.GetCount();
	
	// Vertex Positions
	int vtxId = baseVertex;
	outPositions.Resize(baseVertex + VERTEX_COUNT);
	// front
	outPositions[vtxId++] = nsVector3(boxMin.X, boxMax.Y, boxMin.Z); // 0
	outPositions[vtxId++] = nsVector3(boxMax.X, boxMax.Y, boxMin.Z); // 1
	outPositions[vtxId++] = nsVector3(boxMax.X, boxMin.Y, boxMin.Z); // 2
	outPositions[vtxId++] = nsVector3(boxMin.X, boxMin.Y, boxMin.Z); // 3
	// back
	outPositions[vtxId++] = nsVector3(boxMax.X, boxMax.Y, boxMax.Z); // 4
	outPositions[vtxId++] = nsVector3(boxMin.X, boxMax.Y, boxMax.Z); // 5
	outPositions[vtxId++] = nsVector3(boxMin.X, boxMin.Y, boxMax.Z); // 6
	outPositions[vtxId++] = nsVector3(boxMax.X, boxMin.Y, boxMax.Z); // 7
	// right
	outPositions[vtxId++] = nsVector3(boxMax.X, boxMax.Y, boxMin.Z); // 8
	outPositions[vtxId++] = nsVector3(boxMax.X, boxMax.Y, boxMax.Z); // 9
	outPositions[vtxId++] = nsVector3(boxMax.X, boxMin.Y, boxMax.Z); // 10
	outPositions[vtxId++] = nsVector3(boxMax.X, boxMin.Y, boxMin.Z); // 11
	// left
	outPositions[vtxId++] = nsVector3(boxMin.X, boxMax.Y, boxMax.Z); // 12
	outPositions[vtxId++] = nsVector3(boxMin.X, boxMax.Y, boxMin.Z); // 13
	outPositions[vtxId++] = nsVector3(boxMin.X, boxMin.Y, boxMin.Z); // 14
	outPositions[vtxId++] = nsVector3(boxMin.X, boxMin.Y, boxMax.Z); // 15
	// top
	outPositions[vtxId++] = nsVector3(boxMin.X, boxMax.Y, boxMax.Z); // 16
	outPositions[vtxId++] = nsVector3(boxMax.X, boxMax.Y, boxMax.Z); // 17
	outPositions[vtxId++] = nsVector3(boxMax.X, boxMax.Y, boxMin.Z); // 18
	outPositions[vtxId++] = nsVector3(boxMin.X, boxMax.Y, boxMin.Z); // 19
	// bottom
	outPositions[vtxId++] = nsVector3(boxMin.X, boxMin.Y, boxMin.Z); // 20
	outPositions[vtxId++] = nsVector3(boxMax.X, boxMin.Y, boxMin.Z); // 21
	outPositions[vtxId++] = nsVector3(boxMax.X, boxMin.Y, boxMax.Z); // 22
	outPositions[vtxId++] = nsVector3(boxMin.X, boxMin.Y, boxMax.Z); // 23


	// Vertex Attributes
	vtxId = baseVertex;
	outAttributes.Resize(baseVertex + VERTEX_COUNT);
	// front
	outAttributes[vtxId++] = { nsVector3(0.0f, 0.0f, -1.0f), nsVector3(0.0f), nsVector2(0.0f, 0.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(0.0f, 0.0f, -1.0f), nsVector3(0.0f), nsVector2(1.0f, 0.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(0.0f, 0.0f, -1.0f), nsVector3(0.0f), nsVector2(1.0f, 1.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(0.0f, 0.0f, -1.0f), nsVector3(0.0f), nsVector2(0.0f, 1.0f) * uvScale };
	// back
	outAttributes[vtxId++] = { nsVector3(0.0f, 0.0f, 1.0f), nsVector3(0.0f), nsVector2(0.0f, 0.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(0.0f, 0.0f, 1.0f), nsVector3(0.0f), nsVector2(1.0f, 0.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(0.0f, 0.0f, 1.0f), nsVector3(0.0f), nsVector2(1.0f, 1.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(0.0f, 0.0f, 1.0f), nsVector3(0.0f), nsVector2(0.0f, 1.0f) * uvScale };
	// right
	outAttributes[vtxId++] = { nsVector3(1.0f, 0.0f, 0.0f), nsVector3(0.0f), nsVector2(0.0f, 0.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(1.0f, 0.0f, 0.0f), nsVector3(0.0f), nsVector2(1.0f, 0.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(1.0f, 0.0f, 0.0f), nsVector3(0.0f), nsVector2(1.0f, 1.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(1.0f, 0.0f, 0.0f), nsVector3(0.0f), nsVector2(0.0f, 1.0f) * uvScale };
	// left
	outAttributes[vtxId++] = { nsVector3(-1.0f, 0.0f, 0.0f), nsVector3(0.0f), nsVector2(0.0f, 0.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(-1.0f, 0.0f, 0.0f), nsVector3(0.0f), nsVector2(1.0f, 0.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(-1.0f, 0.0f, 0.0f), nsVector3(0.0f), nsVector2(1.0f, 1.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(-1.0f, 0.0f, 0.0f), nsVector3(0.0f), nsVector2(0.0f, 1.0f) * uvScale };
	// top
	outAttributes[vtxId++] = { nsVector3(0.0f, 1.0f, 0.0f), nsVector3(0.0f), nsVector2(0.0f, 0.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(0.0f, 1.0f, 0.0f), nsVector3(0.0f), nsVector2(1.0f, 0.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(0.0f, 1.0f, 0.0f), nsVector3(0.0f), nsVector2(1.0f, 1.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(0.0f, 1.0f, 0.0f), nsVector3(0.0f), nsVector2(0.0f, 1.0f) * uvScale };
	// bottom
	outAttributes[vtxId++] = { nsVector3(0.0f, -1.0f, 0.0f), nsVector3(0.0f), nsVector2(0.0f, 0.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(0.0f, -1.0f, 0.0f), nsVector3(0.0f), nsVector2(1.0f, 0.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(0.0f, -1.0f, 0.0f), nsVector3(0.0f), nsVector2(1.0f, 1.0f) * uvScale };
	outAttributes[vtxId++] = { nsVector3(0.0f, -1.0f, 0.0f), nsVector3(0.0f), nsVector2(0.0f, 1.0f) * uvScale };


	// Indices
	const int INDEX_COUNT = 36;
	const uint32 indices[INDEX_COUNT] =
	{
		0, 1, 2, 2, 3, 0, // back
		4, 5, 6, 6, 7, 4, // front
		8, 9, 10, 10, 11, 8, // right
		12, 13, 14, 14, 15, 12, // left
		16, 17, 18, 18, 19, 16, // top
		20, 21, 22, 22, 23, 20 // bottom
	};

	const int baseIndex = outIndices.GetCount();
	outIndices.Resize(baseIndex + INDEX_COUNT);

	for (int i = 0; i < INDEX_COUNT; ++i)
	{
		outIndices[baseIndex + i] = baseVertex + indices[i];
	}
}




static int ns_AddPrimitiveAABB(nsTArray<nsVertexPrimitive>& outVertices, nsVector3 boxMin, nsVector3 boxMax, nsColor color) noexcept
{
	const int VERTEX_COUNT = 8;
	const int baseVertex = outVertices.GetCount();
	int vtxId = baseVertex;
	outVertices.Resize(baseVertex + VERTEX_COUNT);
	outVertices[vtxId++] = { nsVector3(boxMin.X, boxMax.Y, boxMin.Z), color };
	outVertices[vtxId++] = { nsVector3(boxMax.X, boxMax.Y, boxMin.Z), color };
	outVertices[vtxId++] = { nsVector3(boxMax.X, boxMin.Y, boxMin.Z), color };
	outVertices[vtxId++] = { nsVector3(boxMin.X, boxMin.Y, boxMin.Z), color };
	outVertices[vtxId++] = { nsVector3(boxMin.X, boxMax.Y, boxMax.Z), color };
	outVertices[vtxId++] = { nsVector3(boxMax.X, boxMax.Y, boxMax.Z), color };
	outVertices[vtxId++] = { nsVector3(boxMax.X, boxMin.Y, boxMax.Z), color };
	outVertices[vtxId++] = { nsVector3(boxMin.X, boxMin.Y, boxMax.Z), color };

	return baseVertex;
}



void nsGeometryFactory::AddPrimitiveLine(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 start, nsVector3 end, nsColor color) noexcept
{
	const int VERTEX_COUNT = 2;
	const int baseVertex = outVertices.GetCount();

	int vtxId = baseVertex;
	outVertices.Resize(baseVertex + VERTEX_COUNT);
	outVertices[vtxId++] = { start, color };
	outVertices[vtxId++] = { end, color };

	const int INDEX_COUNT = 2;
	int idxId = outIndices.GetCount();
	outIndices.Resize(idxId + INDEX_COUNT);
	outIndices[idxId++] = baseVertex;
	outIndices[idxId++] = baseVertex + 1;
}


void nsGeometryFactory::AddPrimitiveLine_Circle(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 center, float radius, float halfArcRadian, nsEAxisType arcAxis, nsColor color) noexcept
{
	constexpr int COUNT = 36;
	const float step = halfArcRadian * 2.0f / COUNT;

	const int vertexCount = COUNT + 1;
	const uint32 baseVertex = (uint32)outVertices.GetCount();
	outVertices.Resize(baseVertex + vertexCount);
	float theta = 0.0f;

	for (uint32 i = baseVertex; i < baseVertex + vertexCount; ++i)
	{
		const float sinTheta = sinf(theta);
		const float cosTheta = cosf(theta);
		nsVertexPrimitive& vertex = outVertices[i];

		if (arcAxis == nsEAxisType::X_Axis)
		{
			vertex.Position = nsVector3(center.X, center.Y + radius * sinTheta, center.Z + radius * cosTheta);
		}
		else if (arcAxis == nsEAxisType::Y_Axis)
		{
			vertex.Position = nsVector3(center.X + radius * cosTheta, center.Y, center.Z + radius * sinTheta);
		}
		else // Z_Axis
		{
			vertex.Position = nsVector3(center.X + radius * cosTheta, center.Y + radius * sinTheta, center.Z);
		}

		vertex.Color = color;
		theta += step;
	}

	const int indexCount = vertexCount * 2;
	const uint32 baseIndex = (uint32)outIndices.GetCount();
	outIndices.Resize(baseIndex + indexCount);

	outIndices[baseIndex] = baseVertex;
	outIndices[baseIndex + 1] = baseVertex + 1;

	for (uint32 i = baseIndex + 2; i < baseIndex + indexCount; i += 2)
	{
		outIndices[i] = outIndices[i - 1];
		outIndices[i + 1] = outIndices[i] + 1;
	}

	outIndices[baseIndex + indexCount - 1] = outIndices[baseIndex + indexCount - 2];
}


void nsGeometryFactory::AddPrimitiveLine_Sphere(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 center, float radius, nsColor color) noexcept
{
	AddPrimitiveLine_Circle(outVertices, outIndices, center, radius, NS_MATH_PI, nsEAxisType::X_Axis, color);
	AddPrimitiveLine_Circle(outVertices, outIndices, center, radius, NS_MATH_PI, nsEAxisType::Y_Axis, color);
	AddPrimitiveLine_Circle(outVertices, outIndices, center, radius, NS_MATH_PI, nsEAxisType::Z_Axis, color);
}


void nsGeometryFactory::AddPrimitiveLine_AABB(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 boxMin, nsVector3 boxMax, nsColor color) noexcept
{
	const int baseVertex = ns_AddPrimitiveAABB(outVertices, boxMin, boxMax, color);

	const int INDEX_COUNT = 24;
	const uint32 indices[INDEX_COUNT] =
	{
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7, 
	};

	const int baseIndex = outIndices.GetCount();
	outIndices.Resize(baseIndex + INDEX_COUNT);

	for (int i = 0; i < INDEX_COUNT; ++i)
	{
		outIndices[baseIndex + i] = baseVertex + indices[i];
	}
}


void nsGeometryFactory::AddPrimitiveLine_OBB(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 center, nsQuaternion rotation, nsVector3 halfExtents, nsColor color) noexcept
{
	const int baseVertex = outVertices.GetCount();
	AddPrimitiveLine_AABB(outVertices, outIndices, -halfExtents, halfExtents, color);
	const nsMatrix4 rotMatrix = nsTransform(center, rotation).ToMatrixNoScale();

	for (int i = 0; i < 8; ++i)
	{
		const int vtxId = baseVertex + i;
		const nsVector4 position = outVertices[vtxId].Position;
		outVertices[vtxId].Position = position * rotMatrix;
	}
}


void nsGeometryFactory::AddPrimitiveLine_Capsule(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 position, nsQuaternion rotation, float radius, float height, nsColor color) noexcept
{
	const int baseVertex = outVertices.GetCount();

	// Bottom half sphere
	const nsVector3 bottomSphereCenter = nsVector3(0.0f, 0.0f + radius, 0.0f);
	AddPrimitiveLine_Circle(outVertices, outIndices, bottomSphereCenter, radius, -NS_MATH_PI_2, nsEAxisType::X_Axis, color);
	AddPrimitiveLine_Circle(outVertices, outIndices, bottomSphereCenter, radius, NS_MATH_PI, nsEAxisType::Y_Axis, color);
	AddPrimitiveLine_Circle(outVertices, outIndices, bottomSphereCenter, radius, -NS_MATH_PI_2, nsEAxisType::Z_Axis, color);

	// Top half sphere
	const nsVector3 topSphereCenter = nsVector3(0.0f, bottomSphereCenter.Y + height, 0.0f);
	AddPrimitiveLine_Circle(outVertices, outIndices, topSphereCenter, radius, NS_MATH_PI_2, nsEAxisType::X_Axis, color);
	AddPrimitiveLine_Circle(outVertices, outIndices, topSphereCenter, radius, NS_MATH_PI, nsEAxisType::Y_Axis, color);
	AddPrimitiveLine_Circle(outVertices, outIndices, topSphereCenter, radius, NS_MATH_PI_2, nsEAxisType::Z_Axis, color);

	// Mid lines
	const nsVector3 bottomPointBack = bottomSphereCenter + nsVector3(0.0f, 0.0f, -radius);
	const nsVector3 bottomPointFront = bottomSphereCenter + nsVector3(0.0f, 0.0f, radius);
	const nsVector3 bottomPointLeft = bottomSphereCenter + nsVector3(-radius, 0.0f, 0.0f);
	const nsVector3 bottomPointRight = bottomSphereCenter + nsVector3(radius, 0.0f, 0.0f);

	const nsVector3 topPointBack = topSphereCenter + nsVector3(0.0f, 0.0f, -radius);
	const nsVector3 topPointFront = topSphereCenter + nsVector3(0.0f, 0.0f, radius);
	const nsVector3 topPointLeft = topSphereCenter + nsVector3(-radius, 0.0f, 0.0f);
	const nsVector3 topPointRight = topSphereCenter + nsVector3(radius, 0.0f, 0.0f);

	AddPrimitiveLine(outVertices, outIndices, bottomPointBack, topPointBack, color);
	AddPrimitiveLine(outVertices, outIndices, bottomPointFront, topPointFront, color);
	AddPrimitiveLine(outVertices, outIndices, bottomPointLeft, topPointLeft, color);
	AddPrimitiveLine(outVertices, outIndices, bottomPointRight, topPointRight, color);

	const nsMatrix4 rotMatrix = nsTransform(position, rotation).ToMatrixNoScale();
	const int vertexCount = outVertices.GetCount() - baseVertex;

	for (int i = 0; i < vertexCount; ++i)
	{
		const int vtxId = baseVertex + i;
		outVertices[vtxId].Position = outVertices[vtxId].Position * rotMatrix;
	}
}


void nsGeometryFactory::AddPrimitiveLine_Convex(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, const nsVector3* points, int count, nsColor color) noexcept
{
	const int baseVertex = outVertices.GetCount();
	outVertices.Resize(baseVertex + count);

	for (int i = 0; i < count; ++i)
	{
		outVertices[baseVertex + i] = { points[i], color };
	}

	const int INDEX_COUNT = count * 2;
	const int baseIndex = outIndices.GetCount();
	outIndices.Resize(baseIndex + INDEX_COUNT);

	outIndices[baseIndex] = baseVertex;

	for (int i = 1; i < INDEX_COUNT; i += 2)
	{
		const int idx = baseIndex + i;
		outIndices[idx] = outIndices[idx - 1] + 1;

		if (idx >= outIndices.GetCount() - 1)
		{
			break;
		}

		outIndices[idx + 1] = outIndices[idx];
	}

	outIndices[baseIndex + INDEX_COUNT - 1] = baseVertex;
}


void nsGeometryFactory::AddPrimitiveTriangle(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 a, nsVector3 b, nsVector3 c, nsColor color, bool bIsLine) noexcept
{
	const int VERTEX_COUNT = 3;
	const int baseVertex = outVertices.GetCount();

	int vtxId = baseVertex;
	outVertices.Resize(baseVertex + VERTEX_COUNT);
	outVertices[vtxId++] = { a, color };
	outVertices[vtxId++] = { b, color };
	outVertices[vtxId++] = { c, color };

	const int baseIndex = outIndices.GetCount();

	if (bIsLine)
	{
		const int INDEX_COUNT = 6;
		const uint32 indices[INDEX_COUNT] = { 0, 1, 1, 2, 2, 0 };
		outIndices.Resize(baseIndex + INDEX_COUNT);

		for (int i = 0; i < INDEX_COUNT; ++i)
		{
			outIndices[baseIndex + i] = baseVertex + indices[i];
		}
	}
	else
	{
		outIndices.Resize(baseIndex + 3);
		outIndices[baseIndex] = baseVertex;
		outIndices[baseIndex + 1] = baseVertex + 1;
		outIndices[baseIndex + 2] = baseVertex + 2;
	}
}


void nsGeometryFactory::AddPrimitiveTriangle_Plane(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 normal, float width, nsColor color) noexcept
{
	NS_Assert(width > 0.0f);

	const int VERTEX_COUNT = 4;
	const int baseVertex = outVertices.GetCount();
	outVertices.Resize(baseVertex + VERTEX_COUNT);
	outVertices[baseVertex + 0] = { nsVector4(-0.5f, 0.0f,  0.5f, 1.0f), color }; // 0
	outVertices[baseVertex + 1] = { nsVector4( 0.5f, 0.0f,  0.5f, 1.0f), color }; // 1
	outVertices[baseVertex + 2] = { nsVector4( 0.5f, 0.0f, -0.5f, 1.0f), color }; // 2
	outVertices[baseVertex + 3] = { nsVector4(-0.5f, 0.0f, -0.5f, 1.0f), color }; // 3

	for (int i = 0; i < VERTEX_COUNT; ++i)
	{
		nsVector3 position = outVertices[baseVertex + i].Position.ToVector3();
		position = nsQuaternion::RotateVector(nsQuaternion::FromVectors(nsVector3::UP, normal.GetNormalized()), position);
		position *= width;
		outVertices[baseVertex + i].Position = nsVector4(position, 1.0f);
	}

	const int INDEX_COUNT = 6;
	const uint32 indices[INDEX_COUNT] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	const int baseIndex = outIndices.GetCount();
	outIndices.Resize(baseIndex + INDEX_COUNT);

	for (int i = 0; i < INDEX_COUNT; ++i)
	{
		outIndices[baseIndex + i] = baseVertex + indices[i];
	}
}


void nsGeometryFactory::AddPrimitiveTriangle_AABB(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 boxMin, nsVector3 boxMax, nsColor color) noexcept
{
	const int baseVertex = ns_AddPrimitiveAABB(outVertices, boxMin, boxMax, color);

	const int INDEX_COUNT = 36;
	const uint32 indices[INDEX_COUNT] =
	{
		0, 1, 2, 2, 3, 0, // back
		1, 5, 6, 6, 2, 1, // right
		5, 4, 7, 7, 6, 5, // front
		4, 0, 3, 3, 7, 4, // left
		4, 5, 1, 1, 0, 4, // top
		3, 2, 6, 6, 7, 3, // bottom
	};

	const int baseIndex = outIndices.GetCount();
	outIndices.Resize(baseIndex + INDEX_COUNT);

	for (int i = 0; i < INDEX_COUNT; ++i)
	{
		outIndices[baseIndex + i] = baseVertex + indices[i];
	}
}


void nsGeometryFactory::AddPrimitiveTriangle_OBB(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, nsVector3 center, nsQuaternion rotation, nsVector3 halfExtents, nsColor color) noexcept
{
	const int baseVertex = outVertices.GetCount();
	AddPrimitiveTriangle_AABB(outVertices, outIndices, -halfExtents, halfExtents, color);
	const nsMatrix4 worldMatrix = nsTransform(center, rotation).ToMatrixNoScale();

	for (int i = 0; i < 8; ++i)
	{
		const int vtxId = baseVertex + i;
		const nsVector4 position = outVertices[vtxId].Position;
		outVertices[vtxId].Position = position * worldMatrix;
	}
}


void nsGeometryFactory::AddPrimitiveTriangle_Prism(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, float width, float height, nsColor color) noexcept
{
	NS_Assert(width > 0.0f && height > 0.0f);

	const float halfWidth = width * 0.5f;

	const int VERTEX_COUNT = 5;
	const int baseVertex = outVertices.GetCount();

	outVertices.Resize(baseVertex + VERTEX_COUNT);
	outVertices[baseVertex + 0] = { nsVector4(0.0f, height, 0.0f, 1.0f), color }; // 0
	outVertices[baseVertex + 1] = { nsVector4(-halfWidth, 0.0f, -halfWidth, 1.0f), color }; // 1
	outVertices[baseVertex + 2] = { nsVector4(halfWidth, 0.0f, -halfWidth, 1.0f), color }; // 2
	outVertices[baseVertex + 3] = { nsVector4(halfWidth, 0.0f,  halfWidth, 1.0f), color }; // 3
	outVertices[baseVertex + 4] = { nsVector4(-halfWidth, 0.0f,  halfWidth, 1.0f), color }; // 4


	const int INDEX_COUNT = 18;
	const uint32 indices[INDEX_COUNT] =
	{
		// front
		0, 2, 1,

		// right
		0, 3, 2,

		// back
		0, 4, 3,

		// left,
		0, 1, 4,

		// bottom
		1, 2, 3, 3, 4, 1
	};

	const int baseIndex = outIndices.GetCount();
	outIndices.Resize(baseIndex + INDEX_COUNT);

	for (int i = 0; i < INDEX_COUNT; ++i)
	{
		outIndices[baseIndex + i] = baseVertex + indices[i];
	}
}


void nsGeometryFactory::AddPrimitiveTriangle_Arrow(nsTArray<nsVertexPrimitive>& outVertices, nsTArray<uint32>& outIndices, float shaftLength, float width, float tipHeight, nsColor shaftColor, nsColor arrowColor) noexcept
{
	NS_Assert(width > 0.0f);

	const float halfWidth = width * 0.5f;
	const float quarterWidth = halfWidth * 0.5f;
	AddPrimitiveTriangle_AABB(outVertices, outIndices, nsVector3(-quarterWidth, 0.0f, -quarterWidth), nsVector3(quarterWidth, shaftLength, quarterWidth), shaftColor);

	const int baseVertex = outVertices.GetCount();

	int vtxId = baseVertex;
	outVertices.Resize(vtxId + 5);
	outVertices[vtxId++] = { nsVector4(0.0f, shaftLength + tipHeight, 0.0f, 1.0f), arrowColor }; // 0
	outVertices[vtxId++] = { nsVector4(-halfWidth, shaftLength, -halfWidth, 1.0f), arrowColor }; // 1
	outVertices[vtxId++] = { nsVector4( halfWidth, shaftLength, -halfWidth, 1.0f), arrowColor }; // 2
	outVertices[vtxId++] = { nsVector4( halfWidth, shaftLength,  halfWidth, 1.0f), arrowColor }; // 3
	outVertices[vtxId++] = { nsVector4(-halfWidth, shaftLength,  halfWidth, 1.0f), arrowColor }; // 4


	const int INDEX_COUNT = 18;
	const uint32 indices[INDEX_COUNT] =
	{
		// front
		0, 2, 1,

		// right
		0, 3, 2,

		// back
		0, 4, 3,

		// left,
		0, 1, 4,

		// bottom
		1, 2, 3, 3, 4, 1
	};

	const int baseIndex = outIndices.GetCount();
	outIndices.Resize(baseIndex + INDEX_COUNT);

	for (int i = 0; i < INDEX_COUNT; ++i)
	{
		outIndices[baseIndex + i] = baseVertex + indices[i];
	}
}
