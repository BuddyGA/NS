#version 450

layout (location = 0) in vec3 IN_Position;
layout (location = 1) in vec3 IN_Normal;
layout (location = 2) in vec3 IN_Tangent;
layout (location = 3) in vec2 IN_TexCoord;
layout (location = 4) in vec4 IN_BoneWeights;
layout (location = 5) in ivec4 IN_BoneIds;


layout (set = 1, binding = 0) uniform UBO_Camera
{
	mat4 CameraView;
	mat4 CameraProjection;
	vec4 CameraWorldPosition;
};


layout (set = 3, binding = 0) readonly buffer SSBO_BoneTransform
{
	mat4 BoneTransforms[];
};


layout (push_constant) uniform PC_Vertex
{
	mat4 WorldTransform;
	int BoneTransformIndex;
};



layout (location = 0) out vec2 OUT_TexCoord;
layout (location = 1) out vec3 OUT_WorldPosition;
layout (location = 2) out vec3 OUT_WorldNormal;
layout (location = 3) out vec3 OUT_ViewPosition;
layout (location = 4) out vec3 OUT_CameraWorldPosition;


void main()
{
	mat4 weightBoneTransform = BoneTransforms[BoneTransformIndex + IN_BoneIds.x] * IN_BoneWeights.x;
	weightBoneTransform += BoneTransforms[BoneTransformIndex + IN_BoneIds.y] * IN_BoneWeights.y;
	weightBoneTransform += BoneTransforms[BoneTransformIndex + IN_BoneIds.z] * IN_BoneWeights.z;
	weightBoneTransform += BoneTransforms[BoneTransformIndex + IN_BoneIds.w] * IN_BoneWeights.w;

	mat4 vertexWorldTransform = WorldTransform * weightBoneTransform;
	vec4 vertexWorldPosition = vertexWorldTransform * vec4(IN_Position, 1.0);

	gl_Position = CameraProjection * CameraView * vertexWorldPosition;
	OUT_TexCoord = IN_TexCoord;
	OUT_WorldPosition = vertexWorldPosition.xyz;
	OUT_WorldNormal = normalize(transpose(inverse(mat3(vertexWorldTransform))) * IN_Normal);
	OUT_ViewPosition = vec3(CameraView * vertexWorldPosition);
	OUT_CameraWorldPosition = CameraWorldPosition.xyz;
}
