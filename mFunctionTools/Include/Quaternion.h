#ifndef __MQUATERNION_H_
#define __MQUATERNION_H_

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#define ORIGIN_FORWARD glm::vec3(1.0, 0.0, 0.0)
#define ORIGIN_LEFTWARD glm::vec3(0.0, -1.0, 0.0)
#define ORIGIN_UPWARD glm::vec3(0.0, 0.0, 1.0)

struct mQuaternion
{
public:
	static glm::mat4 AxisAngle(glm::float32 thet, const glm::vec3 &vector);
};



#endif 