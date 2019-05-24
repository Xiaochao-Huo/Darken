#pragma once

#include <float.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

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
	bool bOutOfCamera(glm::mat4 ModelMatrix, glm::mat4 VPMatrix, float scale);

	SphereBox& operator+= (const glm::vec3& newVertex);

	glm::vec3 GetCenterPointLocal();
	float GetRadius();

private:
	glm::vec4 ClipPlanesLocal[6];

	glm::vec4 PointMin;
	glm::vec4 PointMax;
	glm::vec4 CenterLocal;
	float Radius;
	float Height;
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

	void UpdateBoxModel(glm::uint32 vertexNum, glm::uint8 * pointPtr, glm::uint8 stride);
	//void UpdateBoxWorld(glm::mat4 & ModelMatrix);
	//void CreateBoxFromCornerWorld(float Xmin, float Xmax, float Ymin, float Ymax, float Zmin, float Zmax);
	bool NeedClipFromObjSpace(glm::mat4 & MVP_Matrix);
	//bool NeedClipFromWorldSpace(glm::mat4 & VP_Matrix);
	//int HitBox(RectBox & Box);
	//bool CenterInsideBoxWorldSpace(RectBox & Box);

	RectBox& operator+= (const glm::vec3& newVertex);

	glm::vec3 GetCenterPointLocal();
	glm::vec3 GetBoundMin();
	glm::vec3 GetBoundMax();
	float GetLength();
	float GetWidth();
	float GetHeight();

private:
	glm::vec4 CenterLocal;
	glm::vec3 BoxSize;

	//glm::vec4 CenterWorld;
	glm::vec4 pointCornersModel[8];
	glm::vec4 clipPlaneModel[6];

	glm::vec4 PointMin = glm::vec4(FLT_MAX, FLT_MAX, FLT_MAX, 1.0);
	glm::vec4 PointMax = glm::vec4(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0);

	//glm::vec4 pointCornersWorld[8];
	//glm::vec4 clipPlaneWorld[6];
	void createCornerPointsModel();
	//void createCornerPointsWorld();
};

