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

Int32 Animation::GetNumFrames()
{
	return NumFrames;
}

void Animation::SetPlaySpeed(Float32 speed)
{
	Speed = speed;
}

void Animation::SetAttitudeMatrixes(const Float32 * matrixData, Int32 numFrames)
{
	AttitudeMatrixes.resize(numFrames);
	memcpy(&AttitudeMatrixes[0][0].x, matrixData, 16 * numFrames * sizeof(Float32));
	NumFrames = numFrames;
}

Mat4f Animation::GetNextFrameAttitudeMatrix()
{
	if (isPlay)
	{
		CurrentFrame += Speed;
		CurrentFrame = Math::Ceil(CurrentFrame) >= NumFrames ? 0 : CurrentFrame;
	}

	return Math::Lerp(AttitudeMatrixes[Math::Floor(CurrentFrame)], AttitudeMatrixes[Math::Ceil(CurrentFrame)], Math::Fract(CurrentFrame));
}

void Animation::Play()
{
	isPlay = true;
}

void Animation::Stop()
{
	isPlay = false;
}