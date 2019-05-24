#version 330

#define Texture2DSample(tex, coord) texture(tex, coord)
#define samplePoint(tex, coord) texture(tex, coord)
#define sampleLevelZeroOffset(tex, coord, offset) textureLodOffset(tex, coord, 0.0, offset)
#define sampleOffset(tex, coord, offset) texture(tex, coord, offset)
#define saturate(a) clamp(a, 0.0, 1.0)
#define lerp(a, b, t) mix(a, b, t)
#define mul(v, m) (m * v)
#define mad(a, b, c) (a * b + c)
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define float4x4 mat4x4
#define float3x3 mat3x3
precision mediump float;
in vec2 ScreenUV;
out vec4 color;

uniform sampler2D PostprocessInput0; // LIGHT COLOR
uniform sampler2D PostprocessInput1; // SPECULAR AND SHADING MODELID
uniform sampler2D PostprocessInput2; // DEPTH
uniform vec2      CameraNearFar;

const int skinshadingmodelid = 5;
const int hairshadingmodelid = 7;
struct SDiffuseAndSpecular
{
	float3 Diffuse;
	float3 Specular;
};
int DecodeShadingModelId(float InPackedChannel)
{
	return int(round(InPackedChannel * 255.0f)) ;
}
float LinearDepth(float z, float Near, float Far)
{

    float Depth = z * 2.0 - 1.0;
    return (2.0 * Near) / (Far + Near - Depth *(Far - Near));
}
float Luminance(vec3 linearColor)
{
	return dot(linearColor,  vec3(0.3, 0.59, 0.11));
}
bool UseSubsurfaceProfile(int shadingmodelid)
{
    return (shadingmodelid == skinshadingmodelid || shadingmodelid == 9 ) ;
}
SDiffuseAndSpecular ReconstructLighting(float4 CenterSample)
{
	SDiffuseAndSpecular Ret;
	{

	  float3 CombinedColor   = CenterSample.rgb;
	  float DiffuseLuminance = CenterSample.a;

	  float CombinedLuminance = Luminance(CombinedColor);
	  float DiffuseFactor = saturate(DiffuseLuminance / CombinedLuminance);
	  float SpecularFactor = 1.0f - DiffuseFactor;

	  Ret.Diffuse  = CombinedColor * DiffuseFactor;
	  Ret.Specular = CombinedColor * SpecularFactor;
	}
	return Ret;
}
float4 SetupSubsurfaceForOnePixel(float2 UVSceneColor)
{
	 float4 Ret = float4(1.0);
    float4 CenterSample  = texture(PostprocessInput0, UVSceneColor);
    int ShadingModelID   = DecodeShadingModelId( texture(PostprocessInput1, UVSceneColor).g ) ;
    Ret = CenterSample;
    if( UseSubsurfaceProfile(ShadingModelID) )
    {
        SDiffuseAndSpecular DiffuseAndSpecular = ReconstructLighting(CenterSample);
        Ret.rgb = DiffuseAndSpecular.Diffuse;
        //Ret.rgb =vec3(1.0,0.0,0.0);
		Ret.a   = 1.0f;
    }
    return Ret;
}
void main()
{
    color = vec4( 1.0f ) ;
    color = SetupSubsurfaceForOnePixel(ScreenUV);
    float4 CenterSample  = texture(PostprocessInput0, ScreenUV);
   // color =vec4(vec3(CenterSample.r),1.0);
    if( color.a > 0.0f)
    {
			  const float depthScaleParam = 60.0f;
        color.a = LinearDepth(texture(PostprocessInput2,ScreenUV).r,CameraNearFar.x,CameraNearFar.y)*(CameraNearFar.y/depthScaleParam);
	}

}
