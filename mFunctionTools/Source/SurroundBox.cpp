#include "SurroundBox.h"

SphereBox::SphereBox()
{
	PointMin = glm::vec4(FLT_MAX, FLT_MAX, FLT_MAX, 1.0);
	PointMax = glm::vec4(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0);
	CenterLocal = glm::vec4(0.0, 0.0, 0.0, 1.0);
	Radius = 1.0;
}

SphereBox::~SphereBox()
{
}

void SphereBox::Reset()
{
	PointMin = glm::vec4(FLT_MAX, FLT_MAX, FLT_MAX, 1.0);
	PointMax = glm::vec4(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0);
	CenterLocal = glm::vec4(0.0, 0.0, 0.0, 1.0);
	Radius = 1.0;
}

bool SphereBox::bOutOfCamera(glm::mat4 ModelMatrix, glm::mat4 VPMatrix, float scale)
{
	glm::mat4 MVP_Matrix_T = glm::transpose(VPMatrix);
	glm:: vec4 centerLocal = ModelMatrix * CenterLocal;
	ClipPlanesLocal[0] = MVP_Matrix_T[0] + MVP_Matrix_T[3]; //left
	ClipPlanesLocal[1] = MVP_Matrix_T[3] - MVP_Matrix_T[0]; //right
	ClipPlanesLocal[2] = MVP_Matrix_T[1] + MVP_Matrix_T[3]; //bottom
	ClipPlanesLocal[3] = MVP_Matrix_T[3] - MVP_Matrix_T[1]; //top
	ClipPlanesLocal[4] = MVP_Matrix_T[2] + MVP_Matrix_T[3]; //near
	ClipPlanesLocal[5] = MVP_Matrix_T[3] - MVP_Matrix_T[2]; //far
	for (int i = 0; i < 6; i++)
	{
		float dot = glm::dot(centerLocal, ClipPlanesLocal[i]);
		if (dot > 0.001f)
		{	
			continue;
		} 
		else
		{
			float distance = -dot / glm::sqrt(ClipPlanesLocal[i].x * ClipPlanesLocal[i].x 
										    + ClipPlanesLocal[i].y * ClipPlanesLocal[i].y 
										    + ClipPlanesLocal[i].z * ClipPlanesLocal[i].z);
			if (distance > Radius * scale)
			{
				return true;
			}
		}
		
	}
	return false;
}

SphereBox& SphereBox::operator+=(const glm::vec3& newVertex)
{
	PointMin.x = glm::min(PointMin.x, newVertex.x);
	PointMin.y = glm::min(PointMin.y, newVertex.y);
	PointMin.z = glm::min(PointMin.z, newVertex.z);

	PointMax.x = glm::max(PointMax.x, newVertex.x);
	PointMax.y = glm::max(PointMax.y, newVertex.y);
	PointMax.z = glm::max(PointMax.z, newVertex.z);

	CenterLocal = (PointMin + PointMax) / 2.0f;
	Radius = glm::distance(CenterLocal, PointMax);
	Height = PointMax.z - PointMin.z;

	return *this;
}

glm::vec3 SphereBox::GetCenterPointLocal()
{
	return glm::vec3(CenterLocal);
}

float SphereBox::GetRadius()
{
	return Radius;
}

RectBox::RectBox()
{

}

RectBox::~RectBox()
{
}

void RectBox::UpdateBoxModel(glm::uint32 vertexNum, glm::uint8 * pointPtr, glm::uint8 stride)
{
	float x, y, z;
	for (unsigned int i = 0; i < vertexNum; ++i)
	{
		x = *(float*)(pointPtr + i * stride);
		y = *(float*)(pointPtr + i * stride + 4);
		z = *(float*)(pointPtr + i * stride + 8);

		if (x < PointMin.x) PointMin.x = x;
		if (x > PointMax.x) PointMax.x = x;
		if (y < PointMin.y) PointMin.y = y;
		if (y > PointMax.y) PointMax.y = y;
		if (z < PointMin.z) PointMin.z = z;
		if (z > PointMax.z) PointMax.z = z;
	}
	CenterLocal = (PointMin + PointMax) / 2.0f;
	createCornerPointsModel();
	clipPlaneModel[0] = glm::vec4(1.0, 0.0, 0.0, -PointMin.x); //left
	clipPlaneModel[1] = glm::vec4(-1.0, 0.0, 0.0, PointMax.x); //right
	clipPlaneModel[2] = glm::vec4(0.0, 1.0, 0.0, -PointMin.y); //bottom
	clipPlaneModel[3] = glm::vec4(0.0, -1.0, 0.0, PointMax.y); //top
	clipPlaneModel[4] = glm::vec4(0.0, 0.0, 1.0, -PointMin.z); //near
	clipPlaneModel[5] = glm::vec4(0.0, 0.0, -1.0, PointMax.z); //far
}

RectBox& RectBox::operator += (const glm::vec3& newVertex)
{
	PointMin.x = glm::min(PointMin.x, newVertex.x);
	PointMin.y = glm::min(PointMin.y, newVertex.y);
	PointMin.z = glm::min(PointMin.z, newVertex.z);

	PointMax.x = glm::max(PointMax.x, newVertex.x);
	PointMax.y = glm::max(PointMax.y, newVertex.y);
	PointMax.z = glm::max(PointMax.z, newVertex.z);

	CenterLocal = (PointMin + PointMax) / 2.0f;
	BoxSize = glm::vec3(PointMax - PointMin);

	return *this;
}

//void RectBox::UpdateBoxWorld(glm::mat4 & ModelMatrix)
//{
//	for (int i = 0; i < 8; ++i)
//	{
//		pointCornersWorld[i] = ModelMatrix * pointCornersModel[i];
//	}
//	for (int i = 0; i < 6; ++i)
//	{
//		clipPlaneWorld[i] = ModelMatrix * clipPlaneModel[i];
//	}
//	CenterWorld = ModelMatrix * CenterModel;
//}

//void RectBox::CreateBoxFromCornerWorld(float Xmin, float Xmax, float Ymin, float Ymax, float Zmin, float Zmax)
//{
//	pointMin.x = Xmin;
//	pointMin.y = Ymin;
//	pointMin.z = Zmin;
//	pointMax.x = Xmax;
//	pointMax.y = Ymax;
//	pointMax.z = Zmax;
//	CenterWorld = (pointMin + pointMax) / 2.0f;
//	createCornerPointsWorld();
//	clipPlaneWorld[0] = glm::vec4(1.0, 0.0, 0.0, -Xmin); //left
//	clipPlaneWorld[1] = glm::vec4(-1.0, 0.0, 0.0, Xmax); //right
//	clipPlaneWorld[2] = glm::vec4(0.0, 1.0, 0.0, -Ymin); //bottom
//	clipPlaneWorld[3] = glm::vec4(0.0, -1.0, 0.0, Ymax); //top
//	clipPlaneWorld[4] = glm::vec4(0.0, 0.0, 1.0, -Zmin); //near
//	clipPlaneWorld[5] = glm::vec4(0.0, 0.0, -1.0, Zmax); //far
//}
/*!****************************************************************************
@Function		HitBox
@Output		HitState		NoHit,	Cross,	Inside,	OutSurround
@Return		int			HitState
@Description	check Hit State, Inside means leftBox is inside Box, 
OutSurround means Box is inside leftBox
******************************************************************************/
//int RectBox::HitBox(RectBox & Box)
//{
//	int sSize = 0;
//	int bSize = 0;
//	for (int i = 0; i < 6; ++i){
//		bSize = 0;
//		if (glm::dot(Box.clipPlaneWorld[i], pointCornersWorld[0]) <= 0) sSize++;	else bSize++;
//		if (glm::dot(Box.clipPlaneWorld[i], pointCornersWorld[1]) <= 0) sSize++;	else bSize++;
//		if (glm::dot(Box.clipPlaneWorld[i], pointCornersWorld[2]) <= 0) sSize++;	else bSize++;
//		if (glm::dot(Box.clipPlaneWorld[i], pointCornersWorld[3]) <= 0) sSize++;	else bSize++;
//		if (glm::dot(Box.clipPlaneWorld[i], pointCornersWorld[4]) <= 0) sSize++;	else bSize++;
//		if (glm::dot(Box.clipPlaneWorld[i], pointCornersWorld[5]) <= 0) sSize++;	else bSize++;
//		if (glm::dot(Box.clipPlaneWorld[i], pointCornersWorld[6]) <= 0) sSize++;	else bSize++;
//		if (glm::dot(Box.clipPlaneWorld[i], pointCornersWorld[7]) <= 0) sSize++;	else bSize++;
//		if (bSize == 0){
//			return NoHit;
//		}
//	}
//	if (sSize == 0){
//		return Inside;
//	}
//
//	sSize = 0;
//	bSize = 0;
//	for (int i = 0; i < 6; ++i){
//		bSize = 0;
//		if (glm::dot(clipPlaneWorld[i], Box.pointCornersWorld[0]) <= 0) sSize++;	else bSize++;
//		if (glm::dot(clipPlaneWorld[i], Box.pointCornersWorld[1]) <= 0) sSize++;	else bSize++;
//		if (glm::dot(clipPlaneWorld[i], Box.pointCornersWorld[2]) <= 0) sSize++;	else bSize++;
//		if (glm::dot(clipPlaneWorld[i], Box.pointCornersWorld[3]) <= 0) sSize++;	else bSize++;
//		if (glm::dot(clipPlaneWorld[i], Box.pointCornersWorld[4]) <= 0) sSize++;	else bSize++;
//		if (glm::dot(clipPlaneWorld[i], Box.pointCornersWorld[5]) <= 0) sSize++;	else bSize++;
//		if (glm::dot(clipPlaneWorld[i], Box.pointCornersWorld[6]) <= 0) sSize++;	else bSize++;
//		if (glm::dot(clipPlaneWorld[i], Box.pointCornersWorld[7]) <= 0) sSize++;	else bSize++;
//		if (bSize == 0){
//			return NoHit;
//		}
//	}
//	if (sSize == 0){
//		return OutSurround;
//	}
//
//	return Cross;
//}

bool RectBox::NeedClipFromObjSpace(glm::mat4 & MVP_Matrix)
{
	glm::vec4 clipPlanes[6];
	glm::mat4 MVP_Matrix_T = glm::transpose(MVP_Matrix);
	glm::vec4 row1 = MVP_Matrix_T[0];
	glm::vec4 row2 = MVP_Matrix_T[1];
	glm::vec4 row3 = MVP_Matrix_T[2];
	glm::vec4 row4 = MVP_Matrix_T[3];
	clipPlanes[0] = row1 + row4; //left
	clipPlanes[1] = row4 - row1; //right
	clipPlanes[2] = row2 + row4; //bottom
	clipPlanes[3] = row4 - row2; //top
	clipPlanes[4] = row3 + row4; //near
	clipPlanes[5] = row4 - row3; //far
	
	for (int i = 0; i < 6; ++i){
		if (glm::dot(clipPlanes[i], pointCornersModel[0]) > 0) continue;
		if (glm::dot(clipPlanes[i], pointCornersModel[1]) > 0) continue;
		if (glm::dot(clipPlanes[i], pointCornersModel[2]) > 0) continue;
		if (glm::dot(clipPlanes[i], pointCornersModel[3]) > 0) continue;
		if (glm::dot(clipPlanes[i], pointCornersModel[4]) > 0) continue;
		if (glm::dot(clipPlanes[i], pointCornersModel[5]) > 0) continue;
		if (glm::dot(clipPlanes[i], pointCornersModel[6]) > 0) continue;
		if (glm::dot(clipPlanes[i], pointCornersModel[7]) > 0) continue;
		return true;
	}
	return false;
}

//bool RectBox::NeedClipFromWorldSpace(glm::mat4 & VP_Matrix)
//{
//	glm::vec4 clipPlanes[6];
//	glm::mat4 VP_Matrix_T = glm::transpose(VP_Matrix);
//	glm::vec4 row1 = VP_Matrix_T[0];
//	glm::vec4 row2 = VP_Matrix_T[1];
//	glm::vec4 row3 = VP_Matrix_T[2];
//	glm::vec4 row4 = VP_Matrix_T[3];
//	clipPlanes[0] = row1 + row4; //left
//	clipPlanes[1] = row4 - row1; //right
//	clipPlanes[2] = row2 + row4; //bottom
//	clipPlanes[3] = row4 - row2; //top
//	clipPlanes[4] = row3 + row4; //near
//	clipPlanes[5] = row4 - row3; //far
//
//	for (int i = 0; i < 6; ++i){
//		if (glm::dot(clipPlanes[i], pointCornersWorld[0]) > 0) continue;
//		if (glm::dot(clipPlanes[i], pointCornersWorld[1]) > 0) continue;
//		if (glm::dot(clipPlanes[i], pointCornersWorld[2]) > 0) continue;
//		if (glm::dot(clipPlanes[i], pointCornersWorld[3]) > 0) continue;
//		if (glm::dot(clipPlanes[i], pointCornersWorld[4]) > 0) continue;
//		if (glm::dot(clipPlanes[i], pointCornersWorld[5]) > 0) continue;
//		if (glm::dot(clipPlanes[i], pointCornersWorld[6]) > 0) continue;
//		if (glm::dot(clipPlanes[i], pointCornersWorld[7]) > 0) continue;
//		return true;
//	}
//	return false;
//}

//bool RectBox::CenterInsideBoxWorldSpace(RectBox & Box)
//{
//	if (glm::dot(CenterWorld, Box.clipPlaneWorld[0]) <= 0) return false;
//	if (glm::dot(CenterWorld, Box.clipPlaneWorld[1]) < 0) return false;
//	if (glm::dot(CenterWorld, Box.clipPlaneWorld[2]) <= 0) return false;
//	if (glm::dot(CenterWorld, Box.clipPlaneWorld[3]) < 0) return false;
//	if (glm::dot(CenterWorld, Box.clipPlaneWorld[4]) <= 0) return false;
//	if (glm::dot(CenterWorld, Box.clipPlaneWorld[5]) < 0) return false;
//	return true;
//}

void RectBox::createCornerPointsModel()
{
	pointCornersModel[0] = glm::vec4(PointMin.x, PointMin.y, PointMin.z, 1.0);
	pointCornersModel[1] = glm::vec4(PointMax.x, PointMin.y, PointMin.z, 1.0);
	pointCornersModel[2] = glm::vec4(PointMin.x, PointMax.y, PointMin.z, 1.0);
	pointCornersModel[3] = glm::vec4(PointMax.x, PointMax.y, PointMin.z, 1.0);
	pointCornersModel[4] = glm::vec4(PointMin.x, PointMin.y, PointMax.z, 1.0);
	pointCornersModel[5] = glm::vec4(PointMax.x, PointMin.y, PointMax.z, 1.0);
	pointCornersModel[6] = glm::vec4(PointMin.x, PointMax.y, PointMax.z, 1.0);
	pointCornersModel[7] = glm::vec4(PointMax.x, PointMax.y, PointMax.z, 1.0);
}

//void RectBox::createCornerPointsWorld()
//{
//	pointCornersWorld[0] = glm::vec4(pointMin.x, pointMin.y, pointMin.z, 1.0);
//	pointCornersWorld[1] = glm::vec4(pointMax.x, pointMin.y, pointMin.z, 1.0);
//	pointCornersWorld[2] = glm::vec4(pointMin.x, pointMax.y, pointMin.z, 1.0);
//	pointCornersWorld[3] = glm::vec4(pointMax.x, pointMax.y, pointMin.z, 1.0);
//	pointCornersWorld[4] = glm::vec4(pointMin.x, pointMin.y, pointMax.z, 1.0);
//	pointCornersWorld[5] = glm::vec4(pointMax.x, pointMin.y, pointMax.z, 1.0);
//	pointCornersWorld[6] = glm::vec4(pointMin.x, pointMax.y, pointMax.z, 1.0);
//	pointCornersWorld[7] = glm::vec4(pointMax.x, pointMax.y, pointMax.z, 1.0);
//}

glm::vec3 RectBox::GetCenterPointLocal()
{
	return glm::vec3(CenterLocal);
}

glm::vec3 RectBox::GetBoundMin()
{
	return glm::vec3(PointMin);
}

glm::vec3 RectBox::GetBoundMax()
{
	return glm::vec3(PointMax);
}

float RectBox::GetLength()
{
	return BoxSize.x;
}

float RectBox::GetWidth()
{
	return BoxSize.y;
}

float RectBox::GetHeight()
{
	return BoxSize.z;
}