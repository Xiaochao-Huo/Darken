#include "SurroundBox.h"
#include "Util.h"

SphereBox::SphereBox()
{
	PointMin = Vector4f(FLT_MAX, FLT_MAX, FLT_MAX, 1.0);
	PointMax = Vector4f(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0);
	CenterLocal = Vector4f(0.0, 0.0, 0.0, 1.0);
	Radius = 1.0;
}

SphereBox::~SphereBox()
{
}

void SphereBox::Reset()
{
	PointMin = Vector4f(FLT_MAX, FLT_MAX, FLT_MAX, 1.0);
	PointMax = Vector4f(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0);
	CenterLocal = Vector4f(0.0, 0.0, 0.0, 1.0);
	Radius = 1.0;
}

Bool SphereBox::bOutOfCamera(Mat4f ModelMatrix, Mat4f VPMatrix, Float32 scale)
{
	Mat4f MVP_Matrix_T = Math::Transpose(VPMatrix);
	Vector4f centerLocal = ModelMatrix * CenterLocal;
	ClipPlanesLocal[0] = MVP_Matrix_T[0] + MVP_Matrix_T[3]; //left
	ClipPlanesLocal[1] = MVP_Matrix_T[3] - MVP_Matrix_T[0]; //right
	ClipPlanesLocal[2] = MVP_Matrix_T[1] + MVP_Matrix_T[3]; //bottom
	ClipPlanesLocal[3] = MVP_Matrix_T[3] - MVP_Matrix_T[1]; //top
	ClipPlanesLocal[4] = MVP_Matrix_T[2] + MVP_Matrix_T[3]; //near
	ClipPlanesLocal[5] = MVP_Matrix_T[3] - MVP_Matrix_T[2]; //far
	for (Int32 i = 0; i < 6; i++)
	{
		Float32 dot = Math::Dot(centerLocal, ClipPlanesLocal[i]);
		if (dot > 0.001f)
		{	
			continue;
		} 
		else
		{
			Float32 distance = -dot / Math::Sqrt(ClipPlanesLocal[i].x * ClipPlanesLocal[i].x 
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

SphereBox& SphereBox::operator+=(const Vector3f& newVertex)
{
	PointMin.x = Math::Min(PointMin.x, newVertex.x);
	PointMin.y = Math::Min(PointMin.y, newVertex.y);
	PointMin.z = Math::Min(PointMin.z, newVertex.z);

	PointMax.x = Math::Max(PointMax.x, newVertex.x);
	PointMax.y = Math::Max(PointMax.y, newVertex.y);
	PointMax.z = Math::Max(PointMax.z, newVertex.z);

	CenterLocal = (PointMin + PointMax) / 2.0f;
	Radius = Math::Distance(CenterLocal, PointMax);
	Height = PointMax.z - PointMin.z;

	return *this;
}

Vector3f SphereBox::GetCenterPointLocal()
{
	return Vector3f(CenterLocal);
}

Float32 SphereBox::GetRadius()
{
	return Radius;
}

RectBox::RectBox()
{

}

RectBox::~RectBox()
{
}

void RectBox::UpdateBoxModel(UInt32 vertexNum, UInt8 * pointPtr, UInt8 stride)
{
	Float32 x, y, z;
	for (UInt32 i = 0; i < vertexNum; ++i)
	{
		x = *(Float32*)(pointPtr + i * stride);
		y = *(Float32*)(pointPtr + i * stride + 4);
		z = *(Float32*)(pointPtr + i * stride + 8);

		if (x < PointMin.x) PointMin.x = x;
		if (x > PointMax.x) PointMax.x = x;
		if (y < PointMin.y) PointMin.y = y;
		if (y > PointMax.y) PointMax.y = y;
		if (z < PointMin.z) PointMin.z = z;
		if (z > PointMax.z) PointMax.z = z;
	}
	CenterLocal = (PointMin + PointMax) / 2.0f;
	createCornerPointsModel();
	clipPlaneModel[0] = Vector4f(1.0, 0.0, 0.0, -PointMin.x); //left
	clipPlaneModel[1] = Vector4f(-1.0, 0.0, 0.0, PointMax.x); //right
	clipPlaneModel[2] = Vector4f(0.0, 1.0, 0.0, -PointMin.y); //bottom
	clipPlaneModel[3] = Vector4f(0.0, -1.0, 0.0, PointMax.y); //top
	clipPlaneModel[4] = Vector4f(0.0, 0.0, 1.0, -PointMin.z); //near
	clipPlaneModel[5] = Vector4f(0.0, 0.0, -1.0, PointMax.z); //far
}

RectBox& RectBox::operator += (const Vector3f& newVertex)
{
	PointMin.x = Math::Min(PointMin.x, newVertex.x);
	PointMin.y = Math::Min(PointMin.y, newVertex.y);
	PointMin.z = Math::Min(PointMin.z, newVertex.z);

	PointMax.x = Math::Max(PointMax.x, newVertex.x);
	PointMax.y = Math::Max(PointMax.y, newVertex.y);
	PointMax.z = Math::Max(PointMax.z, newVertex.z);

	CenterLocal = (PointMin + PointMax) / 2.0f;
	BoxSize = Vector3f(PointMax - PointMin);

	return *this;
}

//void RectBox::UpdateBoxWorld(Mat4f & ModelMatrix)
//{
//	for (Int32 i = 0; i < 8; ++i)
//	{
//		pointCornersWorld[i] = ModelMatrix * pointCornersModel[i];
//	}
//	for (Int32 i = 0; i < 6; ++i)
//	{
//		clipPlaneWorld[i] = ModelMatrix * clipPlaneModel[i];
//	}
//	CenterWorld = ModelMatrix * CenterModel;
//}

//void RectBox::CreateBoxFromCornerWorld(Float32 Xmin, Float32 Xmax, Float32 Ymin, Float32 Ymax, Float32 Zmin, Float32 Zmax)
//{
//	pointMin.x = Xmin;
//	pointMin.y = Ymin;
//	pointMin.z = Zmin;
//	pointMax.x = Xmax;
//	pointMax.y = Ymax;
//	pointMax.z = Zmax;
//	CenterWorld = (pointMin + pointMax) / 2.0f;
//	createCornerPointsWorld();
//	clipPlaneWorld[0] = Vector4f(1.0, 0.0, 0.0, -Xmin); //left
//	clipPlaneWorld[1] = Vector4f(-1.0, 0.0, 0.0, Xmax); //right
//	clipPlaneWorld[2] = Vector4f(0.0, 1.0, 0.0, -Ymin); //bottom
//	clipPlaneWorld[3] = Vector4f(0.0, -1.0, 0.0, Ymax); //top
//	clipPlaneWorld[4] = Vector4f(0.0, 0.0, 1.0, -Zmin); //near
//	clipPlaneWorld[5] = Vector4f(0.0, 0.0, -1.0, Zmax); //far
//}
/*!****************************************************************************
@Function		HitBox
@Output		HitState		NoHit,	Cross,	Inside,	OutSurround
@Return		Int32			HitState
@Description	check Hit State, Inside means leftBox is inside Box, 
OutSurround means Box is inside leftBox
******************************************************************************/
//Int32 RectBox::HitBox(RectBox & Box)
//{
//	Int32 sSize = 0;
//	Int32 bSize = 0;
//	for (Int32 i = 0; i < 6; ++i){
//		bSize = 0;
//		if (Math::Dot(Box.clipPlaneWorld[i], pointCornersWorld[0]) <= 0) sSize++;	else bSize++;
//		if (Math::Dot(Box.clipPlaneWorld[i], pointCornersWorld[1]) <= 0) sSize++;	else bSize++;
//		if (Math::Dot(Box.clipPlaneWorld[i], pointCornersWorld[2]) <= 0) sSize++;	else bSize++;
//		if (Math::Dot(Box.clipPlaneWorld[i], pointCornersWorld[3]) <= 0) sSize++;	else bSize++;
//		if (Math::Dot(Box.clipPlaneWorld[i], pointCornersWorld[4]) <= 0) sSize++;	else bSize++;
//		if (Math::Dot(Box.clipPlaneWorld[i], pointCornersWorld[5]) <= 0) sSize++;	else bSize++;
//		if (Math::Dot(Box.clipPlaneWorld[i], pointCornersWorld[6]) <= 0) sSize++;	else bSize++;
//		if (Math::Dot(Box.clipPlaneWorld[i], pointCornersWorld[7]) <= 0) sSize++;	else bSize++;
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
//	for (Int32 i = 0; i < 6; ++i){
//		bSize = 0;
//		if (Math::Dot(clipPlaneWorld[i], Box.pointCornersWorld[0]) <= 0) sSize++;	else bSize++;
//		if (Math::Dot(clipPlaneWorld[i], Box.pointCornersWorld[1]) <= 0) sSize++;	else bSize++;
//		if (Math::Dot(clipPlaneWorld[i], Box.pointCornersWorld[2]) <= 0) sSize++;	else bSize++;
//		if (Math::Dot(clipPlaneWorld[i], Box.pointCornersWorld[3]) <= 0) sSize++;	else bSize++;
//		if (Math::Dot(clipPlaneWorld[i], Box.pointCornersWorld[4]) <= 0) sSize++;	else bSize++;
//		if (Math::Dot(clipPlaneWorld[i], Box.pointCornersWorld[5]) <= 0) sSize++;	else bSize++;
//		if (Math::Dot(clipPlaneWorld[i], Box.pointCornersWorld[6]) <= 0) sSize++;	else bSize++;
//		if (Math::Dot(clipPlaneWorld[i], Box.pointCornersWorld[7]) <= 0) sSize++;	else bSize++;
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

Bool RectBox::NeedClipFromObjSpace(Mat4f & MVP_Matrix)
{
	Vector4f clipPlanes[6];
	Mat4f MVP_Matrix_T = Math::Transpose(MVP_Matrix);
	Vector4f row1 = MVP_Matrix_T[0];
	Vector4f row2 = MVP_Matrix_T[1];
	Vector4f row3 = MVP_Matrix_T[2];
	Vector4f row4 = MVP_Matrix_T[3];
	clipPlanes[0] = row1 + row4; //left
	clipPlanes[1] = row4 - row1; //right
	clipPlanes[2] = row2 + row4; //bottom
	clipPlanes[3] = row4 - row2; //top
	clipPlanes[4] = row3 + row4; //near
	clipPlanes[5] = row4 - row3; //far
	
	for (Int32 i = 0; i < 6; ++i){
		if (Math::Dot(clipPlanes[i], pointCornersModel[0]) > 0) continue;
		if (Math::Dot(clipPlanes[i], pointCornersModel[1]) > 0) continue;
		if (Math::Dot(clipPlanes[i], pointCornersModel[2]) > 0) continue;
		if (Math::Dot(clipPlanes[i], pointCornersModel[3]) > 0) continue;
		if (Math::Dot(clipPlanes[i], pointCornersModel[4]) > 0) continue;
		if (Math::Dot(clipPlanes[i], pointCornersModel[5]) > 0) continue;
		if (Math::Dot(clipPlanes[i], pointCornersModel[6]) > 0) continue;
		if (Math::Dot(clipPlanes[i], pointCornersModel[7]) > 0) continue;
		return true;
	}
	return false;
}

//Bool RectBox::NeedClipFromWorldSpace(Mat4f & VP_Matrix)
//{
//	Vector4f clipPlanes[6];
//	Mat4f VP_Matrix_T = Math::Transpose(VP_Matrix);
//	Vector4f row1 = VP_Matrix_T[0];
//	Vector4f row2 = VP_Matrix_T[1];
//	Vector4f row3 = VP_Matrix_T[2];
//	Vector4f row4 = VP_Matrix_T[3];
//	clipPlanes[0] = row1 + row4; //left
//	clipPlanes[1] = row4 - row1; //right
//	clipPlanes[2] = row2 + row4; //bottom
//	clipPlanes[3] = row4 - row2; //top
//	clipPlanes[4] = row3 + row4; //near
//	clipPlanes[5] = row4 - row3; //far
//
//	for (Int32 i = 0; i < 6; ++i){
//		if (Math::Dot(clipPlanes[i], pointCornersWorld[0]) > 0) continue;
//		if (Math::Dot(clipPlanes[i], pointCornersWorld[1]) > 0) continue;
//		if (Math::Dot(clipPlanes[i], pointCornersWorld[2]) > 0) continue;
//		if (Math::Dot(clipPlanes[i], pointCornersWorld[3]) > 0) continue;
//		if (Math::Dot(clipPlanes[i], pointCornersWorld[4]) > 0) continue;
//		if (Math::Dot(clipPlanes[i], pointCornersWorld[5]) > 0) continue;
//		if (Math::Dot(clipPlanes[i], pointCornersWorld[6]) > 0) continue;
//		if (Math::Dot(clipPlanes[i], pointCornersWorld[7]) > 0) continue;
//		return true;
//	}
//	return false;
//}

//Bool RectBox::CenterInsideBoxWorldSpace(RectBox & Box)
//{
//	if (Math::Dot(CenterWorld, Box.clipPlaneWorld[0]) <= 0) return false;
//	if (Math::Dot(CenterWorld, Box.clipPlaneWorld[1]) < 0) return false;
//	if (Math::Dot(CenterWorld, Box.clipPlaneWorld[2]) <= 0) return false;
//	if (Math::Dot(CenterWorld, Box.clipPlaneWorld[3]) < 0) return false;
//	if (Math::Dot(CenterWorld, Box.clipPlaneWorld[4]) <= 0) return false;
//	if (Math::Dot(CenterWorld, Box.clipPlaneWorld[5]) < 0) return false;
//	return true;
//}

void RectBox::createCornerPointsModel()
{
	pointCornersModel[0] = Vector4f(PointMin.x, PointMin.y, PointMin.z, 1.0);
	pointCornersModel[1] = Vector4f(PointMax.x, PointMin.y, PointMin.z, 1.0);
	pointCornersModel[2] = Vector4f(PointMin.x, PointMax.y, PointMin.z, 1.0);
	pointCornersModel[3] = Vector4f(PointMax.x, PointMax.y, PointMin.z, 1.0);
	pointCornersModel[4] = Vector4f(PointMin.x, PointMin.y, PointMax.z, 1.0);
	pointCornersModel[5] = Vector4f(PointMax.x, PointMin.y, PointMax.z, 1.0);
	pointCornersModel[6] = Vector4f(PointMin.x, PointMax.y, PointMax.z, 1.0);
	pointCornersModel[7] = Vector4f(PointMax.x, PointMax.y, PointMax.z, 1.0);
}

//void RectBox::createCornerPointsWorld()
//{
//	pointCornersWorld[0] = Vector4f(pointMin.x, pointMin.y, pointMin.z, 1.0);
//	pointCornersWorld[1] = Vector4f(pointMax.x, pointMin.y, pointMin.z, 1.0);
//	pointCornersWorld[2] = Vector4f(pointMin.x, pointMax.y, pointMin.z, 1.0);
//	pointCornersWorld[3] = Vector4f(pointMax.x, pointMax.y, pointMin.z, 1.0);
//	pointCornersWorld[4] = Vector4f(pointMin.x, pointMin.y, pointMax.z, 1.0);
//	pointCornersWorld[5] = Vector4f(pointMax.x, pointMin.y, pointMax.z, 1.0);
//	pointCornersWorld[6] = Vector4f(pointMin.x, pointMax.y, pointMax.z, 1.0);
//	pointCornersWorld[7] = Vector4f(pointMax.x, pointMax.y, pointMax.z, 1.0);
//}

Vector3f RectBox::GetCenterPointLocal()
{
	return Vector3f(CenterLocal);
}

Vector3f RectBox::GetBoundMin()
{
	return Vector3f(PointMin);
}

Vector3f RectBox::GetBoundMax()
{
	return Vector3f(PointMax);
}

Float32 RectBox::GetLength()
{
	return BoxSize.x;
}

Float32 RectBox::GetWidth()
{
	return BoxSize.y;
}

Float32 RectBox::GetHeight()
{
	return BoxSize.z;
}