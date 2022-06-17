#version 450
#extension GL_EXT_nonuniform_qualifier : enable


layout (location = 0) in vec2 IN_TexCoord;
layout (location = 1) in vec4 IN_Color;


layout (set = 0, binding = 0) uniform sampler2D Textures[];

layout (push_constant) uniform PC_Texture
{
	layout (offset = 16) int Id;
};

layout (location = 0) out vec4 OUT_FragColor;



void main()
{
#ifdef FONT
	OUT_FragColor = vec4(IN_Color.rgb, texture(Textures[Id], IN_TexCoord).r * IN_Color.a);
#else
	OUT_FragColor = IN_Color * texture(Textures[Id], IN_TexCoord);
#endif
}
