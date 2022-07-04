#include "nsRenderPrimitiveBatch.h"
#include "nsGeometryFactory.h"



nsRenderPrimitiveBatch::nsRenderPrimitiveBatch()
{

}


void nsRenderPrimitiveBatch::UpdateIndicesAtVertexOffset(int vertexOffset)
{
	for (int i = 0; i < Indices.GetCount(); ++i)
	{
		Indices[i] += vertexOffset;
	}
}




nsRenderPrimitiveBatchLine::nsRenderPrimitiveBatchLine()
{
}


void nsRenderPrimitiveBatchLine::AddLine(const nsVector3& start, const nsVector3& end, nsColor color)
{
	nsGeometryFactory::AddPrimitiveLine(Vertices, Indices, start, end, color);
}


void nsRenderPrimitiveBatchLine::AddBoxAABB(const nsVector3& boxMin, const nsVector3& boxMax, nsColor color)
{
	nsGeometryFactory::AddPrimitiveLine_AABB(Vertices, Indices, boxMin, boxMax, color);
}


void nsRenderPrimitiveBatchLine::AddCircle(const nsVector3& center, float radius, float halfArcRadian, nsEAxisType arcAxis, nsColor color)
{
	nsGeometryFactory::AddPrimitiveLine_Circle(Vertices, Indices, center, radius, halfArcRadian, arcAxis, color);
}


void nsRenderPrimitiveBatchLine::AddCircleAroundAxis(const nsVector3& center, float radius, const nsVector3& axis, float halfArcRadian, nsColor color)
{
	const int baseVertex = Vertices.GetCount();
	nsGeometryFactory::AddPrimitiveLine_Circle(Vertices, Indices, nsVector3::ZERO, radius, halfArcRadian, nsEAxisType::Y_Axis, color);

	const nsMatrix4 worldMatrix = nsMatrix4::RotationQuat(nsQuaternion::FromVectors(nsVector3::UP, axis.GetNormalized())) * nsMatrix4::Translation(center);

	for (int v = baseVertex; v < Vertices.GetCount(); ++v)
	{
		const nsVector4 vtxPosition = Vertices[v].Position;
		Vertices[v].Position = vtxPosition * worldMatrix;
	}
}




nsRenderPrimitiveBatchMesh::nsRenderPrimitiveBatchMesh()
{
}


void nsRenderPrimitiveBatchMesh::AddBoxAABB(const nsVector3& boxMin, const nsVector3& boxMax, nsColor color)
{
	nsGeometryFactory::AddPrimitiveTriangle_AABB(Vertices, Indices, boxMin, boxMax, color);
}


void nsRenderPrimitiveBatchMesh::AddBoxOBB(const nsVector3& center, const nsVector3& halfExtent, const nsQuaternion& rotation, nsColor color)
{
	nsGeometryFactory::AddPrimitiveTriangle_OBB(Vertices, Indices, center, rotation, halfExtent, color);
}


void nsRenderPrimitiveBatchMesh::AddPrism(const nsVector3& position, const nsQuaternion& rotation, float width, float height, nsColor color)
{
	const uint32 baseVertex = Vertices.GetCount();
	nsGeometryFactory::AddPrimitiveTriangle_Prism(Vertices, Indices, width, height, color);

	const nsMatrix4 worldMatrix = nsMatrix4::RotationQuat(rotation) * nsMatrix4::Translation(position);

	for (int v = baseVertex; v < Vertices.GetCount(); ++v)
	{
		const nsVector4 vtxPosition = Vertices[v].Position;
		Vertices[v].Position = vtxPosition * worldMatrix;
	}
}
