#version 450

layout (location = 0) in vec4 IN_WorldPosition;
layout (location = 1) in vec4 IN_Color;

layout (set = 0, binding = 0) uniform UBO_Camera
{
	mat4 CameraView;
	mat4 CameraProjection;
	vec4 CameraWorldPosition;
};


layout (location = 0) out vec4 OUT_Color;



void main()
{
	gl_Position = CameraProjection * CameraView * IN_WorldPosition;
	OUT_Color = IN_Color;
}
