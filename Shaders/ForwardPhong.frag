#version 450
#extension GL_EXT_nonuniform_qualifier : enable


layout (location = 0) in vec2 IN_TexCoord;
layout (location = 1) in vec3 IN_FragWorldPosition;
layout (location = 2) in vec3 IN_FragWorldNormal;
layout (location = 3) in vec3 IN_FragViewPosition;
layout (location = 4) in vec3 IN_CameraWorldPosition;


layout (set = 0, binding = 0) uniform sampler2D Textures[];


layout (set = 1, binding = 0) uniform UBO_Environment
{
	mat4 DirectionalLightViewProjection;
	vec4 DirectionalLightDirection;
	vec4 DirectionalLightColorIntensity; // RGB = Color, A = Intensity;
	vec4 AmbientColorStrength;	// RGB = Color, A = Strength
};


layout (set = 3, binding = 0) readonly buffer UBO_Material
{
	vec4 BaseColor;
	float Shininess;
	int DiffuseTextureId;
	int SpecularTextureId;
};


layout (location = 0) out vec4 OUT_FragColor;



void main()
{

#ifdef CHECKER
	vec4 BaseColor = vec4(1.0);
	float Shininess = 32.0;
	vec2 scaledUV = IN_TexCoord * 2.0;
	vec2 checker = mod(floor(scaledUV), 2.0);
	float value = mod(checker.x + checker.y, 2.0) < 1.0 ? 0.5 : 0.25;
	vec3 diffuseColor = vec3(1.0) * value;
	vec3 specularColor = vec3(0.5);

#else
	vec3 diffuseColor = texture(Textures[DiffuseTextureId], IN_TexCoord).rgb;
	vec3 specularColor = texture(Textures[SpecularTextureId], IN_TexCoord).rrr;

#endif // CHECKER


	vec3 color = vec3(0.0);

	// Ambient
	color += (diffuseColor * AmbientColorStrength.rgb * AmbientColorStrength.a);

	vec3 normal = normalize(IN_FragWorldNormal);
	vec3 toViewDir = normalize(IN_CameraWorldPosition - IN_FragWorldPosition);


	// Directional light
	{
		vec3 toLightDir = normalize(-DirectionalLightDirection.xyz);

		float diffuseFactor = max(0.0, dot(normal, toLightDir));
		color += (diffuseColor * DirectionalLightColorIntensity.rgb * diffuseFactor);

		vec3 halfwayDir = normalize(toLightDir + toViewDir);
		float specularFactor = pow(max(0.0, dot(normal, halfwayDir)), Shininess);
		color += (specularColor * DirectionalLightColorIntensity.rgb * specularFactor);
	}


	// TODO: Point lights

	// TOOD: Spot lights


	OUT_FragColor = vec4(BaseColor.rgb * color, 1.0);
}
