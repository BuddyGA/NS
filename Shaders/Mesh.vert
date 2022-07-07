#version 450

layout (location = 0) in vec3 IN_Position;
layout (location = 1) in vec3 IN_Normal;
layout (location = 2) in vec3 IN_Tangent;
layout (location = 3) in vec2 IN_TexCoord;
layout (location = 4) in vec4 IN_Weights;
layout (location = 5) in uint IN_Joints;


layout (set = 2, binding = 0) readonly buffer SSBO_BoneTransform
{
	mat4 BoneTransforms[];
};


layout (set = 3, binding = 0) uniform UBO_Camera
{
	mat4 CameraView;
	mat4 CameraProjection;
	vec4 CameraWorldPosition;
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
	vec4 vertexWorldPosition = vec4(0.0);
	vec3 vertexNormalPosition = vec3(0.0);

	if (BoneTransformIndex == -1)
	{
		vertexWorldPosition = WorldTransform * vec4(IN_Position, 1.0);
		vertexNormalPosition = normalize(transpose(inverse(mat3(WorldTransform))) * IN_Normal);
	}
	else
	{
		uint boneId_0 = (IN_Joints & 0x000000FF);
		uint boneId_1 = (IN_Joints & 0x0000FF00) >> 8;
		uint boneId_2 = (IN_Joints & 0x00FF0000) >> 16;
		uint boneId_3 = (IN_Joints & 0xFF000000) >> 24;

		mat4 vertexSkinnedTransform = BoneTransforms[BoneTransformIndex + boneId_0] * IN_Weights.x;
		vertexSkinnedTransform += BoneTransforms[BoneTransformIndex + boneId_1] * IN_Weights.y;
		vertexSkinnedTransform += BoneTransforms[BoneTransformIndex + boneId_2] * IN_Weights.z;
		vertexSkinnedTransform += BoneTransforms[BoneTransformIndex + boneId_3] * IN_Weights.w;

		mat4 vertexWorldTransform = WorldTransform * vertexSkinnedTransform;
		vertexWorldPosition = vertexWorldTransform * vec4(IN_Position, 1.0);
		vertexNormalPosition = normalize(transpose(inverse(mat3(vertexWorldTransform))) * IN_Normal);
	}

	gl_Position = CameraProjection * CameraView * vertexWorldPosition;
	OUT_TexCoord = IN_TexCoord;
	OUT_WorldPosition = vertexWorldPosition.xyz;
	OUT_WorldNormal = vertexNormalPosition;
	OUT_ViewPosition = vec3(CameraView * vertexWorldPosition);
	OUT_CameraWorldPosition = CameraWorldPosition.xyz;
}
