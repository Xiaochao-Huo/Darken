#pragma once
#include "MaterialInstance.h"
#include "RectBufferObject.h"
#include <unordered_map>
#include <vector>
#include "glm.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include "TypeDefine.h"


struct Math
{
	template<typename T, typename U>
	static T Lerp(const T& A, const T& B, const U& alpha)
	{
		return (T)(A + alpha * (B - A));
	}

	static Int32 GeometryDis(const Vector2f& A, const Vector2f& B)
	{
		return Int32(abs(A.x - B.x) + abs(A.y - B.y));
	}

	static Bool IsNearlyEqual(const Vector2f& A, const Vector2f& B, const Float32 th)
	{
		return abs(A.x - B.x) < th && abs(A.y - B.y) < th;
	}

	static Bool IsNearlyEqual(const Vector3f& A, const Vector3f& B, const Float32 th)
	{
		return abs(A.x - B.x) < th && abs(A.y - B.y) < th && abs(A.z - B.z) < th;
	}

	static Int32 Floor(const Float32 &A)
	{
		return (Int32) glm::floor(A);
	}

	static Int32 Ceil(const Float32 &A)
	{
		return (Int32) glm::ceil(A);
	}

	static Float32 Fract(const Float32 &A)
	{
		return glm::fract(A);
	}

	template<typename T>
	static T Clamp(const T& A, const T& Min, const T& Max)
	{
		return glm::clamp(A, Min, Max);
	}

	static Float32 Pow(const Float32& A, const Float32& Power)
	{
		return glm::pow(A, Power);
	}

	template<typename T>
	static T Inverse(const T& A)
	{
		return glm::inverse(A);
	}

	template<typename T>
	static T Transpose(const T& A)
	{
		return glm::transpose(A);
	}

	static Vector3f Normalize(const Vector3f& A)
	{
		return glm::normalize(A);
	}

	static Float32 Radians(const Float32&A)
	{
		return glm::radians(A);
	}

	static Float32 Degrees(const Float32& A)
	{
		return glm::degrees(A);
	}

	static Float32 Tan(const Float32& A)
	{
		return glm::tan(A);
	}

	static Float32 Exp(const Float32& A)
	{
		return glm::exp(A);
	}

	static Float32 Sin(const Float32& A)
	{
		return glm::sin(A);
	}

	static Float32 Cos(const Float32& A)
	{
		return glm::cos(A);
	}

	static Float32 aCos(const Float32& A)
	{
		return glm::acos(A);
	}

	static Float32 Log2(const Float32& A)
	{
		return glm::log2(A);
	}

	template<typename T>
	static T Min(const T& A, const T & B)
	{
		return glm::min(A, B);
	}

	template<typename T>
	static T Max(const T& A, const T& B)
	{
		return glm::max(A, B);
	}

	template<typename T>
	static T Sqrt(const T& A)
	{
		return glm::sqrt(A);
	}

	static Float32 Dot(const Vector2f & A, const Vector2f &B)
	{
		return glm::dot(A, B);
	}

	static Float32 Dot(const Vector3f& A, const Vector3f& B)
	{
		return glm::dot(A, B);
	}

	static Float32 Dot(const Vector4f& A, const Vector4f& B)
	{
		return glm::dot(A, B);
	}

	static Vector3f Cross(const Vector3f& A, const Vector3f& B)
	{
		return glm::cross(A, B);
	}

	static Float32 Distance(const Vector2f& A, const Vector2f& B)
	{
		return glm::distance(A, B);
	}

	static Float32 Distance(const Vector3f& A, const Vector3f& B)
	{
		return glm::distance(A, B);
	}

	static Float32 Distance(const Vector4f& A, const Vector4f& B)
	{
		return glm::distance(A, B);
	}

	static Float32 Length(const Vector2f& A)
	{
		return glm::length(A);
	}

	static Float32 Length(const Vector3f& A)
	{
		return glm::length(A);
	}

	static Float32 Length(const Vector4f& A)
	{
		return glm::length(A);
	}

	static Mat4f Translate(const Mat4f& A, const Vector3f& position)
	{
		return glm::translate(A, position);
	}

	static Mat4f Rotate(const Mat4f& A, const Float32& angle,  const Vector3f& axis)
	{
		return glm::rotate(A, angle, axis);
	}

	static Mat4f Scale(const Mat4f& A, const Vector3f& scale)
	{
		return glm::scale(A, scale);
	}
};

struct CameraUtil
{
	static Mat4f LookAt(const Vector3f& Position, const Vector3f& Center, const Vector3f& Upward)
	{
		return glm::lookAtLH(Position, Center, Upward);
	}

	static Mat4f Perspective(const Float32& Fovy, const Float32& Aspect, const Float32& NearPlane, const Float32& FarPlane)
	{
		return glm::perspectiveLH(Fovy, Aspect, NearPlane, FarPlane);
	}
	
	static Mat4f Frustum(const Float32& Left, const Float32& Right, const Float32& Bottom, const Float32& Top, const Float32& Near, const Float32& Far)
	{
		return glm::frustum(Left, Right, Bottom, Top, Near, Far);
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
	Int8 ch;
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

	static void FlipTex2DY_Once(UInt32 TextureID, UInt32 Width, UInt32 Height, UInt32 InternalFormat, UInt32 glTexFormat, UInt32 glTexType)
	{
		UInt32 RenderTex;
		glGenTextures(1, &RenderTex);
		glBindTexture(GL_TEXTURE_2D, RenderTex);
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, glTexFormat, glTexType, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		UInt32 FrameBuffer;
		glGenFramebuffers(1, &FrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTex, 0);
		UInt32 attachments[1] = { GL_COLOR_ATTACHMENT0 };
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

	static void CopyTex2D_Once(UInt32 SrcTextureID, UInt32 DsetTextureID, Int32 DestTexWidth, Int32 DestTexHeight)
	{
		UInt32 FrameBuffer;
		glGenFramebuffers(1, &FrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, DsetTextureID, 0);
		UInt32 attachments[1] = { GL_COLOR_ATTACHMENT0 };
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

	static void CopyTex2DToCubeFace_Once(UInt32 SrcTextureID, UInt32 DsetTextureID, UInt32 InternalFormat, UInt32 glTexFormat, UInt32 glTexType, Int32 DestTexWidth, Int32 DestTexHeight, Int32 FaceIndex)
	{
		UInt32 RenderTex;
		glGenTextures(1, &RenderTex);
		glBindTexture(GL_TEXTURE_2D, RenderTex);
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, DestTexWidth, DestTexHeight, 0, glTexFormat, glTexType, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		UInt32 FrameBuffer;
		glGenFramebuffers(1, &FrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTex, 0);
		UInt32 attachments[1] = { GL_COLOR_ATTACHMENT0 };
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