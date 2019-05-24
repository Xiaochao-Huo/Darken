#version 330 core
uniform sampler2D normaltex;
uniform int  addToChanel;
uniform float compositepower;
in vec2 ScreenUV;
out vec4 fragmentColor;

const float SMALL_NUMBER = (1.e-8f);
vec3 GetSafeNormal (vec3 Normal, float Tolerance)
{
		float X = Normal.x;
		float Y = Normal.y;
		float Z = Normal.z;
		float SquareSum = X * X + Y * Y + Z * Z;

		// Not sure if it's safe to add tolerance in there. Might introduce too many errors
		if (SquareSum == 1.f)
		{
			return Normal;
		}
		else if (SquareSum < Tolerance)
		{
			return vec3(0.0);
		}
		float Scale = inversesqrt(SquareSum);
		return vec3(X*Scale, Y*Scale, Z*Scale);
};

void main()
{

	int kernelsizex =2;
	int kernelsizey =2;
	vec3 FilteredColor =vec3(0.0);
	for(int y =0;y<kernelsizey; ++y)
	{
		for(int x=0;x<kernelsizex;++x)
		{
			float Weight =0.25;
			vec3 Normal = textureOffset(normaltex, vec2(ScreenUV.x, ScreenUV.y), ivec2(x,  y)).bgr;
			Normal = vec3(Normal.r * 2.0f - 1.0f, Normal.g * 2.0f - 1.0f, Normal.b * 2.0f - 1.0f);
			Normal = GetSafeNormal(Normal, SMALL_NUMBER);
			Normal = vec3(Normal.r * 0.5 + 0.5, Normal.g * 0.5 + 0.5, Normal.b * 0.5 + 0.5);
			FilteredColor += vec3(Weight) * Normal;
		}
	}
vec3 Normal = FilteredColor;
float power = compositepower;
Normal.z = Normal.z*2.0 - 1.0;
Normal.x = Normal.x*2.0 - 1.0;
Normal.y = Normal.y*2.0 - 1.0;
float ns = length(Normal);
float lengthN = ns < 1.0 ? ns : 1.0;
float Variance = (1.0f - lengthN) / lengthN;
Variance = 0.0 > (Variance - 0.00004) ? 0.0 : (Variance - 0.00004);
Variance *= power;
float rougnness = 0.0;
float B0 = 2.0 *Variance*(-1.0);
float a2 = B0 / (B0 - 1.0);
rougnness = pow(a2, 0.25);
fragmentColor = vec4(vec3(0.0),1.0);
fragmentColor[addToChanel] = rougnness;
//fragmentColor.rgb =	FilteredColor;
}
