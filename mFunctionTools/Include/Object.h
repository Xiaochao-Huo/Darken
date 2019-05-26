#pragma once

#include "Transform.h"
#include "SurroundBox.h"
#include "GlobalPram.h"

class Object
{
public:
	Object();
	~Object();

	Transform ObjectTransform;
	
	Bool bNeedCheckClip;
	Bool bNeedClip;

	virtual void CheckWhetherNeedClip(std::shared_ptr<Camera> camera) = 0;
	virtual void Start() = 0;
	virtual void InternalUpdate() = 0;
	virtual void Update() = 0;
	virtual void FixUpdate() = 0;
	virtual void Draw() = 0;
private:
	
};


