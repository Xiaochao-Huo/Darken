#include "Quaternion.h"
#include "Util.h"

//mQuaternion::mQuaternion()
//{
//	RotationMatrix = PVRTMat4::Identity();
//}
//
//mQuaternion::~mQuaternion()
//{
//}

Mat4f mQuaternion::AxisAngle(Float32 thet, const Vector3f &vector)
{
	Float32 x = Math::Normalize(vector).x * sin(thet / 2.0f);
	Float32 y = Math::Normalize(vector).y * sin(thet / 2.0f);
	Float32 z = Math::Normalize(vector).z * sin(thet / 2.0f);
	Float32 w = Math::Cos(thet / 2.0f);

	return Mat4f(1.0 - 2.0 * y * y - 2.0 * z * z, 2.0 * (x * y - z * w), 2.0 * (x * z + y * w), 0.0,
		2.0 * (x * y + z * w), 1.0 - 2.0 * x * x - 2.0 * z * z, 2.0 * (y * z - x * w), 0.0,
		2.0 * (x * z - y * w), 2.0 * (y * z + x * w), 1 - 2 * x * x - 2 * y * y, 0.0,
		0.0, 0.0, 0.0, 1.0);
}

