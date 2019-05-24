#version 330
/**
 * Copyright (C) 2012 Jorge Jimenez (jorge@iryoku.com)
 * Copyright (C) 2012 Diego Gutierrez (diegog@unizar.es)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution:
 *
 *       "Uses Separable SSS. Copyright (C) 2012 by Jorge Jimenez and Diego
 *        Gutierrez."
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the copyright holders.
 */


/**
 *                  _______      _______      _______      _______
 *                 /       |    /       |    /       |    /       |
 *                |   (----    |   (----    |   (----    |   (----
 *                 \   \        \   \        \   \        \   \
 *              ----)   |    ----)   |    ----)   |    ----)   |
 *             |_______/    |_______/    |_______/    |_______/
 *
 *        S E P A R A B L E   S U B S U R F A C E   S C A T T E R I N G
 *
 *                           http://www.iryoku.com/
 *
 * Hi, thanks for your interest in Separable SSS!
 *
 * It's a simple shader composed of two components:
 *
 * 1) A transmittance function, 'SSSSTransmittance', which allows to calculate
 *    light transmission in thin slabs, useful for ears and nostrils. It should
 *    be applied during the main rendering pass as follows:
 *
 *        float3 t = albedo.rgb * lights[i].color * attenuation * spot;
 *        color.rgb += t * SSSSTransmittance(...)
 *
 *    (See 'Main.fx' for more details).
 *
 * 2) A simple two-pass reflectance post-processing shader, 'SSSSBlur*', which
 *    softens the skin appearance. It should be applied as a regular
 *    post-processing effect like bloom (the usual framebuffer ping-ponging):
 *
 *    a) The first pass (horizontal) must be invoked by taking the final color
 *       framebuffer as input, and storing the results into a temporal
 *       framebuffer.
 *    b) The second pass (vertical) must be invoked by taking the temporal
 *       framebuffer as input, and storing the results into the original final
 *       color framebuffer.
 *
 *    Note that This SSS filter should be applied *before* tonemapping.
 *
 * Before including SeparableSSS.h you'll have to setup the target. The
 * following targets are available:
 *         SMAA_HLSL_3
 *         SMAA_HLSL_4
 *         SMAA_GLSL_3
 *
 * For more information of what's under the hood, you can check the following
 * URLs (but take into account that the shader has evolved a little bit since
 * these publications):
 *
 * 1) Reflectance: http://www.iryoku.com/sssss/
 * 2) Transmittance: http://www.iryoku.com/translucency/
 *
 * If you've got any doubts, just contact us!
 */
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
#define SSSS_KERNEL0_OFFSET						0
#define SSSS_KERNEL0_SIZE						  13
#define SSSS_KERNEL1_OFFSET					  13
#define SSSS_KERNEL1_SIZE						   9
#define SSSS_KERNEL2_OFFSET						22
#define SSSS_KERNEL2_SIZE						   6
#define SSSS_KERNEL_TOTAL_SIZE					28
#define SSSS_KERNEL_TOTAL_SIZE_WITH_PADDING		32

#define SSS_SAMPLEQUALITY    2

#if SSS_SAMPLEQUALITY == 0
	#define	SSSS_N_KERNELWEIGHTCOUNT SSSS_KERNEL2_SIZE
	#define	SSSS_N_KERNELWEIGHTOFFSET SSSS_KERNEL2_OFFSET
#elif SSS_SAMPLEQUALITY == 1
	#define	SSSS_N_KERNELWEIGHTCOUNT SSSS_KERNE	L1_SIZE
	#define	SSSS_N_KERNELWEIGHTOFFSET SSSS_KERNEL1_OFFSET
#else // SSS_SAMPLEQUALITY == 2
	#define	SSSS_N_KERNELWEIGHTCOUNT SSSS_KERNEL0_SIZE
	#define	SSSS_N_KERNELWEIGHTOFFSET SSSS_KERNEL0_OFFSET
#endif
precision mediump float;
in vec2 ScreenUV;
out vec4 color;
const int skinshadingmodelid = 5;
const int hairshadingmodelid = 7;
const int ssssprofileSize =13;
struct SDiffuseAndSpecular
{
	float3 Diffuse;
	float3 Specular;
};
uniform sampler2D PostprocessInput0;  //diffuse
uniform sampler2D PostprocessInput1;  // specular shadingmodelid opacity
uniform vec2      SSSParams;
uniform vec4      ViewSizeAndInvSize;
uniform vec4      SSSSMirroredProfilekernel_SKIN[32];
uniform vec4      SSSSMirroredProfilekernel_EYEBLEND[32];
uniform vec4      SSSSMirroredProfilekernel_EYEREFRACTIVE[32];
uniform int       SSS_DIRECTION ; // 0 : horizontal ,1 : vertical

const vec4 View_BufferBilinearUVMinMax = vec4(0.00027,0.00056,0.99866,0.99722);
const int SUBSURFACE_KERNEL_SIZE = 3;
int DecodeShadingModelId(float InPackedChannel)
{
	return int(round(InPackedChannel * 255.0f)) ;
}

vec4 GetProfile(int index,int MaterialID)
{
	if(MaterialID ==4)  //eye blend
	{
		return SSSSMirroredProfilekernel_EYEBLEND[index];

	}
	else if(MaterialID ==2) //eye refractive
	{
		return SSSSMirroredProfilekernel_EYEREFRACTIVE[index];

	}
	else if(MaterialID ==3) //skin
	{
		return SSSSMirroredProfilekernel_SKIN[index];
	}

}
bool UseSubsurfaceProfile(int shadingmodelid)
{
    return (shadingmodelid == skinshadingmodelid || shadingmodelid == 9 ) ;
}

float4 SampleSetup(float2 BufferUV)
{
	BufferUV = clamp(BufferUV, View_BufferBilinearUVMinMax.xy, View_BufferBilinearUVMinMax.zw);
	return texture(PostprocessInput0,BufferUV);
}


float GetSubsurfaceStrength(float2 UV)
{
	vec4 res = texture(PostprocessInput1, UV);
	return res.b;
}





float4 SSSSBlurPS(float2 texcoord,float2 dir,int MaterialID) {

    float sssWidth =0.025;
    const float RANGE =  3.0f ;
    const float EXPONENT = 2.0f;
    int nsamples = SSSS_N_KERNELWEIGHTCOUNT*2 -1;

    float4 colorM = SampleSetup(texcoord);//texture(PostprocessInput0, texcoord);
    float SSSStrength = GetSubsurfaceStrength(texcoord);
    float  depthM = colorM.a;
    // Calculate the sssWidth scale (1.0 for a unit plane sitting on the
    // projection window):
    float distanceToProjectionWindow = SSSParams.y;
    float scale = SSSParams.x / depthM;

    // Calculate the final step to fetch the surrounding pixels:
    float2 finalStep = sssWidth * scale * dir;
    //finalStep *= SSSStrength; // Modulate it using the alpha channel.

	float range = nsamples > 20 ? 3.0 : 2.0;
    finalStep *= 1.0 / range; // Divide by 3 as the kernels range from -3 to 3.
   vec3 sssStrength = vec3(0.68, 0.33, 0.23);
    // Accumulate the center sample:
    //float4 colorBlurred = colorM;
		float3 kernel0 = GetProfile(SSSS_N_KERNELWEIGHTOFFSET+0,MaterialID).rgb;
		//kernel0 = lerp(float3(1.0),kernel0,sssStrength);
    //colorBlurred.rgb *= kernel0;//SSSSMirroredProfilekernel[SSSS_N_KERNELWEIGHTOFFSET+0].rgb;
		float3 colorAccum = float3(0);
// >0 to avoid division by 0, not 100% correct to not visible
float3 colorInvDiv = float3(0.00001f);

// center sample
colorInvDiv += kernel0.rgb;
colorAccum = colorM.rgb * kernel0.rgb;
		for (int i = 1; i <  SSSS_N_KERNELWEIGHTCOUNT ; ++i)
		{

			float4  Kernel = GetProfile(SSSS_N_KERNELWEIGHTOFFSET+i,MaterialID);
      //Kernel.rgb    *= sssStrength;
			float4 LocalAccum = vec4(0.0f);

			float2 UVOffset = Kernel.a * finalStep;

			for (int Side = -1; Side <= 1; Side += 2)
			{
				float2 offset_ = float2(1.0);
				if(Side == -1)
				{
					offset_ = float2 (-1.0f);
				}
				float2 LocalUV = texcoord + UVOffset * offset_;
				float4 color = SampleSetup(LocalUV);//texture(PostprocessInput0, LocalUV);
			  float LocalDepth = color.a;
        // If the difference in depth is huge, we lerp color back to "colorM":

        float s = saturate(300.0f * distanceToProjectionWindow *sssWidth * abs(depthM - LocalDepth));
        //color.rgb = lerp(color.rgb, colorM.rgb, s);
				color.a *= 1 - s;

					// approximation, ideally we would reconstruct the mask with ComputeMaskFromDepthInAlpha() and do manual bilinear filter
					// needed?
					   color.rgb *= color.a ;
							//color.rgb = lerp(color.rgb, colorM.rgb, s);

				LocalAccum += color;

        // Accumulate:
      //  colorBlurred.rgb += Kernel.rgb * color.rgb;

			}
			colorAccum += Kernel.rgb * LocalAccum.rgb;
	 colorInvDiv += Kernel.rgb * LocalAccum.a;
		}

		float3 OutColor = colorAccum / colorInvDiv;
		    return vec4(OutColor.rgb,depthM);
    //return vec4(colorBlurred.rgb,depthM);
}
void main()
{

    color =vec4( 0.0f ) ;
	int ShadingModelID   = DecodeShadingModelId( texture(PostprocessInput1, ScreenUV).g ) ;
	int MaterialID   = DecodeShadingModelId( texture(PostprocessInput1, ScreenUV).a ) ;
	if(!UseSubsurfaceProfile(ShadingModelID))
	{
		return ;
	}
	float2 dir = float2(1.0, 0);
	float2 ViewportDirectionUV =float2(0.0);
	if( SSS_DIRECTION == 0)
	{
		// horizontal
	 ViewportDirectionUV = float2(1.0, 0.0) ;
	}
	else
	{
		// vertical
		ViewportDirectionUV = float2(0.0, 1.0) ;
	}

	//ViewportDirectionUV *= (ViewSizeAndInvSize.x * ViewSizeAndInvSize.z); //View_BufferSizeAndInvSize ????????? ViewportDirectionUV *= (View_ViewSizeAndInvSize.x * View_BufferSizeAndInvSize.z);
	color = SSSSBlurPS(ScreenUV,ViewportDirectionUV,MaterialID);
	if(SSS_DIRECTION == 1)
	{
		if(color.a > 0.0) color.a = 1.0f;
		else              color.a = 0.0f;

	}
	
}
