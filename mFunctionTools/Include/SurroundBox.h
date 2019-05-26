#pragma once

#include <float.h>
#include "TypeDefine.h"

enum HitState
{
	NoHit,
	Cross,
	Inside,
	OutSurround
};

//2018.11.8
class SphereBox
{
public:
	SphereBox();
	~SphereBox();
 
	void Reset();
	Bool bOutOfCamera(Mat4f ModelMatrix, Mat4f VPMatrix, Float32 scale);

	SphereBox& operator+= (const Vector3f& newVertex);

	Vector3f GetCenterPointLocal();
	Float32 GetRadius();

private:
	Vector4f ClipPlanesLocal[6];

	Vector4f PointMin;
	Vector4f PointMax;
	Vector4f CenterLocal;
	Float32 Radius;
	Float32 Height;
};




//2018.9
//            2                                3
//              ------------------------------
//             /|                           /|
//            / |                          / |				ÓÒÊÖ×ø±êÏµ
//           /  |                         /  |
//          /   |                        /   |
//         /    |                       /    |				+Y
//        /     |                      /     |				|
//       /      |                     /      |				|
//      /       |                    /       |				|
//     /        |                   /        |				|
//  6 /         |                7 /         |				|
//   /----------------------------/          |				/-------------- +X
//   |          |                 |          |			   /	
//   |          |                 |          |      	  /
//   |        0 |                 |          |			 /
//   |          |-----------------|----------|			/+Z
//   |         /                  |         /  1    
//   |        /                   |        /               
//   |       /                    |       /         
//   |      /                     |      /              
//   |     /                      |     /               
//   |    /                       |    /               
//   |   /                        |   /           
//   |  /                         |  /            
//   | /                          | /             
//   |/                           |/              
//   ------------------------------               
//  4                              5
//

class RectBox
{
public:
	RectBox();
	~RectBox();

	void UpdateBoxModel(UInt32 vertexNum, UInt8 * pointPtr, UInt8 stride);
	//void UpdateBoxWorld(Mat4f & ModelMatrix);
	//void CreateBoxFromCornerWorld(Float32 Xmin, Float32 Xmax, Float32 Ymin, Float32 Ymax, Float32 Zmin, Float32 Zmax);
	Bool NeedClipFromObjSpace(Mat4f & MVP_Matrix);
	//Bool NeedClipFromWorldSpace(Mat4f & VP_Matrix);
	//Int32 HitBox(RectBox & Box);
	//Bool CenterInsideBoxWorldSpace(RectBox & Box);

	RectBox& operator+= (const Vector3f& newVertex);

	Vector3f GetCenterPointLocal();
	Vector3f GetBoundMin();
	Vector3f GetBoundMax();
	Float32 GetLength();
	Float32 GetWidth();
	Float32 GetHeight();

private:
	Vector4f CenterLocal;
	Vector3f BoxSize;

	//Vector4f CenterWorld;
	Vector4f pointCornersModel[8];
	Vector4f clipPlaneModel[6];

	Vector4f PointMin = Vector4f(FLT_MAX, FLT_MAX, FLT_MAX, 1.0);
	Vector4f PointMax = Vector4f(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0);

	//Vector4f pointCornersWorld[8];
	//Vector4f clipPlaneWorld[6];
	void createCornerPointsModel();
	//void createCornerPointsWorld();
};

