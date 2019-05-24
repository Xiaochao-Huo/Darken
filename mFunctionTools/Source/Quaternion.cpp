#include "Quaternion.h"

//mQuaternion::mQuaternion()
//{
//	RotationMatrix = PVRTMat4::Identity();
//}
//
//mQuaternion::~mQuaternion()
//{
//}

glm::mat4 mQuaternion::AxisAngle(glm::float32 thet, const glm::vec3 &vector)
{
	float x = glm::normalize(vector).x * sin(thet / 2.0f);
	float y = glm::normalize(vector).y * sin(thet / 2.0f);
	float z = glm::normalize(vector).z * sin(thet / 2.0f);
	float w = cos(thet / 2.0f);

	return glm::mat4(1.0 - 2.0 * y * y - 2.0 * z * z, 2.0 * (x * y - z * w), 2.0 * (x * z + y * w), 0.0,
		2.0 * (x * y + z * w), 1.0 - 2.0 * x * x - 2.0 * z * z, 2.0 * (y * z - x * w), 0.0,
		2.0 * (x * z - y * w), 2.0 * (y * z + x * w), 1 - 2 * x * x - 2 * y * y, 0.0,
		0.0, 0.0, 0.0, 1.0);
}

