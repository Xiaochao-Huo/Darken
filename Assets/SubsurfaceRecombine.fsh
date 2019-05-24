
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

uniform sampler2D PostprocessInput0;  //lightcolor
uniform sampler2D PostprocessInput1;  //sss color
uniform sampler2D PostprocessInput2;  //base color
uniform sampler2D PostprocessInput3;  //specular shadingmodelid opacity

//uniform sampler2D PostprocessInput4;  //base color
//uniform sampler2D PostprocessInput5;  //specular shadingmodelid opacity

uniform vec4      SubsurfaceColor_SKIN;
uniform vec4      SubsurfaceColor_EYEBLEND;
uniform vec4      SubsurfaceColor_EYEREFRACTIVE;

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

	  Ret.Diffuse = CombinedColor * DiffuseFactor;
	  Ret.Specular = CombinedColor * SpecularFactor;
	}
	return Ret;
}




vec3 srgbToLinear(vec3 c) {
    vec3 gamma = vec3(1.0/2.2);
    return pow(c, gamma);
}
vec4 linearToSrgba(vec4 c) {
    vec4 gamma = vec4(2.2);
    return pow(c, gamma);
}

void main()
{
    color =vec4( 1.0f ) ;
    float2 BufferUV = ScreenUV.xy;
    //vec4 setuptex = texture(PostprocessInput4, BufferUV);
    //vec4 ssssx = texture(PostprocessInput5, BufferUV);
  vec4 sso = texture(PostprocessInput3, BufferUV);
	 int  ShadingModelID   = DecodeShadingModelId(sso.g) ;
	 int  MaterialID   = DecodeShadingModelId(sso.a) ;
  float StoredSpecular        = sso.r;
  float4 lightcolor      = texture(PostprocessInput0, BufferUV);
  if (!UseSubsurfaceProfile(ShadingModelID))
	{
		color = lightcolor;
		return;
	}
	float3 SSSColor;
    float4 SSSColorWithAlpha = texture(PostprocessInput1, BufferUV);
    SSSColor = SSSColorWithAlpha.rgb / max(SSSColorWithAlpha.a, 0.00001f);
    float LerpFactor = 1.0;
    float3 StoredBaseColor = texture(PostprocessInput2, BufferUV).rgb;

    SDiffuseAndSpecular DiffuseAndSpecular = ReconstructLighting(lightcolor);
    float3 ExtractedNonSubsurface = DiffuseAndSpecular.Specular;

		float3 SubsurfaceColor;
	if(MaterialID ==4)  //eye blend
	{
	SubsurfaceColor = SubsurfaceColor_EYEBLEND.rgb;

	}
	else if(MaterialID ==2) //eye refractive
	{
		SubsurfaceColor = SubsurfaceColor_EYEREFRACTIVE.rgb;

	}
	else if(MaterialID ==3) //skin
	{
		SubsurfaceColor = SubsurfaceColor_SKIN.rgb;
	}
	float3 FadedSubsurfaceColor = SubsurfaceColor.rgb * LerpFactor;
    float3 SubsurfaceLighting = lerp(DiffuseAndSpecular.Diffuse, SSSColor, FadedSubsurfaceColor);

    color = float4(SubsurfaceLighting * (StoredBaseColor) +ExtractedNonSubsurface, 0);
    //color = texture(PostprocessInput1, ScreenUV);
   //color =SSSColor;
 //color.rgb =vec3(SubsurfaceLighting);
}
