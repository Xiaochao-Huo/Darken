#pragma once

#include "GlobalPram.h"
#include <vector>


class Animation
{
public:
	Animation();
	~Animation();

	int GetNumFrames();
	void SetPlaySpeed(float speed);
	void Stop();
	void Play();
	void SetAttitudeMatrixes(const float * matrixData, int numFrames);
	glm::mat4 GetNextFrameAttitudeMatrix();
	
private:
	std::vector<glm::mat4> AttitudeMatrixes;
	float CurrentFrame;
	int NumFrames;
	float Speed;
	bool isPlay;
};
