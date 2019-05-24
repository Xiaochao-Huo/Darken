#include "AStarAlgorithm.h"

NavNode::NavNode()
{
}

NavNode::~NavNode()
{
	ParentNode = nullptr;
}

SmallRootNodeHeap::SmallRootNodeHeap(int n)
{
	Capacity = n;
	Heap = (NavNode**)malloc(sizeof(NavNode*)* Capacity);
}
SmallRootNodeHeap::~SmallRootNodeHeap()
{
	for (int Index = 0; Index < Size; Index++)
	{
		*Heap[Index]->Flag = NODE_DEFAULT;
		delete Heap[Index];
	}
	free(Heap);
	Heap = nullptr;
}

void SmallRootNodeHeap::bubbleUp(int i, NavNode* node)
{
	int parent = (i - 1) / 2;
	// note: (index > 0) means there is a parent
	while ((i > 0) && (Heap[parent]->Total > node->Total))
	{
		Heap[i] = Heap[parent];
		i = parent;
		parent = (i - 1) / 2;
	}
	Heap[i] = node;
}

void SmallRootNodeHeap::trickleDown(int i, NavNode* node)
{
	int child = (i * 2) + 1;
	while (child < Size)
	{
		if (((child + 1) < Size) &&
			(Heap[child]->Total > Heap[child + 1]->Total))
		{
			child++;
		}
		Heap[i] = Heap[child];
		i = child;
		child = (i * 2) + 1;
	}
	bubbleUp(i, node);
}

AStarNavSystem::AStarNavSystem(int NavMapWidth, int NavMapHeight)
{
	OpenList = std::shared_ptr<SmallRootNodeHeap>(new SmallRootNodeHeap(2048));
	NavGuideMap = std::shared_ptr<Texture>(new Texture());
	//NavGuideMap->InitNoMipRawTextureData(NavMapWidth, NavMapHeight, TextureDataTypeFormat::R8);
}

AStarNavSystem::~AStarNavSystem()
{
	
}

std::vector<glm::vec2> AStarNavSystem::FindPathFromGuideMap(glm::vec2 startPosWorld, glm::vec2 endPosWorld)
{
	if (NavGuideMap == nullptr) return std::vector<glm::vec2>(0);

	glm::vec2 startPos = glm::vec2(glm::inverse(ObjectTransform.GetModelMatrix()) * glm::vec4(startPosWorld, 0.0f, 1.0f));
	glm::vec2 endPos = glm::vec2(glm::inverse(ObjectTransform.GetModelMatrix()) * glm::vec4(endPosWorld, 0.0f, 1.0f));

	glm::vec2 StartPos = glm::floor(startPos);
	glm::vec2 EndPos = glm::floor(endPos);

	if (Math::IsNearlyEqual(StartPos, EndPos, 0.0001f)) return std::vector<glm::vec2>(0);
	if (GetNavMapPosStatePtr(EndPos)[0] >= NODE_CLOSE) return std::vector<glm::vec2>(0);

	NavNode* startNode = new NavNode();
	startNode->Position = StartPos;
	startNode->Cost = 0.0f;
	startNode->Heuristic = (float) Math::GeometryDis(StartPos, EndPos);
	startNode->Total = startNode->Cost + startNode->Heuristic;
	startNode->ParentNode = nullptr;
	startNode->Flag = GetNavMapPosStatePtr(StartPos);
	SetNavMapPosState(startNode->Flag, NODE_OPEN);

	OpenList->push(startNode);
	NodePool.insert(std::pair<int, NavNode*>((int)startNode->Position.x << 16 | (int)startNode->Position.y, startNode));
	
	NavNode* lastBestNode = startNode;
	while (!OpenList->empty())
	{
		NavNode* bestNode = OpenList->pop();
		*bestNode->Flag = NODE_CLOSE;

		if (bestNode->Position == EndPos)
		{
			lastBestNode = bestNode;
			break;
		}

		//glm::vec2 PreferDir = GetPreferDir(bestNode->Position, EndPos);
		//Find Node From Neighbor
		for (int NeighborIndex = 0; NeighborIndex < 9; NeighborIndex++)
		{
			glm::vec2 NeighborPos = bestNode->Position + DirMap[NeighborIndex];
			unsigned char* PosState = GetNavMapPosStatePtr(NeighborPos);
			if (PosState == nullptr || *PosState >= NODE_CLOSE)
			{
				continue;
			}
			
			float cost = glm::distance(bestNode->Position, NeighborPos);
			float heuristic = (float) Math::GeometryDis(NeighborPos, EndPos);
			float total = bestNode->Cost + cost + heuristic;
			
			if (*PosState != NODE_OPEN)
			{
				NavNode* newNavNode = new NavNode();
				newNavNode->Cost = bestNode->Cost + cost;
				newNavNode->Heuristic = heuristic;
				newNavNode->Position = NeighborPos;
				newNavNode->Total = total;
				newNavNode->ParentNode = bestNode;
				newNavNode->Flag = PosState;
				*newNavNode->Flag = NODE_OPEN;
				OpenList->push(newNavNode);
				NodePool.insert(std::pair<int, NavNode*>((int)NeighborPos.x << 16 | (int)NeighborPos.y, newNavNode));
				//break;
			}

			if (*PosState == NODE_OPEN)
			{
				NavNode* neighborNode = NodePool.find((int)NeighborPos.x << 16 | (int)NeighborPos.y)->second;
				if (neighborNode->ParentNode == bestNode)
				{
					continue;
				}
				if (neighborNode->Total > total)
				{
					neighborNode->Cost = bestNode->Cost + cost;
					neighborNode->Heuristic = heuristic;
					neighborNode->Total = total;
					neighborNode->ParentNode = bestNode;
					OpenList->modify(neighborNode);
					//break;
				}
			}
		}
	}

	int count = 1;
	NavNode* Node = lastBestNode;
	while (Node->ParentNode != nullptr)
	{
		count++;
		Node = Node->ParentNode;
	}

	std::vector<glm::vec2> Path(count);
	Node = lastBestNode;
	int index = count - 1;
	while (Node != nullptr)
	{
		Path[index--] = Node->Position;
		Node = Node->ParentNode;
	}

	for (std::unordered_map<int, NavNode*>::iterator it = NodePool.begin(); it != NodePool.end(); it++)
	{
		*it->second->Flag = NODE_DEFAULT;
		delete it->second;
		it->second = nullptr;
	}

	OpenList->clear();
	NodePool.clear();
	return Path;
}

glm::vec2 AStarNavSystem::GetPreferDir(glm::vec2& startPos, glm::vec2& endPos)
{
	if (abs(startPos.x - endPos.x) < 0.001 && abs(startPos.y - endPos.y) < 0.001){
		return glm::vec2(0.0f, 0.0f);
	}
	glm::vec2 PreferDir = endPos - startPos;
	if (abs(startPos.x - endPos.x) < 0.001)
	{
		if (startPos.y > endPos.y) PreferDir = glm::vec2(0.0f, -1.0f);
		else PreferDir = glm::vec2(0.0f, 1.0f);
		return PreferDir;
	}
	if (abs(startPos.y - endPos.y) < 0.001)
	{
		if (startPos.x > endPos.x) PreferDir = glm::vec2(-1.0f, 0.0f);
		else PreferDir = glm::vec2(1.0f, 0.0f);
		return PreferDir;
	}
	PreferDir.x = PreferDir.x / abs(endPos.x - startPos.x);
	PreferDir.y = PreferDir.y / abs(endPos.y - startPos.y);
	return glm::round(PreferDir);
}

unsigned char* AStarNavSystem::GetNavMapPosStatePtr(glm::vec2 pos)
{
	if ((int)pos.x < 0 || (int)pos.y<0 || (int)pos.x >= NavGuideMap->GetRawDataPtr(0)->Width || (int)pos.y >= NavGuideMap->GetRawDataPtr(0)->Height)
	{
		return nullptr;
	}
	return (NavGuideMap->GetRawDataPtr(0)->MipData + NavGuideMap->GetRawDataPtr(0)->Width * (int)pos.y + (int)pos.x);
}

void AStarNavSystem::SetNavMapPosState(unsigned char * dataPtr, unsigned char state)
{
	*dataPtr = state;
}

void AStarNavSystem::SetPositionAccess(glm::vec2& PosWorld2, unsigned char NodeState)
{
	glm::vec4 PosWorld4 = glm::vec4(PosWorld2, 0.0f, 1.0f);
	glm::vec4 PosLocal4 = glm::inverse(ObjectTransform.GetModelMatrix()) * PosWorld4;
	int X = (int) glm::floor(PosLocal4.x);
	int Y = (int)glm::floor(PosLocal4.y);

	int MapWidth = NavGuideMap->GetWidth();
	int MapHeight = NavGuideMap->GetHeight();

	if (X < 0 || Y < 0 || X >= MapWidth || Y >= MapHeight)
	{
		return;
	}
	unsigned char * AccessPtr = NavGuideMap->GetRawDataPtr(0)->MipData + Y * MapWidth + X;
	if (*AccessPtr == NODE_FORCECLOSE) return;
	*AccessPtr = NodeState;
}

void AStarNavSystem::ScluptPositionAccess(glm::vec2& PosWorld2, unsigned char NodeState)
{
	glm::vec4 PosWorld4 = glm::vec4(PosWorld2, 0.0f, 1.0f);
	glm::vec4 PosLocal4 = glm::inverse(ObjectTransform.GetModelMatrix()) * PosWorld4;
	int X = (int)glm::floor(PosLocal4.x);
	int Y = (int)glm::floor(PosLocal4.y);

	int MapWidth = NavGuideMap->GetWidth();
	int MapHeight = NavGuideMap->GetHeight();

	if (X < 0 || Y < 0 || X >= MapWidth || Y >= MapHeight)
	{
		return;
	}
	unsigned char * AccessPtr = NavGuideMap->GetRawDataPtr(0)->MipData + Y * MapWidth + X;
	*AccessPtr = NodeState;
}

unsigned char AStarNavSystem::GetPositionAccess(glm::vec2& PosWorld2)
{
	glm::vec4 PosWorld4 = glm::vec4(PosWorld2, 0.0f, 1.0f);
	glm::vec4 PosLocal4 = glm::inverse(ObjectTransform.GetModelMatrix()) * PosWorld4;
	int X = (int)glm::floor(PosLocal4.x);
	int Y = (int)glm::floor(PosLocal4.y);

	int MapWidth = NavGuideMap->GetWidth();
	int MapHeight = NavGuideMap->GetHeight();

	if (X < 0 || Y < 0 || X >= MapWidth || Y >= MapHeight)
	{
		return NODE_FORCECLOSE;
	}

	return NavGuideMap->GetRawDataPtr(0)->MipData[Y * MapWidth + X];
}

