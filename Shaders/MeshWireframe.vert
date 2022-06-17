#version 450

layout (location = 0) in vec3 IN_Position;


layout (set = 0, binding = 0) uniform UBO_Camera
{
	mat4 CameraView;
	mat4 CameraProjection;
	vec4 CameraWorldPosition;
};


layout (push_constant) uniform PC_WorldTransform
{
	mat4 WorldTransform;
};


layout (location = 0) out vec4 OUT_Color;


void main()
{
	vec4 vertexWorldPosition = WorldTransform * vec4(IN_Position, 1.0);

	gl_Position = CameraProjection * CameraView * vertexWorldPosition;
	OUT_Color = vec4(0.3, 0.3, 0.3, 1.0);
}
