#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) in vec2 IN_TexCoord;

layout (set = 0, binding = 0) uniform sampler2D Textures[];

layout (push_constant) uniform PC_Texture
{
	int Id;
};


layout (location = 0) out vec4 OUT_FragColor;


/*
#ifdef TEXTURE_DEPTH
layout (push_constant) uniform FragPushConstant
{
	float NearClip;
	float FarClip;
};
#endif // TEXTURE_DEPTH
*/


void main()
{
#ifdef TEXTURE_DEPTH
	//float depth = texture(Texture, IN_TexCoord).r;
	//float value = (FarClip * NearClip) / (FarClip + depth * (FarClip - NearClip) );
	//OUT_FragColor = vec4(vec3(value), 1.0);
	OUT_FragColor = vec4(vec3(texture(Textures[Id], IN_TexCoord).r), 1.0);

#else
	OUT_FragColor = texture(Textures[Id], IN_TexCoord);

#endif // TEXTURE_DEPTH
}
