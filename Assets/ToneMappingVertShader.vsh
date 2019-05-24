#version 440 core

precision mediump float;

layout(location = 0) in vec4 inVertex;
layout(location = 1) in vec2 inTexcoord;

out vec2 ScreenUV;
out vec3 ExposureScaleVignette;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectMatrix;
uniform vec2 ScreenSize;


vec2 VignetteSpace(vec2 Pos)
{

	float Scale = sqrt(2.0) / sqrt(1.0 + (ScreenSize.y * (1.0 / ScreenSize.x)) * (ScreenSize.y * (1.0 / ScreenSize.x)));
	return Pos * vec2(1.0, ScreenSize.y * (1.0 / ScreenSize.x)) * Scale;
}

void main()
{
	//gl_Position = ProjectMatrix * ViewMatrix * ModelMatrix * inVertex;
	gl_Position = inVertex;
	ScreenUV = inTexcoord;	
	ExposureScaleVignette = vec3(1.0, VignetteSpace(inVertex.xy));
}
