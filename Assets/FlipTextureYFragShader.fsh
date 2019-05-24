#version 440 core

precision mediump float;

uniform sampler2D MainTex;
in vec2 ScreenUV;

#define PI 3.1415926535897932384626433832795
#define FLT_MAX 3.402823466e+38

out vec4 fragmentColor;

void main()
{
	fragmentColor = texture(MainTex, vec2(ScreenUV.x, 1.0 - ScreenUV.y));
}
