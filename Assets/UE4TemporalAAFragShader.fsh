#version 440 core
#define FLT_MAX 3.402823466e+38

#define TAASingleTex

const float CurrentFrameWeight = 0.04;

uniform sampler2D ShadowDepth_Tex;
uniform sampler2D PreFrame;
uniform sampler2D CurrentFrame;
uniform sampler2D DepthTex;
uniform sampler2D VelocityTex;
uniform vec2 ScreenSize;
uniform vec4 ViewCamera_ZBufferParams;// x = near, y = far, z = fovInRadian, w = Aspect
uniform vec4 JitterUV;
uniform vec4 PlusWeights_0;
uniform vec4 PlusWeights_1;

in vec2 ScreenUV;
in vec2 ScreenUV_Pre;

#ifdef TAASingleTex
layout(location = 0) out vec4 fragmentColor;
#else

layout(location = 0) out vec4 HistoryBuffer;
layout(location = 1) out vec4 ToScreenBuffer;
layout(location = 2) out vec4 DebugTex;
#endif


ivec2 kOffset3x3[9];
int kPlusIndexes3x3[5];
vec3 FDebugColor;

float Linear01Depth(float z, float Near, float Far)
{
    //linearDepth = (2.0 * near) / (far + near - z * (far - near));
    float Depth = z * 2.0 - 1.0;
    return (2.0 * Near) / (Far + Near - Depth *(Far - Near));
}

float LinearEyeDepth(float z, float Near, float Far)
{
	return Linear01Depth(z, Near, Far) * Far;
}

vec3 RGBToYCoCg(vec3 RGB)
{
	float Y = dot(RGB, vec3(1, 2, 1));
	float Co = dot(RGB, vec3(2, 0, -2));
	float Cg = dot(RGB, vec3(-1, 2, -1));

	vec3 YCoCg = vec3( Y, Co, Cg );
	return YCoCg;
}

vec3 YCoCgToRGB(vec3 YCoCg)
{
	float Y = YCoCg.x * 0.25;
	float Co = YCoCg.y * 0.25;
	float Cg = YCoCg.z * 0.25;

	float R = Y + Co - Cg;
	float G = Y + Cg;
	float B = Y - Co - Cg;

	vec3 RGB = vec3(R, G, B);
	return RGB;
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

void CacheNeighborPixelColor(out vec4 CachedNeighbors[9])
{
	vec4 NeighborColor;
	NeighborColor = textureOffset(CurrentFrame, ScreenUV - JitterUV.xy, ivec2(-1, -1)); CachedNeighbors[0] = vec4(RGBToYCoCg(NeighborColor.rgb), NeighborColor.a);
	NeighborColor = textureOffset(CurrentFrame, ScreenUV - JitterUV.xy, ivec2( 0, -1)); CachedNeighbors[1] = vec4(RGBToYCoCg(NeighborColor.rgb), NeighborColor.a);
	NeighborColor = textureOffset(CurrentFrame, ScreenUV - JitterUV.xy, ivec2( 1, -1)); CachedNeighbors[2] = vec4(RGBToYCoCg(NeighborColor.rgb), NeighborColor.a);
	NeighborColor = textureOffset(CurrentFrame, ScreenUV - JitterUV.xy, ivec2(-1,  0)); CachedNeighbors[3] = vec4(RGBToYCoCg(NeighborColor.rgb), NeighborColor.a);
	NeighborColor = textureOffset(CurrentFrame, ScreenUV - JitterUV.xy, ivec2( 0,  0)); CachedNeighbors[4] = vec4(RGBToYCoCg(NeighborColor.rgb), NeighborColor.a);
	NeighborColor = textureOffset(CurrentFrame, ScreenUV - JitterUV.xy, ivec2( 1,  0)); CachedNeighbors[5] = vec4(RGBToYCoCg(NeighborColor.rgb), NeighborColor.a);
	NeighborColor = textureOffset(CurrentFrame, ScreenUV - JitterUV.xy, ivec2(-1,  1)); CachedNeighbors[6] = vec4(RGBToYCoCg(NeighborColor.rgb), NeighborColor.a);
	NeighborColor = textureOffset(CurrentFrame, ScreenUV - JitterUV.xy, ivec2( 0,  1)); CachedNeighbors[7] = vec4(RGBToYCoCg(NeighborColor.rgb), NeighborColor.a);
	NeighborColor = textureOffset(CurrentFrame, ScreenUV - JitterUV.xy, ivec2( 1,  1)); CachedNeighbors[8] = vec4(RGBToYCoCg(NeighborColor.rgb), NeighborColor.a);
}

vec4 FiltCurrentFrame(vec4 CachedNeighbors[9])
{
	int SampleIndexes[5] = kPlusIndexes3x3;

	float NeighborsFinalWeight = 0.0;
	vec4 ColorWithNeighbor = vec4(0.0);

	float PlusWeights[5];
	PlusWeights[0] = PlusWeights_0.x;
	PlusWeights[1] = PlusWeights_0.y;
	PlusWeights[2] = PlusWeights_0.z;
	PlusWeights[3] = PlusWeights_0.w;
	PlusWeights[4] = PlusWeights_1.x;
	
	for (int i = 0; i <  5 ; i++)
	{
		ivec2 SampleOffset;
		
		int SampleIndex = SampleIndexes[i];
		SampleOffset = kOffset3x3[SampleIndex];
	
		float SampleSpatialWeight = PlusWeights[i];

		vec4 SampleColor = CachedNeighbors[4 + SampleOffset.x + SampleOffset.y * 3];
		float SampleHdrWeight = rcp(SampleColor.x + 4.0);		
			
		float BilateralWeight = 1.0;
		float SampleFinalWeight = SampleSpatialWeight * SampleHdrWeight * BilateralWeight;

		ColorWithNeighbor += SampleFinalWeight * SampleColor;
		NeighborsFinalWeight += SampleFinalWeight;
	}
	
	vec4 FilteredColorCurrent = ColorWithNeighbor * rcp(NeighborsFinalWeight);
	
	return FilteredColorCurrent;
}

void ComputeNeighborhoodBoundingbox(vec4 CachedNeighbors[9], out vec4 NeighborMin, out vec4 NeighborMax)
{
	NeighborMin = vec4(min(CachedNeighbors[1].x, min(CachedNeighbors[3].x, min(CachedNeighbors[4].x, min(CachedNeighbors[5].x, CachedNeighbors[7].x)))),
					   min(CachedNeighbors[1].y, min(CachedNeighbors[3].y, min(CachedNeighbors[4].y, min(CachedNeighbors[5].y, CachedNeighbors[7].y)))),
					   min(CachedNeighbors[1].z, min(CachedNeighbors[3].z, min(CachedNeighbors[4].z, min(CachedNeighbors[5].z, CachedNeighbors[7].z)))),
					   min(CachedNeighbors[1].w, min(CachedNeighbors[3].w, min(CachedNeighbors[4].w, min(CachedNeighbors[5].w, CachedNeighbors[7].w))))	);
	NeighborMax = vec4(max(CachedNeighbors[1].x, max(CachedNeighbors[3].x, max(CachedNeighbors[4].x, max(CachedNeighbors[5].x, CachedNeighbors[7].x)))),
					   max(CachedNeighbors[1].y, max(CachedNeighbors[3].y, max(CachedNeighbors[4].y, max(CachedNeighbors[5].y, CachedNeighbors[7].y)))),
					   max(CachedNeighbors[1].z, max(CachedNeighbors[3].z, max(CachedNeighbors[4].z, max(CachedNeighbors[5].z, CachedNeighbors[7].z)))),
					   max(CachedNeighbors[1].w, max(CachedNeighbors[3].w, max(CachedNeighbors[4].w, max(CachedNeighbors[5].w, CachedNeighbors[7].w))))	);
}

struct FCatmullRomSamples
{
	int Count;
	ivec2 UVDir[5];
	vec2 UV[5];
	float Weight[5];
	float FinalMultiplier;
};

void Bicubic2DCatmullRom(vec2 UV, vec2 Size, vec2 InvSize, out vec2 Sample[3], out vec2 Weight[3])
{
	UV *= Size;

	vec2 tc = floor( UV - 0.5 ) + 0.5;
	vec2 f = UV - tc;
	vec2 f2 = f * f;
	vec2 f3 = f2 * f;

	vec2 w0 = f2 - 0.5 * (f3 + f);
	vec2 w1 = 1.5 * f3 - 2.5 * f2 + 1.0;
	vec2 w3 = 0.5 * (f3 - f2);
	vec2 w2 = 1.0 - w0 - w1 - w3;

	Weight[0] = w0;
	Weight[1] = w1 + w2;
	Weight[2] = w3;

	Sample[0] = tc - 1.0;
	Sample[1] = tc + w2 / Weight[1];
	Sample[2] = tc + 2.0;

	Sample[0] *= InvSize;
	Sample[1] *= InvSize;
	Sample[2] *= InvSize;
}

FCatmullRomSamples GetBicubic2DCatmullRomSamples(vec2 UV, vec2 Size, vec2 InvSize)
{
	FCatmullRomSamples Samples;
	Samples.Count =  5 ;

	vec2 Weight[3];
	vec2 Sample[3];
	Bicubic2DCatmullRom( UV, Size, InvSize, Sample, Weight );


	Samples.UV[0] = vec2(Sample[1].x, Sample[0].y);
	Samples.UV[1] = vec2(Sample[0].x, Sample[1].y);
	Samples.UV[2] = vec2(Sample[1].x, Sample[1].y);
	Samples.UV[3] = vec2(Sample[2].x, Sample[1].y);
	Samples.UV[4] = vec2(Sample[1].x, Sample[2].y);

	Samples.Weight[0] = Weight[1].x * Weight[0].y;
	Samples.Weight[1] = Weight[0].x * Weight[1].y;
	Samples.Weight[2] = Weight[1].x * Weight[1].y;
	Samples.Weight[3] = Weight[2].x * Weight[1].y;
	Samples.Weight[4] = Weight[1].x * Weight[2].y;

	Samples.UVDir[0] = ivec2(0, -1);
	Samples.UVDir[1] = ivec2(-1, 0);
	Samples.UVDir[2] = ivec2(0, 0);
	Samples.UVDir[3] = ivec2(1, 0);
	Samples.UVDir[4] = ivec2(0, 1);


	float CornerWeights;
	CornerWeights = Samples.Weight[0];
	CornerWeights += Samples.Weight[1];
	CornerWeights += Samples.Weight[2];
	CornerWeights += Samples.Weight[3];
	CornerWeights += Samples.Weight[4];
	Samples.FinalMultiplier = 1.0 / CornerWeights;

	return Samples;
}

vec4 GetHistoryColor(vec2 UV)
{
	return texture(PreFrame, UV);
}

vec4 SampleHistory(vec2 HistoryPositionNDC)
{
	vec2 HistoryUV = HistoryPositionNDC * 0.5 + 0.5;
	HistoryUV = clamp(HistoryUV, vec2(0.0), vec2(1.0));
	
	FCatmullRomSamples Samples = GetBicubic2DCatmullRomSamples(HistoryUV, ScreenSize, 1.0 / ScreenSize);
	
	vec4 HistoryColor = vec4(0.0);
	for (int i = 0; i < Samples.Count; i++)
	{
		vec2 SampleUV = Samples.UV[i];
		
		if (Samples.UVDir[i].x < 0)
		{
			SampleUV.x = max(SampleUV.x, 0.0);
		}
		else if (Samples.UVDir[i].x > 0)
		{
			SampleUV.x = min(SampleUV.x, 1.0);
		}

		if (Samples.UVDir[i].y < 0)
		{
			SampleUV.y = max(SampleUV.y, 0.0);
		}
		else if (Samples.UVDir[i].y > 0)
		{
			SampleUV.y = min(SampleUV.y, 1.0);
		}
	
		HistoryColor += GetHistoryColor(SampleUV) * Samples.Weight[i];
	}
	HistoryColor *= Samples.FinalMultiplier;
	
	return vec4(RGBToYCoCg(HistoryColor.rgb), 0.0);
}

vec2 WeightedLerpFactors(float WeightA, float WeightB, float Blend)
{
	float BlendA = (1.0 - Blend) * WeightA;
	float BlendB = Blend * WeightB;
	float RcpBlend = rcp(BlendA + BlendB);
	BlendA *= RcpBlend;
	BlendB *= RcpBlend;
	return vec2(BlendA, BlendB);
}

vec3 LinearTosRGB(vec3 LinearColor)
{
 return (LinearColor.r < 0.0031308 && LinearColor.g < 0.0031308 && LinearColor.b < 0.0031308) ? 12.92 * LinearColor.rgb : 1.055 * pow(LinearColor.rgb, vec3(1.0 / 2.4)) - vec3(0.055, 0.055, 0.055);
}

void main()
{
	kOffset3x3[0] = ivec2(-1, -1);
	kOffset3x3[1] = ivec2( 0, -1);
	kOffset3x3[2] = ivec2( 1, -1);
	kOffset3x3[3] = ivec2(-1,  0);
	kOffset3x3[4] = ivec2( 0,  0);
	kOffset3x3[5] = ivec2( 1,  0);
	kOffset3x3[6] = ivec2(-1,  1);
	kOffset3x3[7] = ivec2( 0,  1);
	kOffset3x3[8] = ivec2( 1,  1);
	
	kPlusIndexes3x3[0] = 1;
	kPlusIndexes3x3[1] = 3;
	kPlusIndexes3x3[2] = 4;
	kPlusIndexes3x3[3] = 5;
	kPlusIndexes3x3[4] = 7;

	vec3 PositionNDC = vec3(0.0);
	PositionNDC.xy = ScreenUV.xy * 2.0 - 1.0;
	PositionNDC.z = Linear01Depth(texture(DepthTex, ScreenUV).r, ViewCamera_ZBufferParams.x, ViewCamera_ZBufferParams.y);
	
	vec2 VelocityOffset = vec2(0.0);
	vec4 Depths;
	
	Depths.x = Linear01Depth(textureOffset(DepthTex, ScreenUV, ivec2(-2, -2)).r, ViewCamera_ZBufferParams.x, ViewCamera_ZBufferParams.y);
	Depths.y = Linear01Depth(textureOffset(DepthTex, ScreenUV, ivec2( 2, -2)).r, ViewCamera_ZBufferParams.x, ViewCamera_ZBufferParams.y);
	Depths.z = Linear01Depth(textureOffset(DepthTex, ScreenUV, ivec2(-2,  2)).r, ViewCamera_ZBufferParams.x, ViewCamera_ZBufferParams.y);
	Depths.w = Linear01Depth(textureOffset(DepthTex, ScreenUV, ivec2( 2,  2)).r, ViewCamera_ZBufferParams.x, ViewCamera_ZBufferParams.y);
	
	vec2 DepthOffset = vec2(2.0, 2.0);
	float DepthOffsetXx = 2.0;
	
	if(Depths.x < Depths.y)
	{
		DepthOffsetXx = -2.0;
	}
	if(Depths.z > Depths.w)
	{
		DepthOffset.x = -2.0;
	}
	
	float DepthsXY = min(Depths.x, Depths.y);
	float DepthsZW = min(Depths.z, Depths.w);
	if(DepthsXY < DepthsZW)
	{
		DepthOffset.y = -2.0 ;
		DepthOffset.x = DepthOffsetXx;
	}
	
	float DepthsXYZW = min(DepthsXY, DepthsZW);
	if(DepthsXYZW < PositionNDC.z)
	{
		VelocityOffset = DepthOffset / ScreenSize;
		PositionNDC.z = DepthsXYZW;
	}
	
	bool OffScreen = false;	
	vec2 VelocityNDC;
	VelocityNDC = texture(VelocityTex, ScreenUV  + VelocityOffset).xy * 2.0;

	vec2 BackTemp = VelocityNDC * ScreenSize;
	
	float Velocity = sqrt(dot(BackTemp, BackTemp));
	
	vec2 HistoryPositionNDC = PositionNDC.xy  - VelocityNDC ;
	
	//Check whether Pixel is out of Screen
	OffScreen = max(abs(HistoryPositionNDC.x), abs(HistoryPositionNDC.y)) >= 1.0;
	
	//CacheNeighbor Pixel Color
	vec4 CachedNeighbors[9];
	CacheNeighborPixelColor(CachedNeighbors);
	
	//Filt Current Pixel	
	vec4 FilteredColorCurrent;
	FilteredColorCurrent = FiltCurrentFrame(CachedNeighbors);
	
	vec2 TestPos = abs(PositionNDC.xy) + 1.0 / ScreenSize * 2.0;
	if(max(TestPos.x, TestPos.y) >= 1.0)
	{
		FilteredColorCurrent = CachedNeighbors[4];
	}
	FilteredColorCurrent = CachedNeighbors[4];
	
	//ComputeNeighborhoodBoundingbox
	vec4 NeighborMin;
	vec4 NeighborMax;
	ComputeNeighborhoodBoundingbox(CachedNeighbors, NeighborMin, NeighborMax);
	
	//SampleHistory
	vec4 HistoryColor = SampleHistory(HistoryPositionNDC);
	HistoryColor.a = 0.0;
	
	bool IgnoreHistory = OffScreen;
		
	bool Dynamic1 = abs(textureOffset(VelocityTex, ScreenUV, ivec2( 0, -1)).x) > 0.0;
	bool Dynamic3 = abs(textureOffset(VelocityTex, ScreenUV, ivec2(-1,  0)).x) > 0.0;
	bool Dynamic4 = abs(textureOffset(VelocityTex, ScreenUV, ivec2( 0,  0)).x) > 0.0;
	bool Dynamic5 = abs(textureOffset(VelocityTex, ScreenUV, ivec2( 1,  0)).x) > 0.0;
	bool Dynamic7 = abs(textureOffset(VelocityTex, ScreenUV, ivec2( 0,  1)).x) > 0.0;

	bool Dynamic = Dynamic1 || Dynamic3 || Dynamic4 || Dynamic5 || Dynamic7;
	IgnoreHistory = IgnoreHistory || !Dynamic && HistoryColor.a > 0.0;
	
	HistoryColor = clamp(HistoryColor, NeighborMin, NeighborMax);
	
	float LumaFiltered = FilteredColorCurrent.x;
	float LumaHistory = HistoryColor.x;
	
	float BlendFinal = CurrentFrameWeight;
	BlendFinal = mix(BlendFinal, 0.2, clamp(Velocity / 40.0, 0.0, 1.0));
	BlendFinal = max(BlendFinal, clamp(0.01 * LumaHistory / abs(LumaFiltered - LumaHistory), 0.0, 1.0));
	
	if(IgnoreHistory)
	{
		HistoryColor = FilteredColorCurrent;
	}
	
	float FilteredColorWeight = rcp(FilteredColorCurrent.x + 4.0);
	float HistoryColorWeight = rcp(HistoryColor.x + 4.0);
	
	vec2 Weights = WeightedLerpFactors(HistoryColorWeight, FilteredColorWeight, BlendFinal);
	
	vec4 ColorOut = HistoryColor * Weights.x + FilteredColorCurrent * Weights.y;
	//ColorOut = FilteredColorCurrent;
	
	ColorOut.rgb = YCoCgToRGB(ColorOut.rgb);
	ColorOut.a =  1.0;
	#ifdef TAASingleTex
	fragmentColor = ColorOut;// texture(CurrentFrame, ScreenUV);
	#else
	HistoryBuffer = texture(CurrentFrame, ScreenUV);
	ToScreenBuffer = texture(CurrentFrame, ScreenUV);
	#endif
	//ToScreenBuffer = ColorOut;
	
	
	//DebugTex = abs(texture(VelocityTex, ScreenUV));

}
