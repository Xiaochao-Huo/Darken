#version 440 core

precision mediump float;

uniform sampler2D MainTex;

in vec2 ScreenUV;

out vec4 fragmentColor;

void main()
{
	fragmentColor = texture(MainTex, ScreenUV);
}
