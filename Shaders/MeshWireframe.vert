#version 450

layout (location = 0) in vec3 IN_Position;
layout (location = 1) in vec4 IN_Weights;
layout (location = 2) in uint IN_Joints;


layout (set = 0, binding = 0) readonly buffer SSBO_BoneTransform
{
	mat4 BoneTransforms[];
};


layout (set = 1, binding = 0) uniform UBO_Camera
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


layout (location = 0) out vec4 OUT_Color;


void main()
{
	vec4 vertexWorldPosition = vec4(0.0);

	if (BoneTransformIndex == -1)
	{
		vertexWorldPosition = WorldTransform * vec4(IN_Position, 1.0);
	}
	else
	{
		uint boneId_0 = (IN_Joints & 0x000000FF);
		uint boneId_1 = (IN_Joints & 0x0000FF00) >> 8;
		uint boneId_2 = (IN_Joints & 0x00FF0000) >> 16;
		uint boneId_3 = (IN_Joints & 0xFF000000) >> 24;

		mat4 vertexBoneTransform = BoneTransforms[BoneTransformIndex + boneId_0] * IN_Weights.x;
		vertexBoneTransform += BoneTransforms[BoneTransformIndex + boneId_1] * IN_Weights.y;
		vertexBoneTransform += BoneTransforms[BoneTransformIndex + boneId_2] * IN_Weights.z;
		vertexBoneTransform += BoneTransforms[BoneTransformIndex + boneId_3] * IN_Weights.w;

		mat4 vertexWorldTransform = WorldTransform * vertexBoneTransform;
		vertexWorldPosition = vertexWorldTransform * vec4(IN_Position, 1.0);
	}

	gl_Position = CameraProjection * CameraView * vertexWorldPosition;
	OUT_Color = vec4(0.3, 0.3, 0.3, 1.0);
}
