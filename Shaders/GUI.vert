#version 450


layout (location = 0) in vec2 IN_Position;
layout (location = 1) in vec2 IN_TexCoord;
layout (location = 2) in vec4 IN_Color;


layout (push_constant) uniform PC_Transform
{
	vec2 Scale;
	vec2 Translate;
}; 


layout (location = 0) out vec2 OUT_TexCoord;
layout (location = 1) out vec4 OUT_Color;



void main()
{
	gl_Position = vec4(IN_Position * Scale + Translate, 0.0, 1.0);
	OUT_TexCoord = IN_TexCoord;
	OUT_Color = IN_Color;
}
