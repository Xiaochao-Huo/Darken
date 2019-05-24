#version 440 core
#define FLT_MAX 3.402823466e+38

precision mediump float;

uniform sampler2D PreFrame;

uniform sampler2D CurrentFrame;
uniform sampler2D DepthTex;
uniform sampler2D VelocityTex;
uniform vec2 ScreenSize;
uniform vec4 _ZBufferParams;

uniform vec4 JitterUV;

in vec2 ScreenUV;
in vec2 ScreenUV_Pre;
out vec4 fragmentColor;

const float _FeedbackMin = 0.8;
const float _FeedbackMax = 0.9;

vec3 FDebugColor;

float Linear01Depth(float z)
{
	// x = near
    // y = far
    // z = fovInRadian
    // w = Aspect
    //linearDepth = (2.0 * near) / (far + near - z * (far - near));
    float Depth = z * 2.0 - 1.0;
    return (2.0 * _ZBufferParams.x) / (_ZBufferParams.y + _ZBufferParams.x - Depth *(_ZBufferParams.y - _ZBufferParams.x));
}

float LinearEyeDepth(float z)
{
	return Linear01Depth(z) * _ZBufferParams.y;
}

vec3 find_closest_fragment_3x3(vec2 uv)
{
	vec2 dd = 1.0 / ScreenSize;
	vec2 du = vec2(dd.x, 0.0);
	vec2 dv = vec2(0.0, dd.y);

	vec3 dtl = vec3(-1.0, -1.0, 1.0 - texture(DepthTex, uv - dv - du).x);
	vec3 dtc = vec3( 0.0, -1.0, 1.0 - texture(DepthTex, uv - dv).x);
	vec3 dtr = vec3( 1.0, -1.0, 1.0 - texture(DepthTex, uv - dv + du).x);

	vec3 dml = vec3(-1.0, 0.0, 1.0 - texture(DepthTex, uv - du).x);
	vec3 dmc = vec3( 0.0, 0.0, 1.0 - texture(DepthTex, uv).x);
	vec3 dmr = vec3( 1.0, 0.0, 1.0 - texture(DepthTex, uv + du).x);

	vec3 dbl = vec3(-1.0, 1.0, 1.0 - texture(DepthTex, uv + dv - du).x);
	vec3 dbc = vec3( 0.0, 1.0, 1.0 - texture(DepthTex, uv + dv).x);
	vec3 dbr = vec3( 1.0, 1.0, 1.0 - texture(DepthTex, uv + dv + du).x);

	vec3 dmin = dtl;
	if (dmin.z < dtc.z) dmin = dtc;
	if (dmin.z < dtr.z) dmin = dtr;

	if (dmin.z < dml.z) dmin = dml;
	if (dmin.z < dmc.z) dmin = dmc;
	if (dmin.z < dmr.z) dmin = dmr;

	if (dmin.z < dbl.z) dmin = dbl;
	if (dmin.z < dbc.z) dmin = dbc;
	if (dmin.z < dbr.z) dmin = dbr;

	return vec3(uv + dd.xy * dmin.xy, dmin.z);
}

vec4 clip_aabb(vec3 aabb_min, vec3 aabb_max, vec4 p, vec4 q)
{
	/*
	// note: only clips towards aabb center (but fast!)
	vec3 p_clip = 0.5 * (aabb_max + aabb_min);
	vec3 e_clip = 0.5 * (aabb_max - aabb_min) + 0.00000001f;
	
	vec4 v_clip = q - vec4(p_clip, p.w);
	vec3 v_unit = v_clip.xyz / e_clip;
	vec3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

	if (ma_unit > 1.0)
		return vec4(p_clip, p.w) + v_clip / ma_unit;
	else
		return q;// point inside aabb
	*/
	vec4 r = q - p;
	vec3 rmax = aabb_max - p.xyz;
	vec3 rmin = aabb_min - p.xyz;

	float eps = 0.0001f;;

	if (r.x > rmax.x + eps)
		r *= (rmax.x / r.x);
	if (r.y > rmax.y + eps)
		r *= (rmax.y / r.y);
	if (r.z > rmax.z + eps)
		r *= (rmax.z / r.z);
	
	if (r.x < rmin.x - eps)
		r *= (rmin.x / r.x);
	if (r.y < rmin.y - eps)
		r *= (rmin.y / r.y);
	if (r.z < rmin.z - eps)
		r *= (rmin.z / r.z);
		
	return p + r;
}

vec4 DebugOut_1;
vec4 DebugOut_2;

vec4 temporal_reprojection(vec2 ss_txc, vec2 ss_vel, float vs_dist)
{
	// read texels
	vec4 texel0 = texture(CurrentFrame, ss_txc - JitterUV.xy);

	vec4 texel1 = texture(PreFrame, ss_txc - ss_vel);
		// calc min-max of current neighbourhood

	vec2 uv = ss_txc;

	vec2 du = vec2(1.0 / ScreenSize.x, 0.0);
	vec2 dv = vec2(0.0, 1.0 / ScreenSize.y);

	vec4 ctl = texture(CurrentFrame, uv - dv - du);
	vec4 ctc = texture(CurrentFrame, uv - dv);
	vec4 ctr = texture(CurrentFrame, uv - dv + du);
	vec4 cml = texture(CurrentFrame, uv - du);
	vec4 cmc = texture(CurrentFrame, uv);
	vec4 cmr = texture(CurrentFrame, uv + du);
	vec4 cbl = texture(CurrentFrame, uv + dv - du);
	vec4 cbc = texture(CurrentFrame, uv + dv);
	vec4 cbr = texture(CurrentFrame, uv + dv + du);

	//vec4 cmin = min(ctc, min(cml, min(cmc, min(cmr, cbc))));
	//vec4 cmax = max(ctc, max(cml, max(cmc, max(cmr, cbc))));
	//vec4 cavg = (ctc + cml + cmc + cmr + cbc) / 5.0;

	vec4 cmin = min(ctl, min(ctc, min(ctr, min(cml, min(cmc, min(cmr, min(cbl, min(cbc, cbr))))))));
	vec4 cmax = max(ctl, max(ctc, max(ctr, max(cml, max(cmc, max(cmr, max(cbl, max(cbc, cbr))))))));
	vec4 cavg = (ctl + ctc + ctr + cml + cmc + cmr + cbl + cbc + cbr) / 9.0;
	vec4 cmin5 = min(ctc, min(cml, min(cmc, min(cmr, cbc))));
	vec4 cmax5 = max(ctc, max(cml, max(cmc, max(cmr, cbc))));
	vec4 cavg5 = (ctc + cml + cmc + cmr + cbc) / 5.0;
	cmin = 0.5 * (cmin + cmin5);
	cmax = 0.5 * (cmax + cmax5);
	cavg = 0.5 * (cavg + cavg5);

	// clamp to neighbourhood of current sample
	//texel1 = clip_aabb(cmin.xyz, cmax.xyz, clamp(cavg, cmin, cmax), texel1);
	texel1 = clamp(texel1, cavg - 0.75 *(cavg - cmin), cmax + 0.75 *(cmax - cavg));
	
	DebugOut_1 = clip_aabb(cmin.xyz, cmax.xyz, clamp(cavg, cmin, cmax), texel1);
	DebugOut_2 = clamp(texel1, cavg - 0.5 *(cavg - cmin), cmax - 0.5 *(cmax - cavg));
	
	// feedback weight from unbiased luminance diff (t.lottes)

	float lum0 = dot(texel0.rgb, vec3(0.22, 0.707, 0.071));
	float lum1 = dot(texel1.rgb, vec3(0.22, 0.707, 0.071));
		
	float unbiased_diff = abs(lum0 - lum1) / max(lum0, max(lum1, 0.2));
	float unbiased_weight = 1.0 - unbiased_diff;
	float unbiased_weight_sqr = unbiased_weight * unbiased_weight;
	float k_feedback = mix(_FeedbackMin, _FeedbackMax, unbiased_weight_sqr);
	
	// output
	return mix(texel0, texel1, k_feedback);
}

uniform sampler2D ShadowDepth_TexDebug;
uniform int Channel;
void main()
{
	vec3 c_frag = find_closest_fragment_3x3(ScreenUV);
	vec2 ss_vel = texture(VelocityTex, c_frag.xy).rg;
	float vs_dist = LinearEyeDepth(1.0 - c_frag.z);
	
	// temporal resolve
	vec4 color_temporal = temporal_reprojection(ScreenUV, ss_vel, vs_dist);
	
	switch(Channel)
	{
		case 0: fragmentColor = color_temporal; break;
		case 1: fragmentColor = texture(PreFrame, ScreenUV); break;
		case 2: fragmentColor = DebugOut_1; break;
		case 3: fragmentColor = DebugOut_2; break;
	}
}
