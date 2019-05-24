#pragma once

#include "mTexture.h"
#include "mUtil.h"
#include "mObject.h"
#include <set>
#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"

class NavNode
{
public:
	NavNode();
	~NavNode();

	glm::vec2 Position;
	float Cost;
	float Heuristic;
	float Total;
	unsigned char* Flag;
	NavNode* ParentNode;

private:
	
};

class SmallRootNodeHeap
{
public:
	SmallRootNodeHeap(int n);
	~SmallRootNodeHeap();

	inline void clear()
	{
		Size = 0;
	}

	inline NavNode* top()
	{
		return Heap[0];
	}

	inline NavNode* pop()
	{
		NavNode* result = Heap[0];
		Size--;
		trickleDown(0, Heap[Size]);
		return result;
	}

	inline void push(NavNode* node)
	{
		Size++;
		bubbleUp(Size - 1, node);
	}

	inline void modify(NavNode* node)
	{
		for (int i = 0; i < Size; ++i)
		{
			if (Heap[i] == node)
			{
				bubbleUp(i, node);
				return;
			}
		}
	}

	inline bool empty() const { return Size == 0; }

	inline int getMemUsed() const
	{
		return sizeof(*this) +
			sizeof(NavNode*)*(Capacity + 1);
	}

	inline int getCapacity() const { return Capacity; }

	NavNode ** Heap;
private:
	void bubbleUp(int i, NavNode* node);
	void trickleDown(int i, NavNode* node);
	int Capacity;
	int Size;
	
};

class NavNodeCmp{
public:
	bool operator()(const std::shared_ptr<NavNode> A, const std::shared_ptr<NavNode> B)
	{
		return A->Total < B->Total ? true : false;
	}
};

static const glm::vec2 DirMap[9] = { glm::vec2(-1.0f, -1.0f),
									 glm::vec2(0.0f, -1.0f),
									 glm::vec2(1.0f, -1.0f),
									 glm::vec2(-1.0f, 0.0f),
									 glm::vec2(0.0f, 0.0f),
									 glm::vec2(1.0f, 0.0f),
									 glm::vec2(-1.0f, 1.0f),
									 glm::vec2(0.0f, 1.0f),
									 glm::vec2(1.0f, 1.0f) };

static const glm::vec2 NeighborDirSearchMap[8][3][3] = { //Oh my god Useless
	{
		{ DirMap[0], DirMap[3], DirMap[6] },
		{ DirMap[1], DirMap[4], DirMap[7] },
		{ DirMap[2], DirMap[5], DirMap[8] }
	},

	{ 
		{ DirMap[1], DirMap[0], DirMap[3] },
		{ DirMap[2], DirMap[4], DirMap[6] },
		{ DirMap[5], DirMap[8], DirMap[7] }
	}, 

	{
		{ DirMap[3], DirMap[6], DirMap[7] },
		{ DirMap[0], DirMap[4], DirMap[8] },
		{ DirMap[1], DirMap[2], DirMap[5] }
	},

	{
		{ DirMap[2], DirMap[1], DirMap[0] },
		{ DirMap[5], DirMap[4], DirMap[3] },
		{ DirMap[8], DirMap[7], DirMap[6] }
	},

	{
		{ DirMap[6], DirMap[7], DirMap[8] },
		{ DirMap[3], DirMap[4], DirMap[5] },
		{ DirMap[0], DirMap[1], DirMap[2] }
	},

	{
		{ DirMap[5], DirMap[2], DirMap[1] },
		{ DirMap[8], DirMap[4], DirMap[0] },
		{ DirMap[7], DirMap[6], DirMap[3] }
	},

	{
		{ DirMap[7], DirMap[8], DirMap[5] },
		{ DirMap[6], DirMap[4], DirMap[2] },
		{ DirMap[3], DirMap[0], DirMap[1] }
	},

	{
		{ DirMap[8], DirMap[5], DirMap[2] },
		{ DirMap[7], DirMap[4], DirMap[1] },
		{ DirMap[6], DirMap[3], DirMap[0] }
	}
};

static const unsigned char NODE_DEFAULT = 0;
static const unsigned char NODE_OPEN = 128;
static const unsigned char NODE_CLOSE = 254;
static const unsigned char NODE_FORCECLOSE = 255;

class AStarNavSystem : public Object
{
public:
	AStarNavSystem(int NavMapWidth, int NavMapHeight);
	~AStarNavSystem();

	std::shared_ptr<Texture> NavGuideMap;
	std::vector<glm::vec2> FindPathFromGuideMap(glm::vec2 startPosWorld, glm::vec2 endPosWorld);
	void SetPositionAccess(glm::vec2& PosWorld2, unsigned char NodeState);
	void ScluptPositionAccess(glm::vec2& PosWorld2, unsigned char NodeState);
	unsigned char GetPositionAccess(glm::vec2& PosWorld2);

	virtual void CheckWhetherNeedClip() { bNeedClip = true; };
	virtual void Start() {};
	virtual void InternalUpdate(){};
	virtual void Update() {};
	virtual void FixUpdate() {};
	virtual void Draw() {};

private:
	std::shared_ptr<SmallRootNodeHeap> OpenList;
	std::unordered_map<int, NavNode*> NodePool;

	glm::vec2 GetPreferDir(glm::vec2& startPos, glm::vec2& endPos);
	unsigned char* GetNavMapPosStatePtr(glm::vec2 pos);
	void SetNavMapPosState(unsigned char * dataPtr, unsigned char state);
};

