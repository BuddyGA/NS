#version 450


layout (location = 0) in vec4 IN_Color;


layout (location = 0) out vec4 OUT_FragColor;


void main()
{	
	OUT_FragColor = IN_Color;
}
