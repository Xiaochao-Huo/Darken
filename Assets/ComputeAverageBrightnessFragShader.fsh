#version 440 core

precision mediump float;

uniform samplerCube ReflectionTex;

in vec2 ScreenUV;

out vec4 AverageBrightness;

vec4 EncodeBrightness(float Brightness)
{
	float r = floor(Brightness * 10.0);
	float g = floor(Brightness * 100.0) - r * 10.0;
	float b = floor(Brightness * 1000.0) - g * 100.0 - r * 10.0;
	float a = floor(Brightness * 10000.0) - b * 1000.0 - g * 100.0 - r * 10.0;
	return vec4(r, g, b, a) / 10.0;
}

void main()
{
	ivec2 TexSize = textureSize(ReflectionTex, 0);
    vec3 AverageColor = vec3(0.0);
	AverageColor += texture(ReflectionTex, vec3( 1.0,  0.0,  0.0)).rgb;
	AverageColor += texture(ReflectionTex, vec3(-1.0,  0.0,  0.0)).rgb;
	AverageColor += texture(ReflectionTex, vec3( 0.0,  1.0,  0.0)).rgb;
	AverageColor += texture(ReflectionTex, vec3( 0.0, -1.0,  0.0)).rgb;
	AverageColor += texture(ReflectionTex, vec3( 0.0,  0.0,  1.0)).rgb;
	AverageColor += texture(ReflectionTex, vec3( 0.0,  0.0, -1.0)).rgb;
	float Brightness = dot(AverageColor / 6.0, vec3(0.33333));
	AverageBrightness = EncodeBrightness(Brightness);
}
