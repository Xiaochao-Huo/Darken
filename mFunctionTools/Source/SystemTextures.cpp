#include "SystemTextures.h"
#include "Util.h"
#include "glew.h"
#include "TypeDefine.h"

#ifndef PI
#define PI 3.1415926535897932384626433832795028841968f
#endif // !PI

SystemTextureFactory::SystemTextureFactory()
{
}

SystemTextureFactory::~SystemTextureFactory()
{
	glDeleteTextures(1, &PreIntegratedGF_Tex);
}

void SystemTextureFactory::GeneratePreIntegratedGFTexture()
{
	UInt32 Width = 128;
	UInt32 Height = 32;
	UInt32 DestStride = Width * 2;

	Float16 * data = new Float16[Width * Height * 2];
	memset(data, 0, (UInt64)Width * Height * 2 * sizeof(Float16));

	for (UInt32 y = 0; y < Height; y++)
	{
		Float32 Roughness = (Float32)(y + 0.5f) / Height;
		Float32 m = Roughness * Roughness;
		Float32 m2 = m * m;

		for (UInt32 x = 0; x < Width; x++)
		{
			Float32 NoV = (Float32)(x + 0.5f) / Width;

			Vector3f V;
			V.x = sqrt(1.0f - NoV * NoV);	// sin
			V.y = 0.0f;
			V.z = NoV;								// cos

			Float32 A = 0.0f;
			Float32 B = 0.0f;
			Float32 C = 0.0f;

			const UInt32 NumSamples = 128;
			for (UInt32 i = 0; i < NumSamples; i++)
			{
				Float32 E1 = (Float32)i / NumSamples;
				Float32 E2 = (Float32) ((Float64)ReverseBits(i) / (Float64)0x100000000LL);

				{
					Float32 Phi = 2.0f * PI * E1;
					Float32 CosPhi = cos(Phi);
					Float32 SinPhi = sin(Phi);
					Float32 CosTheta = sqrt((1.0f - E2) / (1.0f + (m2 - 1.0f) * E2));
					Float32 SinTheta = sqrt(1.0f - CosTheta * CosTheta);

					Vector3f H(SinTheta * cos(Phi), SinTheta * sin(Phi), CosTheta);
					Vector3f L = 2.0f * (Math::Dot(V, H)) * H - V;

					Float32 NoL = Math::Max(L.z, 0.0f);
					Float32 NoH = Math::Max(H.z, 0.0f);
					Float32 VoH = Math::Max(Math::Dot(V, H), 0.0f);

					if (NoL > 0.0f)
					{
						Float32 Vis_SmithV = NoL * (NoV * (1 - m) + m);
						Float32 Vis_SmithL = NoV * (NoL * (1 - m) + m);
						Float32 Vis = 0.5f / (Vis_SmithV + Vis_SmithL);

						Float32 NoL_Vis_PDF = NoL * Vis * (4.0f * VoH / NoH);
						Float32 Fc = 1.0f - VoH;
						Fc *= pow(Fc*Fc, 2.0f);
						A += NoL_Vis_PDF * (1.0f - Fc);
						B += NoL_Vis_PDF * Fc;
					}
				}

				{
					Float32 Phi = 2.0f * PI * E1;
					Float32 CosPhi = cos(Phi);
					Float32 SinPhi = sin(Phi);
					Float32 CosTheta = sqrt(E2);
					Float32 SinTheta = sqrt(1.0f - CosTheta * CosTheta);

					Vector3f L(SinTheta * cos(Phi), SinTheta * sin(Phi), CosTheta);
					Vector3f H = Math::Normalize(V + L);

					Float32 NoL = Math::Max(L.z, 0.0f);
					Float32 NoH = Math::Max(H.z, 0.0f);
					Float32 VoH = Math::Max(Math::Dot(V, H), 0.0f);

					Float32 FD90 = 0.5f + 2.0f * VoH * VoH * Roughness;
					Float32 FdV = 1.0f + (FD90 - 1.0f) * pow(1.0f - NoV, 5);
					Float32 FdL = 1.0f + (FD90 - 1.0f) * pow(1.0f - NoL, 5);
					C += FdV * FdL;// * ( 1.0f - 0.3333f * Roughness );
				}
			}
			A /= NumSamples;
			B /= NumSamples;
			C /= NumSamples;

			Float16* Dest = (Float16*)(data + (UInt64)x * 2 + (UInt64)y * DestStride);
			Dest[0] = (Float16)A;
			Dest[1] = (Float16)B;
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

UInt32 SystemTextureFactory::GetPreIntegratedGF_Tex()
{
	return PreIntegratedGF_Tex;
}