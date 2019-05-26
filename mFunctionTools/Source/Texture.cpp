#include "GlobalPram.h"
#include <glfw3.h>
#include <glew.h>
//#include <stb_image.h>
#include "Framebuffer2D.h"
#include "Texture.h"

Texture::Texture()
{
	Width = -1;
	Height = -1;
	GPUId = 0;
	GPUSamplerId = 0;
}
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
	#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
	#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
	#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
	#define GL_ALPHA32F_ARB 0x8816
	#define GL_HALF_FLOAT_ARB 0x140B
	#define GL_DEPTH24_STENCIL8_EXT 0x88F0
	#define GL_DEPTH_STENCIL_EXT 0x84F9
	#define GL_RGBA32F_ARB 0x8814
	#define GL_RGB32F_ARB 0x8815
	#define GL_ALPHA32F_ARB 0x8816
	#define GL_INTENSITY32F_ARB 0x8817
	#define GL_LUMINANCE32F_ARB 0x8818
	#define GL_LUMINANCE_ALPHA32F_ARB 0x8819
	#define GL_RGBA16F_ARB 0x881A
	#define GL_RGB16F_ARB 0x881B
	#define GL_ALPHA16F_ARB 0x881C
	#define GL_UNSIGNED_INT_24_8_EXT 0x84FA
Texture::Texture(std::string file, TextureParameter minParm, TextureParameter magParm, TextureParameter wrapParmU, TextureParameter wrapParmV)
{
	LoadTextureFromAsset(AssetFolderPath + file);
	CreateGPUObject(minParm, magParm, wrapParmU, wrapParmV);
	//	std::string  path = (AssetFolderPath + file);
	//	{
	//#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
	//#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
	//#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
	//#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
	//#define GL_ALPHA32F_ARB 0x8816
	//#define GL_HALF_FLOAT_ARB 0x140B
	//#define GL_DEPTH24_STENCIL8_EXT 0x88F0
	//#define GL_DEPTH_STENCIL_EXT 0x84F9
	//#define GL_RGBA32F_ARB 0x8814
	//#define GL_RGB32F_ARB 0x8815
	//#define GL_ALPHA32F_ARB 0x8816
	//#define GL_INTENSITY32F_ARB 0x8817
	//#define GL_LUMINANCE32F_ARB 0x8818
	//#define GL_LUMINANCE_ALPHA32F_ARB 0x8819
	//#define GL_RGBA16F_ARB 0x881A
	//#define GL_RGB16F_ARB 0x881B
	//#define GL_ALPHA16F_ARB 0x881C
	//#define GL_UNSIGNED_INT_24_8_EXT 0x84FA
	//		static enum Format
	//		{
	//			TF_NONE,
	//
	//
	//			TF_R5G6B5,
	//			TF_A1R5G5B5,
	//			TF_A4R4G4B4,
	//			TF_A8,
	//			TF_UL8,
	//			TF_A8L8,
	//			TF_R8G8B8,
	//			TF_A8R8G8B8,
	//			TF_A8B8G8R8,
	//
	//			// 16-bit integer formats.
	//			TF_L16_SNORM,
	//			TF_G16R16,
	//			TF_A16B16G16R16,
	//
	//			// 16-bit floating-point formats ('half Float32' channels).
	//			TF_R16F,
	//			TF_G16R16F,
	//			TF_A16B16G16R16F,
	//
	//			// 32-bit floating-point formats ('Float32' channels).
	//			TF_R32F,
	//			TF_G32R32F,
	//			TF_B32G32R32F,
	//			TF_A32B32G32R32F,
	//
	//			// DXT compressed formats.
	//			TF_DXT1,
	//			TF_DXT3,
	//			TF_DXT5,
	//
	//			// Depth-stencil format.
	//			TF_D24S8,
	//			TF_UL16,  // unsigned 16-bit
	//			TF_R16G16_SNORM,//signed 32-bit
	//			TF_RGB16,     //3 x 16-bit
	//			TF_L8_SNORM, //signed 8-bit
	//			TF_L8I,           //integer 8-bit
	//			TF_L8UI,        // unsigned integer 8-bit
	//			TF_R8G8B8UI, // unsigned integer 24-bit
	//			TF_A8B8G8R8_SNORM,//signed 8_bit
	//
	//			TF_DEPTHCOMPONENT,
	//			TF_A32F,
	//			TF_R32UI,
	//			TF_QUANTITY
	//		};
	//		static GLuint gOGLTextureInternalFormat[TF_QUANTITY] =
	//		{
	//			0,                                  // TF_NONE
	//
	//			GL_RGB5,                            // TF_R5G6B5
	//			GL_RGB5_A1,                         // TF_A1R5G5B5
	//			GL_RGBA4,                           // TF_A4R4G4B4
	//			GL_ALPHA8,                          // TF_A8
	//			GL_LUMINANCE8,                      // TF_UL8
	//			GL_LUMINANCE8_ALPHA8,               // TF_A8L8
	//			GL_RGB8,                            // TF_R8G8B8
	//			GL_RGBA8,                           // TF_A8R8G8B8
	//			GL_RGBA8,                           // TF_A8B8G8R8
	//			//GL_LUMINANCE16,                  
	//			GL_R16_SNORM,                                // TF_L16_SNORM
	//			GL_RG16,                            // TF_G16R16
	//			GL_RGBA16,                          // TF_A16B16G16R16
	//			GL_R16F,                            // TF_R16F
	//			GL_RG16F,                           // TF_G16R16F
	//			GL_RGBA16F_ARB,                     // TF_A16B16G16R16F
	//			GL_R32F,                            // TF_R32F
	//			GL_RGB32F_ARB,                      // TF_B32G32R32F
	//			GL_RG32F,                           // TF_G32R32F       //ADD BY ZHANG QIXIN.
	//			GL_RGBA32F_ARB,                     // TF_A32B32G32R32F
	//
	//			GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   // TF_DXT1
	//			GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,   // TF_DXT3
	//			GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,   // TF_DXT5
	//			GL_DEPTH24_STENCIL8_EXT ,            // TF_D24S8
	//			GL_R16,
	//			GL_RG16_SNORM,
	//			GL_RGB16,
	//			GL_R8_SNORM,
	//			GL_R8I,
	//			GL_R8UI,
	//			GL_RGB8UI,
	//			GL_RGBA8_SNORM,
	//			GL_DEPTH_COMPONENT24,              //TF_DEPTHCOMPONENT
	//			GL_ALPHA32F_ARB,                     // TF_A32F
	//			GL_R32UI                            //TF_R32UI
	//		};
	//
	//		static GLuint gOGLTextureFormat[TF_QUANTITY] =
	//		{
	//			0,                                  // TF_NONE
	//
	//			GL_BGR,                             // TF_R5G6B5
	//			GL_RGBA,                            // TF_A1R5G5B5
	//			GL_RGBA,                            // TF_A4R4G4B4
	//			GL_ALPHA,                           // TF_A8
	//			GL_LUMINANCE,                       // TF_L8
	//			GL_LUMINANCE_ALPHA,                 // TF_A8L8
	//			GL_BGR,                             // TF_R8G8B8
	//			GL_BGRA,                            // TF_A8R8G8B8
	//			GL_RGBA,                            // TF_A8B8G8R8
	//			GL_RED,                       // TF_L16
	//			GL_RG,                              // TF_G16R16
	//			GL_RGBA,                            // TF_A16B16G16R16
	//			GL_RED,                             // TF_R16F
	//			GL_RG,                              // TF_G16R16F
	//			GL_RGBA,                            // TF_A16B16G16R16F
	//			GL_RED,                             // TF_R32F
	//			GL_RG,                              // TF_G32R32F
	//			GL_RGB,                             //TF_B32G32R32F
	//			GL_RGBA,                            // TF_A32B32G32R32F
	//
	//			GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   // TF_DXT1
	//			GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,   // TF_DXT3
	//			GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,   // TF_DXT5
	//			GL_DEPTH_STENCIL_EXT,                // TF_D24S8
	//			GL_RED,
	//			GL_RG,
	//			GL_RGB,
	//			GL_RED,
	//			GL_RED_INTEGER,
	//			GL_RED_INTEGER,
	//			GL_RGB_INTEGER,
	//			GL_RGBA,
	//			GL_DEPTH_COMPONENT,     //TF_DEPTHCOMPONENT
	//			GL_ALPHA,                // TF_A32F
	//			GL_RED_INTEGER
	//		};
	//
	//		static GLuint gOGLTextureType[TF_QUANTITY] =
	//		{
	//			0,                              // TF_NONE
	//
	//			GL_UNSIGNED_SHORT_5_6_5_REV,    // TF_R5G6B5
	//			GL_UNSIGNED_SHORT_1_5_5_5_REV,  // TF_A1R5G5B5
	//			GL_UNSIGNED_SHORT_4_4_4_4_REV,  // TF_A4R4G4B4
	//			GL_UNSIGNED_BYTE,               // TF_A8
	//			GL_UNSIGNED_BYTE,                                // TF_L8
	//			GL_UNSIGNED_BYTE,               // TF_A8L8
	//			GL_UNSIGNED_BYTE,               // TF_R8G8B8
	//			GL_UNSIGNED_BYTE,               // TF_A8R8G8B8
	//			GL_UNSIGNED_BYTE,               // TF_A8B8G8R8
	//			GL_SHORT,
	//			GL_UNSIGNED_SHORT,              // TF_G16R16
	//			GL_UNSIGNED_SHORT,              // TF_A16B16G16R16
	//			GL_HALF_FLOAT_ARB,              // TF_R16F
	//			GL_HALF_FLOAT_ARB,              // TF_G16R16F
	//			GL_HALF_FLOAT_ARB,              // TF_A16B16G16R16F
	//			GL_FLOAT,                       // TF_R32F
	//			GL_FLOAT,                       // TF_G32R32F
	//			GL_FLOAT,                       // TF_B32G32R32F
	//			GL_FLOAT,                       // TF_A32B32G32R32F
	//
	//			GL_NONE,                        // TF_DXT1 (not needed)
	//			GL_NONE,                        // TF_DXT3 (not needed)
	//			GL_NONE,                        // TF_DXT5 (not needed)
	//			GL_UNSIGNED_INT_24_8_EXT,        // TF_D24S8
	//			GL_UNSIGNED_SHORT,
	//			GL_SHORT,
	//			GL_UNSIGNED_SHORT,
	//			GL_BYTE,
	//			GL_BYTE,
	//			GL_UNSIGNED_BYTE,
	//			GL_UNSIGNED_BYTE,
	//			GL_BYTE,
	//			GL_FLOAT,     //TF_DEPTHCOMPONENT
	//			GL_FLOAT,         // TF_A32F
	//			GL_UNSIGNED_INT
	//
	//		};
	//
	//		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//		// check the file signature and deduce its format
	//		// (the second argument is currently not used by FreeImage)
	//		fif = FreeImage_GetFileType(path.c_str(), 0);
	//		if (fif == FIF_UNKNOWN) {
	//			// no signature ?
	//			// try to guess the file format from the file extension
	//			fif = FreeImage_GetFIFFromFilename(path.c_str());
	//			return ;
	//		}
	//
	//		// check that the plugin has reading capabilities ...
	//		if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
	//			// ok, let's load the file
	//			FIBITMAP *dib = FreeImage_Load(fif, path.c_str());
	//
	//			Int32 numBitsPerPixel = FreeImage_GetBPP(dib);
	//			Format pformat;
	//			Int32 imatype = FreeImage_GetImageType(dib);
	//
	//			Int32 dimension0 = FreeImage_GetWidth(dib);
	//			Int32 dimension1 = FreeImage_GetHeight(dib);
	//
	//			{
	//				if (numBitsPerPixel / 8 == 1) {
	//					pformat = TF_UL8;
	//				}
	//				else if (imatype == 2) {
	//					pformat = TF_UL16;
	//				}
	//				else if (imatype == 3) {
	//					pformat = TF_L16_SNORM;
	//				}
	//
	//				else if (imatype == 4) {
	//					pformat = TF_G16R16;
	//				}
	//				else if (imatype == 5) {
	//					pformat = TF_R16G16_SNORM;
	//				}
	//				else if (imatype == 9) {
	//					pformat = TF_RGB16;
	//				}
	//				else if (imatype == 10) {
	//					pformat = TF_A16B16G16R16;
	//				}
	//				else if (imatype == 11) {
	//					pformat = TF_B32G32R32F;
	//				}
	//				else if (imatype == 6) {
	//					pformat = TF_R32F;
	//				}
	//				else if (imatype == 12) {
	//					pformat = TF_A32B32G32R32F;
	//				}
	//				else if (numBitsPerPixel / 8 == 3 || numBitsPerPixel / 16 == 3)
	//				{
	//					pformat = TF_R8G8B8; // BGR
	//				}
	//				else if (numBitsPerPixel / 8 == 4 || numBitsPerPixel / 16 == 4)
	//				{
	//					pformat = TF_A8R8G8B8; // BGRA
	//				}
	//				else
	//				{
	//
	//				}
	//
	//			}
	//			BYTE* bits = nullptr;
	//			Int32  width = FreeImage_GetWidth(dib);
	//			Int32  height = FreeImage_GetHeight(dib);
	//
	//			bits = FreeImage_GetBits(dib);
	//			glBindTexture(GL_TEXTURE_2D, 0);
	//			/*	FIBITMAP *bitmap = FreeImage_Allocate(1024, 1024, 24);
	//				uint8_t laztchar = bits[4096 * 4095 + 4095];
	//				for (Int32 y = 0; y < 1024; y++)
	//				{
	//					BYTE *bits0 = FreeImage_GetScanLine(bitmap, y);
	//					for (Int32 x = 0; x < 1024; x++)
	//					{
	//						bits0[0] = bits[(y * 1024 + x) * 3 + 0];
	//						bits0[1] = bits[(y * 1024 + x) * 3 + 1];
	//						bits0[2] = bits[(y * 1024 + x) * 3 + 2];
	//						bits0 += 3;
	//
	//					}
	//
	//				}
	//				Bool bSuccess = FreeImage_Save(FIF_PNG, bitmap, "testpng.png", PNG_DEFAULT);
	//				FreeImage_Unload(bitmap);*/
	//				//get the image width and height
	//
	//				//if this somehow one of these failed (they shouldn't), return failure
	//			if ((bits == 0) || (width == 0) || (height == 0))
	//				return ;
	//
	//
	//			//UInt32 textureID;
	//			glGenTextures(1, &GPUId);
	//			//bind to the new texture ID
	//			glBindTexture(GL_TEXTURE_2D, GPUId);
	//			//store the texture data for OpenGL use
	//			GLuint internalformat, format, type;
	//			internalformat = gOGLTextureInternalFormat[pformat];
	//			format = gOGLTextureFormat[pformat];
	//			type = gOGLTextureType[pformat];
	//
	//			//glTextureStorage2D(GL_TEXTURE_2D, 1, internalformat, width, height);
	//			//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, bits);
	//			glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0,
	//				format, type, bits);
	//			{
	//				switch (minParm)
	//				{
	//				case Linear:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//					break;
	//				case Nearest:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//					break;
	//				default:
	//					break;
	//				}
	//			}
	//
	//			switch (magParm)
	//			{
	//			case Linear:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//				break;
	//			case Nearest:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//				break;
	//			default:
	//				break;
	//			}
	//
	//			switch (wrapParmU)
	//			{
	//			case Repeat:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//				break;
	//			case Clamp_To_Edge:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//				break;
	//			case Clamp_To_Border:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//				break;
	//			case Mirrored_Repeat:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	//				break;
	//			default:
	//				break;
	//			}
	//
	//			switch (wrapParmV)
	//			{
	//			case Repeat:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//				break;
	//			case Clamp_To_Edge:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//				break;
	//			case Clamp_To_Border:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//				break;
	//			case Mirrored_Repeat:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	//				break;
	//			default:
	//				break;
	//			}
	//			/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/
	//			//Free FreeImage's copy of the data
	//
	//			FreeImage_Unload(dib);
	//
	//			return ;
	//
	//		}
	//		return ;
	//	}
}



Texture::~Texture()
{
	for (UInt32 Mip = 0; Mip < RawTextureDataPtrs.size(); Mip++)
	{
		delete[] RawTextureDataPtrs[Mip]->MipData;
		RawTextureDataPtrs[Mip]->MipData = nullptr;
	}
}


static GLuint gOGLTextureInternalFormat[TF_QUANTITY] =
{
	0,                                  // TF_NONE

	GL_RGB5,                            // TF_R5G6B5
	GL_RGB5_A1,                         // TF_A1R5G5B5
	GL_RGBA4,                           // TF_A4R4G4B4
	GL_ALPHA8,                          // TF_A8
	GL_LUMINANCE8,                      // TF_UL8
	GL_LUMINANCE8_ALPHA8,               // TF_A8L8
	GL_RGB8,                            // TF_R8G8B8
	GL_RGBA8,                           // TF_A8R8G8B8
	GL_RGBA8,                           // TF_A8B8G8R8
	//GL_LUMINANCE16,                  
	GL_R16_SNORM,                                // TF_L16_SNORM
	GL_RG16,                            // TF_G16R16
	GL_RGBA16,                          // TF_A16B16G16R16
	GL_R16F,                            // TF_R16F
	GL_RG16F,                           // TF_G16R16F
	GL_RGBA16F_ARB,                     // TF_A16B16G16R16F
	GL_R32F,                            // TF_R32F
	GL_RGB32F_ARB,                      // TF_B32G32R32F
	GL_RG32F,                           // TF_G32R32F       //ADD BY ZHANG QIXIN.
	GL_RGBA32F_ARB,                     // TF_A32B32G32R32F

	GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   // TF_DXT1
	GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,   // TF_DXT3
	GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,   // TF_DXT5
	GL_DEPTH24_STENCIL8_EXT ,            // TF_D24S8
	GL_R16,
	GL_RG16_SNORM,
	GL_RGB16,
	GL_R8_SNORM,
	GL_R8I,
	GL_R8UI,
	GL_RGB8UI,
	GL_RGBA8_SNORM,
	GL_DEPTH_COMPONENT24,              //TF_DEPTHCOMPONENT
	GL_ALPHA32F_ARB,                     // TF_A32F
	GL_R32UI                            //TF_R32UI
};

static GLuint gOGLTextureFormat[TF_QUANTITY] =
{
	0,                                  // TF_NONE

	GL_BGR,                             // TF_R5G6B5
	GL_RGBA,                            // TF_A1R5G5B5
	GL_RGBA,                            // TF_A4R4G4B4
	GL_ALPHA,                           // TF_A8
	GL_LUMINANCE,                       // TF_L8
	GL_LUMINANCE_ALPHA,                 // TF_A8L8
	GL_BGR,                             // TF_R8G8B8
	GL_BGRA,                            // TF_A8R8G8B8
	GL_RGBA,                            // TF_A8B8G8R8
	GL_RED,                       // TF_L16
	GL_RG,                              // TF_G16R16
	GL_RGBA,                            // TF_A16B16G16R16
	GL_RED,                             // TF_R16F
	GL_RG,                              // TF_G16R16F
	GL_RGBA,                            // TF_A16B16G16R16F
	GL_RED,                             // TF_R32F
	GL_RG,                              // TF_G32R32F
	GL_RGB,                             //TF_B32G32R32F
	GL_RGBA,                            // TF_A32B32G32R32F

	GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   // TF_DXT1
	GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,   // TF_DXT3
	GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,   // TF_DXT5
	GL_DEPTH_STENCIL_EXT,                // TF_D24S8
	GL_RED,
	GL_RG,
	GL_RGB,
	GL_RED,
	GL_RED_INTEGER,
	GL_RED_INTEGER,
	GL_RGB_INTEGER,
	GL_RGBA,
	GL_DEPTH_COMPONENT,     //TF_DEPTHCOMPONENT
	GL_ALPHA,                // TF_A32F
	GL_RED_INTEGER
};

static GLuint gOGLTextureType[TF_QUANTITY] =
{
	0,                              // TF_NONE

	GL_UNSIGNED_SHORT_5_6_5_REV,    // TF_R5G6B5
	GL_UNSIGNED_SHORT_1_5_5_5_REV,  // TF_A1R5G5B5
	GL_UNSIGNED_SHORT_4_4_4_4_REV,  // TF_A4R4G4B4
	GL_UNSIGNED_BYTE,               // TF_A8
	GL_UNSIGNED_BYTE,                                // TF_L8
	GL_UNSIGNED_BYTE,               // TF_A8L8
	GL_UNSIGNED_BYTE,               // TF_R8G8B8
	GL_UNSIGNED_BYTE,               // TF_A8R8G8B8
	GL_UNSIGNED_BYTE,               // TF_A8B8G8R8
	GL_SHORT,
	GL_UNSIGNED_SHORT,              // TF_G16R16
	GL_UNSIGNED_SHORT,              // TF_A16B16G16R16
	GL_HALF_FLOAT_ARB,              // TF_R16F
	GL_HALF_FLOAT_ARB,              // TF_G16R16F
	GL_HALF_FLOAT_ARB,              // TF_A16B16G16R16F
	GL_FLOAT,                       // TF_R32F
	GL_FLOAT,                       // TF_G32R32F
	GL_FLOAT,                       // TF_B32G32R32F
	GL_FLOAT,                       // TF_A32B32G32R32F

	GL_NONE,                        // TF_DXT1 (not needed)
	GL_NONE,                        // TF_DXT3 (not needed)
	GL_NONE,                        // TF_DXT5 (not needed)
	GL_UNSIGNED_INT_24_8_EXT,        // TF_D24S8
	GL_UNSIGNED_SHORT,
	GL_SHORT,
	GL_UNSIGNED_SHORT,
	GL_BYTE,
	GL_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_BYTE,
	GL_FLOAT,     //TF_DEPTHCOMPONENT
	GL_FLOAT,         // TF_A32F
	GL_UNSIGNED_INT

};

void Texture::CreateGPUObject(TextureParameter mipParm, TextureParameter magParm, TextureParameter wrapParmU, TextureParameter wrapParmV)
{
	UInt32 GL_PixelStorageFormat = gOGLTextureInternalFormat[TypeFormat];
	UInt32 GL_PixelDataFormat = gOGLTextureFormat[TypeFormat];
	UInt32 GL_PixelDataType = gOGLTextureType[TypeFormat];

	/*switch (TypeFormat)
	{
	case TextureDataTypeFormat::RGBA8:
		GL_PixelStorageFormat = GL_RGBA8;
		GL_PixelDataFormat = GL_RGBA;
		GL_PixelDataType = GL_UNSIGNED_BYTE;
		break;
	case TextureDataTypeFormat::R8:
		GL_PixelStorageFormat = GL_R8;
		GL_PixelDataFormat = GL_RED;
		GL_PixelDataType = GL_UNSIGNED_BYTE;
		break;
	case TextureDataTypeFormat::RG8:
		GL_PixelStorageFormat = GL_RG8;
		GL_PixelDataFormat = GL_RG;
		GL_PixelDataType = GL_UNSIGNED_BYTE;
		break;
	case TextureDataTypeFormat::RGB8:
		GL_PixelStorageFormat = GL_RGB8;
		GL_PixelDataFormat = GL_RGB;
		GL_PixelDataType = GL_UNSIGNED_BYTE;
	default:
		break;
	}*/
	Int32 NumMip = (Int32) RawTextureDataPtrs.size();
	glGenTextures(1, &GPUId);
	glBindTexture(GL_TEXTURE_2D, GPUId);
	glTexStorage2D(GL_TEXTURE_2D, NumMip, GL_PixelStorageFormat, Width, Height);
	for (Int32 Mip = 0; Mip < NumMip; Mip++)
	{
		glTexSubImage2D(GL_TEXTURE_2D, Mip, 0, 0, RawTextureDataPtrs[Mip]->Width, RawTextureDataPtrs[Mip]->Height, GL_PixelDataFormat, GL_PixelDataType, RawTextureDataPtrs[Mip]->MipData);
	}

	if (NumMip > 1)
	{
		switch (mipParm)
		{
		case Linear_Mip_Nearest:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			break;
		case Nearest_Mip_Nearest:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			break;
		case Linear_Mip_Linear:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			break;
		case Nearest_Mip_Linear:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			break;
		default:
			break;
		}
	}
	else
	{
		switch (mipParm)
		{
		case Linear:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		case Nearest:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;
		default:
			break;
		}
	}

	switch (magParm)
	{
	case Linear:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	case Nearest:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	default:
		break;
	}

	switch (wrapParmU)
	{
	case Repeat:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		break;
	case Clamp_To_Edge:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		break;
	case Clamp_To_Border:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		break;
	case Mirrored_Repeat:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		break;
	default:
		break;
	}

	switch (wrapParmV)
	{
	case Repeat:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		break;
	case Clamp_To_Edge:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		break;
	case Clamp_To_Border:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		break;
	case Mirrored_Repeat:glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		break;
	default:
		break;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::UpdateGPUObjectData()
{
	UInt32 GL_PixelStorageFormat = gOGLTextureInternalFormat[TypeFormat];
	UInt32 GL_PixelDataFormat = gOGLTextureFormat[TypeFormat];
	UInt32 GL_PixelDataType = gOGLTextureType[TypeFormat];

	/*switch (TypeFormat)
	{
	case TextureDataTypeFormat::RGBA8:
		GL_PixelStorageFormat = GL_RGBA8;
		GL_PixelDataFormat = GL_RGBA;
		GL_PixelDataType = GL_UNSIGNED_BYTE;
		break;
	default:
		break;
	}*/

	glBindTexture(GL_TEXTURE_2D, GPUId);
	for (UInt32 Mip = 0; Mip < RawTextureDataPtrs.size(); Mip++)
	{
		glTexSubImage2D(GL_TEXTURE_2D, Mip, 0, 0, RawTextureDataPtrs[Mip]->Width, RawTextureDataPtrs[Mip]->Height, GL_PixelDataFormat, GL_PixelDataType, RawTextureDataPtrs[Mip]->MipData);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

Int32 Texture::GetWidth()
{
	return Width;
}

Int32 Texture::GetHeight()
{
	return Height;
}

std::shared_ptr<RawTextureData> Texture::GetRawDataPtr(Int32 Mip)
{
	return RawTextureDataPtrs[Mip];
}

Int32 Texture::GetNumMip()
{
	return (Int32)RawTextureDataPtrs.size();
}

//void Texture::InitMipRawTextureData(Int32 width, Int32 height, TextureDataTypeFormat typeFormat)
//{
//	Width = width;
//	Height = height;
//	TypeFormat = typeFormat;
//
//	Int32 NumMips = 0;
//	while ((Int32)min(width, height) >> NumMips)
//	{
//		NumMips++;
//	}
//
//	RawTextureDataPtrs = std::vector<std::shared_ptr<RawTextureData>>(NumMips);
//	for (Int32 Mip = 0; Mip < NumMips; Mip++)
//	{
//		RawTextureDataPtrs[Mip] = std::shared_ptr<RawTextureData>(new RawTextureData());
//		Int32 MipTexSizeU = Width >> Mip;
//		Int32 MipTexSizeV = Height >> Mip;
//		RawTextureDataPtrs[Mip]->Width = MipTexSizeU;
//		RawTextureDataPtrs[Mip]->Height = MipTexSizeV;
//
//		switch (typeFormat)
//		{
//		case RGBA8:
//			RawTextureDataPtrs[Mip]->MipData = new UInt8[MipTexSizeU * MipTexSizeV * 4];
//			memset(RawTextureDataPtrs[Mip]->MipData, 0, MipTexSizeU * MipTexSizeV * 4);
//			break;
//		case R8:
//			RawTextureDataPtrs[Mip]->MipData = new UInt8[MipTexSizeU * MipTexSizeV];
//			memset(RawTextureDataPtrs[Mip]->MipData, 0, MipTexSizeU * MipTexSizeV);
//			break;
//		case RG8:
//			RawTextureDataPtrs[Mip]->MipData = new UInt8[MipTexSizeU * MipTexSizeV * 2];
//			memset(RawTextureDataPtrs[Mip]->MipData, 0, MipTexSizeU * MipTexSizeV * 2);
//			break;
//		default:
//			break;
//		}		
//	}
//}
//
//void Texture::InitNoMipRawTextureData(Int32 width, Int32 height, TextureDataTypeFormat typeFormat)
//{
//	Width = width;
//	Height = height;
//	TypeFormat = typeFormat;
//	RawTextureDataPtrs = std::vector<std::shared_ptr<RawTextureData>>(1);
//	RawTextureDataPtrs[0] = std::shared_ptr<RawTextureData>(new RawTextureData());
//	RawTextureDataPtrs[0]->Width = Width;
//	RawTextureDataPtrs[0]->Height = Height;
//
//	switch (typeFormat)
//	{
//	case RGBA8:
//		RawTextureDataPtrs[0]->MipData = new UInt8[Width * Height * 4];
//		memset(RawTextureDataPtrs[0]->MipData, 0, Width * Height * 4);
//		break;
//	case R8:
//		RawTextureDataPtrs[0]->MipData = new UInt8[Width * Height];
//		memset(RawTextureDataPtrs[0]->MipData, 0, Width* Height);
//		break;
//	case RG8:
//		RawTextureDataPtrs[0]->MipData = new UInt8[Width * Height * 2];
//		memset(RawTextureDataPtrs[0]->MipData, 0, Width * Height * 2);
//		break;
//	default:
//		break;
//	}
//}
FREE_IMAGE_FORMAT Texture::GetFileType(std::string const& file)
{
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	// check the file signature and deduce its format
	// (the second argument is currently not used by FreeImage)
	fif = FreeImage_GetFileType(file.c_str(), 0);
	if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif))
	{
		return fif;
	}
	return FIF_UNKNOWN;
}
TextureDataTypeFormat Texture::GetTextureType(FIBITMAP *dib)
{
	Int32 numBitsPerPixel = FreeImage_GetBPP(dib);
	TextureDataTypeFormat pformat;
	Int32 imatype = FreeImage_GetImageType(dib);

	Int32 dimension0 = FreeImage_GetWidth(dib);
	Int32 dimension1 = FreeImage_GetHeight(dib);

	{
		if (numBitsPerPixel / 8 == 1) {
			pformat = TF_UL8;
		}
		else if (imatype == 2) {
			pformat = TF_UL16;
		}
		else if (imatype == 3) {
			pformat = TF_L16_SNORM;
		}

		else if (imatype == 4) {
			pformat = TF_G16R16;
		}
		else if (imatype == 5) {
			pformat = TF_R16G16_SNORM;
		}
		else if (imatype == 9) {
			pformat = TF_RGB16;
		}
		else if (imatype == 10) {
			pformat = TF_A16B16G16R16;
		}
		else if (imatype == 11) {
			pformat = TF_B32G32R32F;
		}
		else if (imatype == 6) {
			pformat = TF_R32F;
		}
		else if (imatype == 12) {
			pformat = TF_A32B32G32R32F;
		}
		else if (numBitsPerPixel / 8 == 3 || numBitsPerPixel / 16 == 3)
		{
			pformat = TF_R8G8B8; // BGR
		}
		else if (numBitsPerPixel / 8 == 4 || numBitsPerPixel / 16 == 4)
		{
			pformat = TF_A8R8G8B8; // BGRA
		}
		else
		{

		}

	}
	return pformat;
}
void Texture::LoadTextureFromAsset(std::string const&file)
{
	Int32 width, height, nrComponents;


	FREE_IMAGE_FORMAT fif = GetFileType(file);
	if (fif != FIF_UNKNOWN) {
		// ok, let's load the file
		FIBITMAP *dib = FreeImage_Load(fif, file.c_str());

		Int32 numBitsPerPixel = FreeImage_GetBPP(dib);
		TextureDataTypeFormat pformat = GetTextureType(dib);
		TypeFormat = pformat;
		BYTE* bits = FreeImage_GetBits(dib);
		Width = FreeImage_GetWidth(dib);;
		Height = FreeImage_GetHeight(dib);;
		RawTextureDataPtrs.reserve(1);
		RawTextureDataPtrs.resize(1);

		Int32 MipWidth = Width;
		Int32 MipHeight = Height;
		RawTextureDataPtrs[0] = std::shared_ptr<RawTextureData>(new RawTextureData());
		RawTextureDataPtrs[0]->Width = MipWidth;
		RawTextureDataPtrs[0]->Height = MipHeight;
		UInt32 size = MipWidth * MipHeight * numBitsPerPixel / 8;
		RawTextureDataPtrs[0]->MipData = new UInt8[size];
		memcpy(RawTextureDataPtrs[0]->MipData, bits/*data*/, size);
		FreeImage_Unload(dib);
		//stbi_image_free(data);
	}



}
//UInt32 Texture::CreateCompositeTexture(Float32 compositepower, Bool useGPU, Channel AddToChanel, std::string compositeTexFilePath)
//{
//	Int32 width, height, nrComponents;
//	if (compositeTexFilePath.empty())
//	{
//		return 0;
//	}
//	//Int32 num_cpu = std::thread::hardware_concurrency();
//	if (useGPU)
//	{
//		if (!glIsTexture(this->GPUId))
//		{
//			LoadTextureFromAsset(AssetFolderPath + compositeTexFilePath);
//			CreateGPUObject(Linear_Mip_Linear, Linear_Mip_Linear, Clamp_To_Edge, Clamp_To_Edge);
//
//		}
//		std::shared_ptr<Framebuffer2D> composite_RT = std::make_shared<Framebuffer2D>(Width, Height);
//		composite_RT->attachBuffer(FBO_COLOR0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR);
//
//		UInt32 quadvao;
//		{
//			typedef struct
//			{
//				Vector3f position;
//				Vector2f texCoord;
//			}QuadVertex;
//
//			std::vector<QuadVertex> vertices;
//			std::vector<glm::uvec3> triangles;
//
//
//			QuadVertex v;
//
//			v.position = Vector3f(-1, -1, 0); v.texCoord = Vector2f(0, 0); vertices.push_back(v);
//			v.position = Vector3f(1, -1, 0);  v.texCoord = Vector2f(1, 0); vertices.push_back(v);
//			v.position = Vector3f(1, 1, 0);   v.texCoord = Vector2f(1, 1); vertices.push_back(v);
//			v.position = Vector3f(-1, 1, 0);  v.texCoord = Vector2f(0, 1); vertices.push_back(v);
//
//
//			triangles.push_back(glm::uvec3(0, 1, 2));
//			triangles.push_back(glm::uvec3(0, 2, 3));
//
//			GLuint vbo_vertex, vbo_triangle;
//			glGenVertexArrays(1, &quadvao);
//			glBindVertexArray(quadvao);
//			glGenBuffers(1, &vbo_vertex);
//			glGenBuffers(1, &vbo_triangle);
//
//			glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex);
//
//			//	GLint posLoc = 0, GLint texLoc = 1;
//			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(QuadVertex), &vertices[0].position[0], GL_STATIC_DRAW);
//			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (Int8*)NULL);
//			glEnableVertexAttribArray(0);
//			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (Int8*)NULL + 3 * sizeof(glm::f32));
//			glEnableVertexAttribArray(1);
//
//			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_triangle);
//			glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(glm::uvec3), &triangles[0][0], GL_STATIC_DRAW);
//			glBindVertexArray(0);
//
//			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//			glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//		}
//		std::shared_ptr<Material> composite_shader = std::shared_ptr<Material>(new Material(std::vector<std::string> {"PassQuad.vsh", "createCompositeTex.fsh"}));
//		std::shared_ptr<MaterialInstance> composite_instance = std::shared_ptr<MaterialInstance>(new MaterialInstance(composite_shader));
//
//		composite_instance->SetTextureID("normaltex", this->GPUId);
//		composite_instance->SetUniform("addToChanel", (Int32)AddToChanel);
//		composite_instance->SetUniform("compositepower", compositepower);
//
//		glClear(GL_COLOR_BUFFER_BIT);
//		GLint PrevViewport[4];
//		glGetIntegerv(GL_VIEWPORT, PrevViewport);
//		composite_shader->BindProgram();
//		composite_shader->BindSamplers();
//		composite_shader->BindUniforms();
//		composite_RT->bind();
//		glClearColor(0.0, 0.0, 0.0, 0.0);
//		if (quadvao)
//		{
//			glBindVertexArray(quadvao);
//			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//			glBindVertexArray(0);
//		}
//		composite_RT->unbind();
//		composite_shader->UnBindProgram();
//
//
//
//		// read back from gpu to verify
//		/*glm::i8vec4* m_pBGRBuf = new glm::i8vec4[composite_RT->getWidth()*composite_RT->getWidth()];
//		glBindTexture(GL_TEXTURE_2D, composite_RT->getBufferHandle(FBO_COLOR0));
//		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pBGRBuf);
//		FIBITMAP *bitmap = FreeImage_Allocate(composite_RT->getWidth(), composite_RT->getWidth(), 24);
//		for (Int32 y = 0; y < composite_RT->getWidth(); y++)
//		{
//			BYTE *bits0 = FreeImage_GetScanLine(bitmap, y);
//			for (Int32 x = 0; x < composite_RT->getWidth(); x++)
//			{
//				auto colorM = m_pBGRBuf[y*composite_RT->getWidth() + x];
//				bits0[0] = colorM.r;
//				bits0[1] = colorM.g;
//				bits0[2] = colorM.b;
//
//				bits0 += 3;
//			}
//		}
//		Bool bSuccess = FreeImage_Save(FIF_TIFF, bitmap, "Toksvig_mesoNormal.tif", TIFF_DEFAULT);
//		FreeImage_Unload(bitmap);*/
//		glViewport(PrevViewport[0], PrevViewport[1], PrevViewport[2], PrevViewport[3]);
//		return composite_RT->getBufferHandle(FBO_COLOR0);
//	}
//	UInt8 *composite_tex_data = stbi_load((AssetFolderPath + compositeTexFilePath).c_str(), &width, &height, &nrComponents, 0);
//	RawTextureDataPtrs.reserve(1);
//	RawTextureDataPtrs.resize(1);
//	Int32 MipWidth = width;
//	Int32 MipHeight = height;
//	RawTextureDataPtrs[0] = std::shared_ptr<RawTextureData>(new RawTextureData());
//	RawTextureDataPtrs[0]->Width = MipWidth;
//	RawTextureDataPtrs[0]->Height = MipHeight;
//	TypeFormat = TextureDataTypeFormat::TF_R8G8B8;
//	RawTextureDataPtrs[0]->MipData = new UInt8[MipWidth * MipHeight * 3];
//
//
//	FImageKernel2D KernelDownsample;
//	// /2 as input resolution is same as output resolution and the settings assumed the output is half resolution
//	KernelDownsample.BuildSeparatableGaussWithSharpen(2, -4.0);
//	const Int32 KernelCenter = (Int32)KernelDownsample.GetFilterTableSize() / 2 - 1;
//	auto GetSafeNormal = [](glm::dvec3 Normal, Float32 Tolerance) ->glm::dvec3
//	{
//		Float32 X = Normal.x;
//		Float32 Y = Normal.y;
//		Float32 Z = Normal.z;
//		const Float32 SquareSum = X * X + Y * Y + Z * Z;
//
//		// Not sure if it's safe to add tolerance in there. Might introduce too many errors
//		if (SquareSum == 1.f)
//		{
//			return Normal;
//		}
//		else if (SquareSum < Tolerance)
//		{
//			return glm::dvec3(0.0);
//		}
//		const Float32 Scale = glm::inversesqrt(SquareSum);
//		return glm::dvec3(X*Scale, Y*Scale, Z*Scale);
//	};
//	const Float32 SMALL_NUMBER = (1.e-8f);
//	for (Int32 y = 0; y < height; y++)
//	{
//		for (Int32 x = 0; x < width; x++)
//		{
//			Int32 SourceX = x;
//			Int32 SourceY = y;
//			glm::dvec3 FilteredColor = Vector3f(0.0);
//
//			for (UInt32 KernelY = 0; KernelY < KernelDownsample.GetFilterTableSize(); ++KernelY)
//			{
//				for (UInt32 KernelX = 0; KernelX < KernelDownsample.GetFilterTableSize(); ++KernelX)
//				{
//					Float32 Weight = KernelDownsample.GetAt(KernelX, KernelY);
//					Int32 samplex = SourceX + KernelX - KernelCenter;
//					Int32 sampley = SourceY + KernelY - KernelCenter;
//					samplex = (Int32)((UInt32)samplex) & (width - 1);
//					sampley = (Int32)((UInt32)sampley) & (height - 1);
//					Int32 r0 = composite_tex_data[((height - sampley)* width + samplex) * 3 + 0];
//					Int32 g0 = composite_tex_data[((height - sampley)* width + samplex) * 3 + 1];
//					Int32 b0 = composite_tex_data[((height - sampley)* width + samplex) * 3 + 2];
//
//					Float32 testvarR = (((Float32)(r0) / 255.0f));
//					Float32 testvarG = (((Float32)(g0) / 255.0f));
//					Float32 testvarB = (((Float32)(b0) / 255.0f));
//					glm::dvec3 Normal = glm::dvec3(testvarB, testvarG, testvarR);
//					Normal = glm::dvec3(Normal.r * 2.0f - 1.0f, Normal.g * 2.0f - 1.0f, Normal.b * 2.0f - 1.0f);
//					Normal = GetSafeNormal(Normal, SMALL_NUMBER);
//					Normal = glm::dvec3(Normal.r * 0.5 + 0.5, Normal.g * 0.5 + 0.5, Normal.b * 0.5 + 0.5);
//					FilteredColor += glm::dvec3(Weight) * Normal;
//				}
//			}
//			glm::dvec3 Normal = FilteredColor;
//			Float32 power = compositepower;
//			Normal.z = Normal.z*2.0 - 1.0;
//			Normal.x = Normal.x*2.0 - 1.0;
//			Normal.y = Normal.y*2.0 - 1.0;
//
//			Float32 ns = glm::length(Normal);
//
//			Float32 lengthN = ns < 1.0 ? ns : 1.0;
//
//			Float32 Variance = (1.0f - lengthN) / lengthN;
//
//			Variance = 0.0 > (Variance - 0.00004) ? 0.0 : (Variance - 0.00004);
//			Variance *= power;
//
//			Float32 rougnness = 0.0;
//
//			Float32 B0 = 2.0 *Variance*(-1.0);
//
//			Float32 a2 = B0 / (B0 - 1.0);
//
//			rougnness = std::pow(a2, 0.25);
//
//			Int32 colorg = (Int32)(255.0 *(rougnness));
//			Int32 index = ((y)* width + x) * 3;
//			RawTextureDataPtrs[0]->MipData[index + 0] = 0;
//			RawTextureDataPtrs[0]->MipData[index + 1] = 0;
//			RawTextureDataPtrs[0]->MipData[index + 2] = 0;
//			RawTextureDataPtrs[0]->MipData[index + AddToChanel] = colorg;
//			//bits0[0] = RawTextureDataPtrs[0]->MipData[index + 0]; composite_tex_data[((y)* width + x) * 3 + 0];
//			//bits0[1] = RawTextureDataPtrs[0]->MipData[index + 1]; composite_tex_data[((y)* width + x) * 3 + 1];
//			//bits0[2] = RawTextureDataPtrs[0]->MipData[index + 2]; composite_tex_data[((y)* width + x) * 3 + 2];
//			////bits0[3] = 0; (Int32)(255.0 *(testvarB));
//			//bits0 += 3;
//		}
//	}
//	CreateGPUObject(Linear_Mip_Linear, Linear_Mip_Linear, Clamp_To_Edge, Clamp_To_Edge);
//	stbi_image_free(composite_tex_data);
//	return GPUId;
//	//stbi_write_png(saveFile.c_str(), Width, Height, Channels, Output, 0);
//
//}