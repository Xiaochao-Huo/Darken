#include "SystemTextures.h"
#include "Util.h"
#include "glew.h"
#include "half.hpp"

#ifndef PI
#define PI 3.1415926535897932384626433832795028841968f
#endif // !PI

using half_float::half;
SystemTextureFactory::SystemTextureFactory()
{
}

SystemTextureFactory::~SystemTextureFactory()
{
	glDeleteTextures(1, &PreIntegratedGF_Tex);
}

void SystemTextureFactory::GeneratePreIntegratedGFTexture()
{
	unsigned int Width = 128;
	unsigned int Height = 32;
	unsigned int DestStride = Width * 2;

	half * data = new half[Width * Height * 2];
	memset(data, 0, Width * Height * 2 * sizeof(half));

	for (unsigned int y = 0; y < Height; y++)
	{
		float Roughness = (float)(y + 0.5f) / Height;
		float m = Roughness * Roughness;
		float m2 = m * m;

		for (unsigned int x = 0; x < Width; x++)
		{
			float NoV = (float)(x + 0.5f) / Width;

			glm::vec3 V;
			V.x = sqrt(1.0f - NoV * NoV);	// sin
			V.y = 0.0f;
			V.z = NoV;								// cos

			float A = 0.0f;
			float B = 0.0f;
			float C = 0.0f;

			const unsigned int NumSamples = 128;
			for (unsigned int i = 0; i < NumSamples; i++)
			{
				float E1 = (float)i / NumSamples;
				float E2 = (double)ReverseBits(i) / (double)0x100000000LL;

				{
					float Phi = 2.0f * PI * E1;
					float CosPhi = cos(Phi);
					float SinPhi = sin(Phi);
					float CosTheta = sqrt((1.0f - E2) / (1.0f + (m2 - 1.0f) * E2));
					float SinTheta = sqrt(1.0f - CosTheta * CosTheta);

					glm::vec3 H(SinTheta * cos(Phi), SinTheta * sin(Phi), CosTheta);
					glm::vec3 L = 2.0f * (glm::dot(V, H)) * H - V;

					float NoL = glm::max(L.z, 0.0f);
					float NoH = glm::max(H.z, 0.0f);
					float VoH = glm::max(glm::dot(V, H), 0.0f);

					if (NoL > 0.0f)
					{
						float Vis_SmithV = NoL * (NoV * (1 - m) + m);
						float Vis_SmithL = NoV * (NoL * (1 - m) + m);
						float Vis = 0.5f / (Vis_SmithV + Vis_SmithL);

						float NoL_Vis_PDF = NoL * Vis * (4.0f * VoH / NoH);
						float Fc = 1.0f - VoH;
						Fc *= pow(Fc*Fc, 2.0);
						A += NoL_Vis_PDF * (1.0f - Fc);
						B += NoL_Vis_PDF * Fc;
					}
				}

				{
					float Phi = 2.0f * PI * E1;
					float CosPhi = cos(Phi);
					float SinPhi = sin(Phi);
					float CosTheta = sqrt(E2);
					float SinTheta = sqrt(1.0f - CosTheta * CosTheta);

					glm::vec3 L(SinTheta * cos(Phi), SinTheta * sin(Phi), CosTheta);
					glm::vec3 H = glm::normalize(V + L);

					float NoL = glm::max(L.z, 0.0f);
					float NoH = glm::max(H.z, 0.0f);
					float VoH = glm::max(glm::dot(V, H), 0.0f);

					float FD90 = 0.5f + 2.0f * VoH * VoH * Roughness;
					float FdV = 1.0f + (FD90 - 1.0f) * pow(1.0f - NoV, 5);
					float FdL = 1.0f + (FD90 - 1.0f) * pow(1.0f - NoL, 5);
					C += FdV * FdL;// * ( 1.0f - 0.3333f * Roughness );
				}
			}
			A /= NumSamples;
			B /= NumSamples;
			C /= NumSamples;

			half * Dest = (half*)(data + x * 2 + y * DestStride);
			Dest[0] = (half)A;
			Dest[1] = (half)B;
		}
	}

	glGenTextures(1, &PreIntegratedGF_Tex);
	glBindTexture(GL_TEXTURE_2D, PreIntegratedGF_Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, Width, Height, 0, GL_RG, GL_HALF_FLOAT, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	delete [] data;
}

unsigned int SystemTextureFactory::GetPreIntegratedGF_Tex()
{
	return PreIntegratedGF_Tex;
}