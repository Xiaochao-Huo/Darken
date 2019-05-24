#version 440 core

precision mediump float;

layout (location = 0) in vec3 inVertex;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBinormal;
layout (location = 4) in vec2 inUV0;
layout (location = 5) in vec2 inUV1;
layout (location = 6) in vec2 inUV2;

layout(std140) uniform Model
{
uniform	mat4 ModelMatrix;
uniform	mat4 ModelMatrix_IT;
uniform	mat4 ModelMatrix_PreFrame;
uniform	mat4 ModelMatrix_IT_PreFrame;
};

layout(std140) uniform View
{	
	mat4 ViewMatrix;
	mat4 ProjectMatrix;
	
	mat4 ViewMatrix_PreFrame;
	mat4 ProjectMatrix_PreFrame;

	vec3 ViewPosition;
	vec2 ScreenSize;
};

uniform	mat4 LightSpaceVPMatrix;

out float ClipDepth;

void main()
{
	gl_Position = LightSpaceVPMatrix * ModelMatrix * vec4(inVertex, 1.0);
	ClipDepth = gl_Position.z / gl_Position.w;
}
