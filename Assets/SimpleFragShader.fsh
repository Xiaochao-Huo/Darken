#version 440 core

precision mediump float;

uniform vec3 ColorTest;
uniform int bReflect;

#define PI 3.1415926535897932384626433832795
#define FLT_MAX 3.402823466e+38

in vec4 ClipSpacePos_Curr;
in vec4 ClipSpacePos_Pre;
in vec2 VelocityScreen;
in vec2 TexCoord0;
in vec2 TexCoord1;
in vec2 TexCoord2;
in vec2 SVPos;
in vec3 CameraPositionWorld;
in vec3 ViewDirWorld;
in vec3 PositionWorld;
in float Depth;
in mat3 TangentToWorldMatrix;
in mat4 FProjectMatrix;
in vec3 VDebugColor;
in vec3 NormalWorld;
vec3 FDebugColor;

//out vec4 fragmentColor;

layout(location = 0) out vec4 fragmentColor;
layout(location = 1) out float ScreenDepth;
layout(location = 2) out vec2 ScreenVelocity;
layout(location = 3) out vec4 BaseColor_Out;
layout(location = 4) out vec4 CustomData_Out;

////For ShadowMapping////
uniform	sampler2D ShadowDepth_Tex2D;
uniform samplerCube	ShadowDepth_TexCube;
layout(std140) uniform Shadow
{
uniform	mat4 LightSpaceVPMatrix;
uniform	vec4 ShadowBufferSize;
uniform	float SoftTransitionScale;
uniform	vec2 ProjectionDepthBiasParameters;
uniform	vec4 LightCamera_ZBufferParams; // x = near, y = far, z = fov, w = Aspect
uniform	int bDirectLight;
};

vec2 DiscSamples29[29];

uniform mat4 PointLightSpaceVPMatrices[6];

////For Lighting////
layout(std140) uniform LightBuffer
{
uniform	vec3 LightPosition;
uniform	float LightInvRadius;
uniform	vec3 LightColor;
uniform	float LightFallofExponent;
uniform	vec3 NormalizedLightDirection;
uniform	vec3 NormalizedLightTangent;
uniform	vec2 LightSpotAngles;
uniform	float LightSourceRadius;
uniform	float LightSourceLength;
uniform	float LightSoftSourceRadius;
uniform	float LightSpecularScale;
uniform	float LightContactShadowLength;
uniform	vec2 LightDistanceFadeMAD;
uniform	vec4 LightShadowMapChannelMask;
uniform	int LightShadowedBits;
uniform	int LightType;
uniform	int bLastLight;
};

struct GBufferData
{
	vec3 WorldNormal;
	vec3 DiffuseColor;
	vec3 SpecularColor;
	vec3 BaseColor;
	float Metallic;
	float Specular;
	vec4 CustomData;
	float IndirectIrradiance;
	vec4 PrecomputedShadowFactors;
	float Roughness;
	float GBufferAO;
	int ShadingModelID;
	int SelectiveOutputMask;
	float PerObjectGBufferData;
	float CustomDepth;
	int CustomStencil;
	float Depth;
	vec4 Velocity;
	vec3 StoredBaseColor;
	float StoredSpecular;
	float StoredMetallic;
};

//For Reflection
uniform samplerCube ReflectionTex;
uniform sampler2D PreIntegratedGF;
uniform float AverageBrightness;
uniform float Brightness;
uniform vec4 CapturePositionAndInfluenceRadius;


float Luminance(vec3 linearColor)
{
	return dot(linearColor,  vec3(0.3, 0.59, 0.11));
}

vec3 GetOffSpecularPeakReflectionDir(vec3 Normal, vec3 ReflectionVector, float Roughness)
{
	float a = Roughness * Roughness;
	return mix(Normal, ReflectionVector, (1.0 - a) * (sqrt(1.0 - a) + a));
}

float GetSpecularOcclusion(float NoV, float RoughnessSq, float AO)
{
	return clamp(pow(NoV + AO, RoughnessSq) - 1.0 + AO, 0.0, 1.0);
}

vec3 GetLookupVectorForSphereCapture(vec3 ReflectionVector, vec3 WorldPosition, vec4 SphereCapturePositionAndRadius, float NormalizedDistanceToCapture, vec3 LocalCaptureOffset, inout float DistanceAlpha)
{
	vec3 ProjectedCaptureVector = ReflectionVector;
	float ProjectionSphereRadius = SphereCapturePositionAndRadius.w;
	float SphereRadiusSquared = ProjectionSphereRadius * ProjectionSphereRadius;

	vec3 LocalPosition = WorldPosition - SphereCapturePositionAndRadius.xyz;
	float LocalPositionSqr = dot(LocalPosition, LocalPosition);

	vec3 QuadraticCoef;
	QuadraticCoef.x = 1.0;
	QuadraticCoef.y = dot(ReflectionVector, LocalPosition);
	QuadraticCoef.z = LocalPositionSqr - SphereRadiusSquared;

	float Determinant = QuadraticCoef.y * QuadraticCoef.y - QuadraticCoef.z;

	if (Determinant >= 0.0)
	{
		float FarIntersection = sqrt(Determinant) - QuadraticCoef.y;

		vec3 LocalIntersectionPosition = LocalPosition + FarIntersection * ReflectionVector;
		ProjectedCaptureVector = LocalIntersectionPosition - LocalCaptureOffset;

		float x = clamp( 2.5 * NormalizedDistanceToCapture - 1.5, 0.0, 1.0 );
		DistanceAlpha = 1.0 - x*x*(3.0 - 2.0*x);
	}
	return ProjectedCaptureVector;
}

vec3  EnvBRDF(vec3 SpecularColor, float Roughness, float NoV)
{
	vec2 AB = texture(PreIntegratedGF, vec2( NoV, Roughness )).rg;
	vec3 GF = SpecularColor * AB.x + clamp(50.0 * SpecularColor.g, 0.0, 1.0) * AB.y;
	return GF;
}

vec3 CompositeReflectionCapturesAndSkylight(
	float CompositeAlpha,
	vec3 WorldPosition,
	vec3 RayDirection,
	float Roughness,
	float IndirectIrradiance,
	float IndirectSpecularOcclusion,
	vec3 ExtraIndirectSpecular,
	int EyeIndex)
{
	vec4 ImageBasedReflections = vec4(0.0, 0.0, 0.0, CompositeAlpha);
	vec2 CompositedAverageBrightness = vec2(0.0f, 1.0f);

	float CaptureProperties = Brightness;

	vec3 CaptureVector = WorldPosition - CapturePositionAndInfluenceRadius.xyz;
	float CaptureVectorLength = sqrt(dot(CaptureVector, CaptureVector));
	float NormalizedDistanceToCapture = clamp(CaptureVectorLength / CapturePositionAndInfluenceRadius.w, 0.0, 1.0);

	if (CaptureVectorLength < CapturePositionAndInfluenceRadius.w)
	{
		vec3 ProjectedCaptureVector = RayDirection;
		float AvgBrightness = AverageBrightness;
		float DistanceAlpha = 0.0;

		ProjectedCaptureVector = GetLookupVectorForSphereCapture(RayDirection, WorldPosition, CapturePositionAndInfluenceRadius, NormalizedDistanceToCapture, vec3(0.0), DistanceAlpha);

		vec4 Sample = texture(ReflectionTex, ProjectedCaptureVector);

		Sample.rgb *= CaptureProperties;
		Sample *= DistanceAlpha;


		ImageBasedReflections.rgb += Sample.rgb * ImageBasedReflections.a * IndirectSpecularOcclusion;
		ImageBasedReflections.a *= (1.0 - Sample.a);

		CompositedAverageBrightness.x += AvgBrightness * DistanceAlpha * CompositedAverageBrightness.y;
		CompositedAverageBrightness.y *= 1.0 - DistanceAlpha;
	}

	CompositedAverageBrightness.x *= Luminance(vec3(1.0));

	ImageBasedReflections.rgb *= 1.0;


	ImageBasedReflections.rgb += ImageBasedReflections.a * ExtraIndirectSpecular;

	return ImageBasedReflections.rgb;
}

vec3 GatherRadiance(float CompositeAlpha, vec3 WorldPosition, vec3 RayDirection, float Roughness, vec3 BentNormal, float IndirectIrradiance, int ShadingModelID)
{

	float IndirectSpecularOcclusion = 1.0f;
	vec3 ExtraIndirectSpecular = vec3(0.0, 0.0, 0.0);

	return CompositeReflectionCapturesAndSkylight(CompositeAlpha, WorldPosition, RayDirection, Roughness, IndirectIrradiance, IndirectSpecularOcclusion, ExtraIndirectSpecular, 0);
}

vec4 CalculateReflection(GBufferData GBuffer)
{
	vec4 Color = vec4(0.0, 0.0, 0.0, 1.0);

	vec3 WorldPosition = PositionWorld;
	vec3 CameraToPixel = normalize(WorldPosition - CameraPositionWorld);
	float IndirectIrradiance = GBuffer.IndirectIrradiance;

	vec3 N = GBuffer.WorldNormal;
	vec3 V = -CameraToPixel;
	vec3 R = 2.0 * dot( V, N ) * N - V;
	
	float NoV = clamp(dot( N, V ), 0.0, 1.0);

	R = GetOffSpecularPeakReflectionDir(N, R, GBuffer.Roughness);

	/*float4 SSR = Texture2DSample( ScreenSpaceReflectionsTexture, ScreenSpaceReflectionsSampler, ScreenUV );
	Color.rgb = SSR.rgb;
	Color.a = 1 - SSR.a;*/

	float AO = GBuffer.GBufferAO;
	float RoughnessSq = GBuffer.Roughness * GBuffer.Roughness;
	float SpecularOcclusion = GetSpecularOcclusion(NoV, RoughnessSq, AO);
	Color.a *= SpecularOcclusion;

	vec2 LocalPosition = gl_FragCoord.xy;
	Color.rgb += GatherRadiance(Color.a, WorldPosition, R, GBuffer.Roughness, GBuffer.WorldNormal, IndirectIrradiance, 1);

	Color.rgb *= EnvBRDF(GBuffer.SpecularColor, GBuffer.Roughness, NoV);
	return Color;
}

void main()
{
	GBufferData GBuffer;
	GBuffer.WorldNormal = normalize(NormalWorld);
	GBuffer.IndirectIrradiance = 0.0;
	GBuffer.Roughness = 0.0;
	GBuffer.GBufferAO = 1.0;
	GBuffer.SpecularColor = vec3(1.0, 1.0, 1.0);

	vec4 ColorOut = CalculateReflection(GBuffer);
	if(bReflect == 1)
	{
		fragmentColor = ColorOut;
	}
	else{
    	fragmentColor = vec4(ColorTest, 1.0);
	}
    if(bLastLight > 0)
	{
		ScreenDepth = gl_FragCoord.z;
		ScreenVelocity = (ClipSpacePos_Curr.xy / ClipSpacePos_Curr.w - ClipSpacePos_Pre.xy / ClipSpacePos_Pre.w) * 0.5;
		BaseColor_Out = vec4(VDebugColor, 1.0);
		CustomData_Out = vec4(0.0, 1.0, 0.0, 0.0);
	}else
	{
		ScreenDepth = Brightness;
		ScreenVelocity = vec2(AverageBrightness);
		BaseColor_Out = texture(PreIntegratedGF, vec2(0.0));
		CustomData_Out = texture(ReflectionTex, vec3(0.0));
	}
}
