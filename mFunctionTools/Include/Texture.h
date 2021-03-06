#pragma once

#include <vector>
#include <memory>
#include "FreeImage.h"
#ifndef PI
#define PI 3.1415926535897932384626433832795028841968f
#endif
enum TextureDataTypeFormat
{
	/*RGBA8,
	R8,
	RG8,
	RGB8,
	B32G32R32F*/
	TF_NONE,


	TF_R5G6B5,
	TF_A1R5G5B5,
	TF_A4R4G4B4,
	TF_A8,
	TF_UL8,
	TF_A8L8,
	TF_R8G8B8,
	TF_A8R8G8B8,
	TF_A8B8G8R8,

	// 16-bit integer formats.
	TF_L16_SNORM,
	TF_G16R16,
	TF_A16B16G16R16,

	// 16-bit floating-point formats ('half Float32' channels).
	TF_R16F,
	TF_G16R16F,
	TF_A16B16G16R16F,

	// 32-bit floating-point formats ('Float32' channels).
	TF_R32F,
	TF_G32R32F,
	TF_B32G32R32F,
	TF_A32B32G32R32F,

	// DXT compressed formats.
	TF_DXT1,
	TF_DXT3,
	TF_DXT5,

	// Depth-stencil format.
	TF_D24S8,
	TF_UL16,  // unsigned 16-bit
	TF_R16G16_SNORM,//signed 32-bit
	TF_RGB16,     //3 x 16-bit
	TF_L8_SNORM, //signed 8-bit
	TF_L8I,           //integer 8-bit
	TF_L8UI,        // unsigned integer 8-bit
	TF_R8G8B8UI, // unsigned integer 24-bit
	TF_A8B8G8R8_SNORM,//signed 8_bit

	TF_DEPTHCOMPONENT,
	TF_A32F,
	TF_R32UI,
	TF_QUANTITY
};

struct RawTextureData
{
	Int32 Width;
	Int32 Height;
	UInt8* MipData;
	RawTextureData() {};
};

enum TextureParameter
{
	Linear_Mip_Nearest,
	Nearest_Mip_Nearest,
	Linear_Mip_Linear,
	Nearest_Mip_Linear,

	Linear,
	Nearest,

	Repeat,
	Mirrored_Repeat,
	Clamp_To_Edge,
	Clamp_To_Border
};
enum Channel
{
	RED,
	GREEN,
	BLUE
};

class FImageKernel2D
{
public:
	FImageKernel2D() :FilterTableSize(0)
	{
	}

	// @param TableSize1D 2 for 2x2, 4 for 4x4, 6 for 6x6, 8 for 8x8
	// @param SharpenFactor can be negative to blur
	// generate normalized 2D Kernel with sharpening
	void BuildSeparatableGaussWithSharpen(Int32 TableSize1D, Float32 SharpenFactor = 0.0f)
	{
		if (TableSize1D > MaxKernelExtend)
		{
			TableSize1D = MaxKernelExtend;
		}

		Float32 Table1D[MaxKernelExtend];
		Float32 NegativeTable1D[MaxKernelExtend];

		FilterTableSize = TableSize1D;

		if (SharpenFactor < 0.0f)
		{
			// blur only
			BuildGaussian1D(Table1D, TableSize1D, 1.0f, -SharpenFactor);
			BuildFilterTable2DFrom1D(KernelWeights, Table1D, TableSize1D);
			return;
		}
		else if (TableSize1D == 2)
		{
			// 2x2 kernel: simple average
			KernelWeights[0] = KernelWeights[1] = KernelWeights[2] = KernelWeights[3] = 0.25f;
			return;
		}
		else if (TableSize1D == 4)
		{
			// 4x4 kernel with sharpen or blur: can alias a bit
			BuildFilterTable1DBase(Table1D, TableSize1D, 1.0f + SharpenFactor);
			BuildFilterTable1DBase(NegativeTable1D, TableSize1D, -SharpenFactor);
			BlurFilterTable1D(NegativeTable1D, TableSize1D, 1);
		}
		else if (TableSize1D == 6)
		{
			// 6x6 kernel with sharpen or blur: still can alias
			BuildFilterTable1DBase(Table1D, TableSize1D, 1.0f + SharpenFactor);
			BuildFilterTable1DBase(NegativeTable1D, TableSize1D, -SharpenFactor);
			BlurFilterTable1D(NegativeTable1D, TableSize1D, 2);
		}
		else if (TableSize1D == 8)
		{
			//8x8 kernel with sharpen or blur

			// * 2 to get similar appearance as for TableSize 6
			SharpenFactor = SharpenFactor * 2.0f;

			BuildFilterTable1DBase(Table1D, TableSize1D, 1.0f + SharpenFactor);
			// positive lobe is blurred a bit for better quality
			BlurFilterTable1D(Table1D, TableSize1D, 1);
			BuildFilterTable1DBase(NegativeTable1D, TableSize1D, -SharpenFactor);
			BlurFilterTable1D(NegativeTable1D, TableSize1D, 3);
		}
		else
		{
			// not yet supported
			//check(0);
		}

		AddFilterTable1D(Table1D, NegativeTable1D, TableSize1D);
		BuildFilterTable2DFrom1D(KernelWeights, Table1D, TableSize1D);
	}

	inline Int32 GetFilterTableSize() const
	{
		return FilterTableSize;
	}

	inline Float32 GetAt(Int32 X, Int32 Y) const
	{
		//checkSlow(X < FilterTableSize);
		//checkSlow(Y < FilterTableSize);
		return KernelWeights[X + Y * FilterTableSize];
	}

	inline Float32& GetRefAt(Int32 X, Int32 Y)
	{
		//checkSlow(X < FilterTableSize);
		//checkSlow(Y < FilterTableSize);
		return KernelWeights[X + Y * FilterTableSize];
	}

private:

	inline static Float32 NormalDistribution(Float32 X, Float32 Variance)
	{
		const Float32 StandardDeviation = sqrtf(Variance);
		return expf(-(X*X) / (2.0f * Variance)) / (StandardDeviation * sqrtf(2.0f * (Float32)PI));
	}

	// support even and non even sized filters
	static void BuildGaussian1D(Float32 *InOutTable, Int32 TableSize, Float32 Sum, Float32 Variance)
	{
		Float32 Center = TableSize * 0.5f;
		Float32 CurrentSum = 0;
		for (Int32 i = 0; i < TableSize; ++i)
		{
			Float32 Actual = NormalDistribution(i - Center + 0.5f, Variance);
			InOutTable[i] = Actual;
			CurrentSum += Actual;
		}
		// Normalize
		Float32 InvSum = Sum / CurrentSum;
		for (Int32 i = 0; i < TableSize; ++i)
		{
			InOutTable[i] *= InvSum;
		}
	}

	//
	static void BuildFilterTable1DBase(Float32 *InOutTable, Int32 TableSize, Float32 Sum)
	{
		// we require a even sized filter
		//check(TableSize % 2 == 0);

		Float32 Inner = 0.5f * Sum;

		Int32 Center = TableSize / 2;
		for (Int32 x = 0; x < TableSize; ++x)
		{
			if (x == Center || x == Center - 1)
			{
				// center elements
				InOutTable[x] = Inner;
			}
			else
			{
				// outer elements
				InOutTable[x] = 0.0f;
			}
		}
	}

	// InOutTable += InTable
	static void AddFilterTable1D(Float32 *InOutTable, Float32 *InTable, Int32 TableSize)
	{
		for (Int32 x = 0; x < TableSize; ++x)
		{
			InOutTable[x] += InTable[x];
		}
	}

	// @param Times 1:box, 2:triangle, 3:pow2, 4:pow3, ...
	// can be optimized with Float64 buffering but doesn't need to be fast
	static void BlurFilterTable1D(Float32 *InOutTable, Int32 TableSize, Int32 Times)
	{
		//check(Times > 0);
		//check(TableSize < 32);

		Float32 Intermediate[32];

		for (Int32 Pass = 0; Pass < Times; ++Pass)
		{
			for (Int32 x = 0; x < TableSize; ++x)
			{
				Intermediate[x] = InOutTable[x];
			}

			for (Int32 x = 0; x < TableSize; ++x)
			{
				Float32 sum = Intermediate[x];

				if (x)
				{
					sum += Intermediate[x - 1];
				}
				if (x < TableSize - 1)
				{
					sum += Intermediate[x + 1];
				}

				InOutTable[x] = sum / 3.0f;
			}
		}
	}

	static void BuildFilterTable2DFrom1D(Float32 *OutTable2D, Float32 *InTable1D, Int32 TableSize)
	{
		for (Int32 y = 0; y < TableSize; ++y)
		{
			for (Int32 x = 0; x < TableSize; ++x)
			{
				OutTable2D[x + y * TableSize] = InTable1D[y] * InTable1D[x];
			}
		}
	}

	// at max we support MaxKernelExtend x MaxKernelExtend kernels
	const static Int32 MaxKernelExtend = 12;
	// 0 if no kernel was setup yet
	Int32 FilterTableSize;
	// normalized, means the sum of it should be 1.0f
	Float32 KernelWeights[MaxKernelExtend * MaxKernelExtend];
};
class Texture
{
public:
	UInt32 GPUId;
	UInt32 GPUSamplerId;

	Texture();
	~Texture();
	Texture(std::string file, TextureParameter minParm, TextureParameter magParm, TextureParameter wrapParmU, TextureParameter wrapParmV);
	//UInt32 CreateCompositeTexture(Float32 compositepower, Bool useGPU = true, Channel AddToChanel = GREEN, std::string compositeTexFilePath = " ");
	void CreateGPUObject(TextureParameter minParm, TextureParameter magParm, TextureParameter wrapParmU, TextureParameter wrapParmV);
	void UpdateGPUObjectData();

	void LoadTextureFromAsset(std::string const& file);
	//void InitMipRawTextureData(Int32 width, Int32 height, TextureDataTypeFormat typeFormat);
	//void InitNoMipRawTextureData(Int32 width, Int32 height, TextureDataTypeFormat typeFormat);

	Int32 GetNumMip();
	Int32 GetWidth();
	Int32 GetHeight();
	std::shared_ptr<RawTextureData> GetRawDataPtr(Int32 Mip);

private:
	Int32 Width;
	Int32 Height;
	FREE_IMAGE_FORMAT GetFileType(std::string const& file);
	TextureDataTypeFormat GetTextureType(FIBITMAP *dib);
	TextureDataTypeFormat TypeFormat;
	std::vector<std::shared_ptr<RawTextureData>> RawTextureDataPtrs;
};

