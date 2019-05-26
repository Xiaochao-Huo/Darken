#pragma once

#include "TypeDefine.h"
#include "Util.h"
#include <vector>


class Animation
{
public:
	Animation();
	~Animation();

	Int32 GetNumFrames();
	void SetPlaySpeed(Float32 speed);
	void Stop();
	void Play();
	void SetAttitudeMatrixes(const Float32 * matrixData, Int32 numFrames);
	Mat4f GetNextFrameAttitudeMatrix();
	
private:
	std::vector<Mat4f> AttitudeMatrixes;
	Float32 CurrentFrame;
	Int32 NumFrames;
	Float32 Speed;
	Bool isPlay;
};
