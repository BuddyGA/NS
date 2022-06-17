#version 450


layout (location = 0) out vec2 OUT_TexCoord;


void main()
{
	OUT_TexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(OUT_TexCoord * 2.0 + -1.0, 0.0f, 1.0);
}
