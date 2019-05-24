#version 440 core

precision mediump float;

layout(location = 0) in vec4 inVertex;
layout(location = 1) in vec2 inTexcoord;
out vec2 ScreenUV;

void main()
{
	gl_Position = inVertex;
	ScreenUV = inTexcoord;
}
