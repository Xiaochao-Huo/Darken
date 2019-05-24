#include "Animation.h"

Animation::Animation()
{
	CurrentFrame = 0.0f;
	Speed = 1.0f;
	NumFrames = -1;
	isPlay = true;
}

Animation::~Animation()
{
}

int Animation::GetNumFrames()
{
	return NumFrames;
}

void Animation::SetPlaySpeed(float speed)
{
	Speed = speed;
}

void Animation::SetAttitudeMatrixes(const float * matrixData, int numFrames)
{
	AttitudeMatrixes.resize(numFrames);
	memcpy(&AttitudeMatrixes[0][0].x, matrixData, 16 * numFrames * sizeof(float));
	NumFrames = numFrames;
}

glm::mat4 Animation::GetNextFrameAttitudeMatrix()
{
	if (isPlay)
	{
		CurrentFrame += Speed;
		CurrentFrame = glm::ceil(CurrentFrame) >= NumFrames ? 0 : CurrentFrame;
	}

	return Math::Lerp(AttitudeMatrixes[glm::floor(CurrentFrame)], AttitudeMatrixes[glm::ceil(CurrentFrame)], glm::fract(CurrentFrame));	
}

void Animation::Play()
{
	isPlay = true;
}

void Animation::Stop()
{
	isPlay = false;
}