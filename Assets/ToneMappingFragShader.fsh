#version 440 core
#define FLT_MAX 3.402823466e+38

uniform highp sampler3D LUTTexture;
uniform sampler2D TextureBefore;

uniform vec2 GrainRandomFull;

const float LUTSize = 32.0;
const vec2 TonemapperParams = vec2(0.4, 0.11);

in vec2 ScreenUV;
in vec3 ExposureScaleVignette;
out vec4 fragmentColor;

vec3 FDebugColor;

float frac(float x)
{
	return x - floor(x);
}

float GetGrain()
{
	vec2 GrainUV = mix(GrainRandomFull, 1.0 + GrainRandomFull, ScreenUV);
	return frac(sin(GrainUV.x + GrainUV.y * 543.31) * 493013.0);	
}

float Luminance(vec3 LinearColor)
{
	return dot(LinearColor,  vec3(0.3, 0.59, 0.11));
}

float max4(vec4 x)
{
	return max(max(x.r, x.g), max(x.b, x.a));
}

float rcp(float s)
{
	if(s != 0.0)
	{
		return 1.0 / s;
	}
	else
	{
		return FLT_MAX;	
	}
}

float ComputeVignetteMask(vec2 VignetteCircleSpacePos, float Intensity)
{


	VignetteCircleSpacePos *= Intensity;
	float Tan2Angle = dot( VignetteCircleSpacePos, VignetteCircleSpacePos );
	float Cos4Angle = pow(rcp( Tan2Angle + 1.0) ,2.0);
	return Cos4Angle;
}

vec3 LogToLin(vec3 LogColor)
{
	const float LinearRange = 14.0;
	const float LinearGrey = 0.18;
	const float ExposureGrey = 444.0;
	vec3 LinearColor = exp2((LogColor - ExposureGrey / 1023.0) * LinearRange) * LinearGrey;
	return LinearColor;
}

vec3 LinToLog(vec3 LinearColor)
{
	const float LinearRange = 14.0;
	const float LinearGrey = 0.18;
	const float ExposureGrey = 444.0;
	vec3 LogColor = log2(LinearColor) / LinearRange - log2(LinearGrey) / LinearRange + ExposureGrey / 1023.0;
	LogColor = clamp(LogColor, 0.0, 1.0);
	return LogColor;
}

vec3  ColorLookupTable(vec3 LinearColor)
{
	vec3 LUTEncodedColor;
	LUTEncodedColor = LinToLog(LinearColor + LogToLin(vec3(0.0)));
	vec3 UVW = LUTEncodedColor * ((LUTSize - 1.0) / LUTSize) + (0.5f / LUTSize);
	vec3  OutDeviceColor = texture(LUTTexture, UVW).rgb;
	return OutDeviceColor * 1.05;
}

void main()
{
	const float OneOverPreExposure = 1.0f;
	
	float Grain = GetGrain();
	vec4 SceneColor = texture(TextureBefore, ScreenUV);
	float ExposureScale = 1.0;
	
	float SharpenMultiplierDiv6 = TonemapperParams.y;
	
	vec3 C1 = textureOffset(TextureBefore, ScreenUV, ivec2(-1,  0)).rgb;
	vec3 C2 = textureOffset(TextureBefore, ScreenUV, ivec2( 1,  0)).rgb;
	vec3 C3 = textureOffset(TextureBefore, ScreenUV, ivec2(-1, -1)).rgb;
	vec3 C4 = textureOffset(TextureBefore, ScreenUV, ivec2(-1,  1)).rgb;

	float  A0 = Luminance(SceneColor.rgb);
	vec4  LuminanceNeightbors =  vec4(Luminance(C1), Luminance(C2), Luminance(C3), Luminance(C4));
	
	float  HDREdge = ExposureScale * max4(abs(A0 - LuminanceNeightbors));
	float  EdgeMask = clamp(1.0f - HDREdge, 0.0, 1.0);

	float LerpFactor = -EdgeMask * SharpenMultiplierDiv6;
	vec3  DeltaColor = (C1 + C2 + C3 + C4) - SceneColor.rgb * 4.0;
	SceneColor.rgb += DeltaColor * LerpFactor;

	/*Handle Bloom
	float3  LinearColor = SceneColor.rgb * ColorScale0.rgb;
	float2 BloomUV = clamp(UV, SceneBloomUVMinMax.xy, SceneBloomUVMinMax.zw);
	float4 CombinedBloom = Texture2DSample(PostprocessInput1, PostprocessInput1Sampler, BloomUV);
	float2 DirtViewportUV = (SvPosition.xy - float2(ViewportRect.xy)) * ViewportSize.zw;
	float3 BloomDirtMaskColor = Texture2DSample(BloomDirtMask_Mask, BloomDirtMask_MaskSampler, DirtViewportUV).rgb * BloomDirtMask_Tint.rgb;
	LinearColor += CombinedBloom.rgb * (ColorScale1.rgb + BloomDirtMaskColor);
	*/
	
	vec3  LinearColor = SceneColor.rgb;
	LinearColor *= ExposureScale;
	LinearColor.rgb *= ComputeVignetteMask(ExposureScaleVignette.yz, TonemapperParams.x);
	
	vec3  OutDeviceColor = ColorLookupTable(LinearColor);
	float  LuminanceForPostProcessAA = dot(OutDeviceColor,  vec3(0.299f, 0.587f, 0.114f));
	float  GrainQuantization = 1.0/256.0;
	float  GrainAdd = (Grain * GrainQuantization) + (-0.5 * GrainQuantization);
	OutDeviceColor.rgb += GrainAdd;
	vec4 OutColor = vec4(OutDeviceColor, clamp(LuminanceForPostProcessAA, 0.0, 1.0));
	
	fragmentColor = vec4(OutColor);
	//fragmentColor = vec4(abs(vec3(Grain)), 1.0);
	//fragmentColor = texture(TextureBefore, ScreenUV);
	//ragmentColor = vec4(ScreenUV, 1.0, 1.0);
}
