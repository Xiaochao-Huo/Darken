#pragma once
#include "MaterialInstance.h"
#include "RectBufferObject.h"
#include<unordered_map>
#include<vector>
#include "glm.hpp"
#include <fstream>
#include <sstream>
#include <iostream>


struct Math
{
	template<typename T, typename U>
	static T Lerp(const T& A, const T& B, const U& alpha)
	{
		return (T)(A + alpha * (B - A));
	}

	static int GeometryDis(const glm::vec2& A, const glm::vec2& B)
	{
		return int(abs(A.x - B.x) + abs(A.y - B.y));
	}

	static bool IsNearlyEqual(const glm::vec2& A, const glm::vec2& B, const float th)
	{
		return abs(A.x - B.x) < th && abs(A.y - B.y) < th;
	}

	static bool IsNearlyEqual(const glm::vec3& A, const glm::vec3& B, const float th)
	{
		return abs(A.x - B.x) < th && abs(A.y - B.y) < th && abs(A.z - B.z) < th;
	}
};

template <typename T>
T ReverseBits(T Bits)
{
	Bits = (Bits << 16) | (Bits >> 16);
	Bits = ((Bits & 0x00ff00ff) << 8) | ((Bits & 0xff00ff00) >> 8);
	Bits = ((Bits & 0x0f0f0f0f) << 4) | ((Bits & 0xf0f0f0f0) >> 4);
	Bits = ((Bits & 0x33333333) << 2) | ((Bits & 0xcccccccc) >> 2);
	Bits = ((Bits & 0x55555555) << 1) | ((Bits & 0xaaaaaaaa) >> 1);
	return Bits;
}

static std::string ReadSourceCode(std::string souceCodePath)
{
	std::ifstream ifile(souceCodePath);
	//将文件读入到ostringstream对象buf中
	std::ostringstream buf;
	char ch;
	while (buf&&ifile.get(ch))
		buf.put(ch);
	//返回与流对象buf关联的字符串
	return buf.str();
}

class Util
{
public:
	Util() {};
	~Util() {};

	static void FlipTex2DY_Once(unsigned int TextureID, unsigned int Width, unsigned int Height, unsigned int InternalFormat, unsigned int glTexFormat, unsigned int glTexType)
	{
		unsigned int RenderTex;
		glGenTextures(1, &RenderTex);
		glBindTexture(GL_TEXTURE_2D, RenderTex);
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, glTexFormat, glTexType, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		unsigned int FrameBuffer;
		glGenFramebuffers(1, &FrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTex, 0);
		unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachments);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "FrameBufferError" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		std::shared_ptr<Material> FlipYMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> { "DrawRectVertShader.vsh", "FlipTextureYFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> FlipYMaterialInst = std::shared_ptr<MaterialInstance>(new MaterialInstance(FlipYMaterial));
		std::shared_ptr<RectBufferObject> QuadBufferObject = std::shared_ptr<RectBufferObject>(new RectBufferObject());
		FlipYMaterialInst->SetTextureID("MainTex", TextureID);
		FlipYMaterialInst->GetParent()->Draw(QuadBufferObject->VAO, QuadBufferObject->NumFaces, QuadBufferObject->IndexType);
		
		glBindTexture(GL_TEXTURE_2D, TextureID);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, 0, 0, Width, Height, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDeleteFramebuffers(1, &FrameBuffer);
		glDeleteTextures(1, &RenderTex);
	}

	static void CopyTex2D_Once(unsigned int SrcTextureID, unsigned int DsetTextureID, int DestTexWidth, int DestTexHeight)
	{
		unsigned int FrameBuffer;
		glGenFramebuffers(1, &FrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, DsetTextureID, 0);
		unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachments);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "FrameBufferError" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		std::shared_ptr<Material> CopyMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> { "DrawRectVertShader.vsh", "DrawRectFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> CopyMaterialInst = std::shared_ptr<MaterialInstance>(new MaterialInstance(CopyMaterial));
		std::shared_ptr<RectBufferObject> QuadBufferObject = std::shared_ptr<RectBufferObject>(new RectBufferObject());
		CopyMaterialInst->SetTextureID("MainTex", SrcTextureID);
		glViewport(0, 0, DestTexWidth, DestTexHeight);
		CopyMaterialInst->GetParent()->Draw(QuadBufferObject->VAO, QuadBufferObject->NumFaces, QuadBufferObject->IndexType);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDeleteFramebuffers(1, &FrameBuffer);
	}

	static void CopyTex2DToCubeFace_Once(unsigned int SrcTextureID, unsigned int DsetTextureID, unsigned int InternalFormat, unsigned int glTexFormat, unsigned int glTexType, int DestTexWidth, int DestTexHeight, int FaceIndex)
	{
		unsigned int RenderTex;
		glGenTextures(1, &RenderTex);
		glBindTexture(GL_TEXTURE_2D, RenderTex);
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, DestTexWidth, DestTexHeight, 0, glTexFormat, glTexType, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		unsigned int FrameBuffer;
		glGenFramebuffers(1, &FrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTex, 0);
		unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachments);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "FrameBufferError" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		std::shared_ptr<Material> CopyMaterial = std::shared_ptr<Material>(new Material(std::vector<std::string> { "DrawRectVertShader.vsh", "DrawRectFragShader.fsh" }));
		std::shared_ptr<MaterialInstance> CopyMaterialInst = std::shared_ptr<MaterialInstance>(new MaterialInstance(CopyMaterial));
		std::shared_ptr<RectBufferObject> QuadBufferObject = std::shared_ptr<RectBufferObject>(new RectBufferObject());
		CopyMaterialInst->SetTextureID("MainTex", SrcTextureID);
		glViewport(0, 0, DestTexWidth, DestTexHeight);
		CopyMaterialInst->GetParent()->Draw(QuadBufferObject->VAO, QuadBufferObject->NumFaces, QuadBufferObject->IndexType);

		glBindTexture(GL_TEXTURE_CUBE_MAP, DsetTextureID);
		glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + FaceIndex, 0, InternalFormat, 0, 0, DestTexWidth, DestTexHeight, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDeleteFramebuffers(1, &FrameBuffer);
	}

private:

};