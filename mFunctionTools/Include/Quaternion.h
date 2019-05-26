#ifndef __MQUATERNION_H_
#define __MQUATERNION_H_

#include "TypeDefine.h"

#define ORIGIN_FORWARD Vector3f(1.0, 0.0, 0.0)
#define ORIGIN_LEFTWARD Vector3f(0.0, -1.0, 0.0)
#define ORIGIN_UPWARD Vector3f(0.0, 0.0, 1.0)

struct mQuaternion
{
public:
	static Mat4f AxisAngle(Float32 thet, const Vector3f &vector);
};



#endif 