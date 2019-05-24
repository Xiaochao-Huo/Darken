#pragma once

#include "Material.h"
#include <map>
#include <unordered_map>

class MaterialInstance
{
public:
	MaterialInstance(std::shared_ptr<Material> parentMaterial);
	~MaterialInstance();

	void SetParent(std::shared_ptr<Material> parentMaterial);
	void ChangedParentDynamic(std::shared_ptr<Material> parentMaterial);
	std::shared_ptr<Material> GetParent();
	static int GetID(const std::string &UniformName);

	template<class T>
	void SetUniform(int UniformID, const T &data)
	{
		if (BasicUniformID_PtrMap.find(UniformID) == BasicUniformID_PtrMap.end()) return;
		memcpy(BasicUniformID_PtrMap[UniformID], &data, sizeof(T));
	}

	template<class T>
	void SetUniform(const std::string &UniformName, const T &data)
	{
		int ID = hs(UniformName);
		if (BasicUniformID_PtrMap.find(ID) == BasicUniformID_PtrMap.end()) return;
		memcpy(BasicUniformID_PtrMap[ID], &data, sizeof(T));
	}

	template<class T>
	void SetUniformArray(int UniformID, T * data, int count)
	{
		if (BasicUniformID_PtrMap.find(UniformID) == BasicUniformID_PtrMap.end()) return;
		memcpy(BasicUniformID_PtrMap[UniformID], data, sizeof(T) * count);
	}

	template<class T>
	void SetUniformArray(const std::string &UniformName, T * data, int count)
	{
		int ID = hs(UniformName);
		if (BasicUniformID_PtrMap.find(ID) == BasicUniformID_PtrMap.end()) return;
		memcpy(BasicUniformID_PtrMap[ID], data, sizeof(T) * count);
	}

	template<class T>
	void SetBlockUniform(int BlockID, int UniformID, const T &data)
	{
		if (BlockID_UniformID_DataPtrMap.find(BlockID) == BlockID_UniformID_DataPtrMap.end()) return;
		if (BlockID_UniformID_DataPtrMap[BlockID].find(UniformID) == BlockID_UniformID_DataPtrMap[BlockID].end()) return;
		memcpy(BlockID_UniformID_DataPtrMap[BlockID][UniformID], &data, sizeof(T));
		MarkDirty(BlockID);
	}

	template<class T>
	void SetBlockUniform(const std::string &BlockName, const std::string &UniformName, const T &data)
	{
		int BlockID = hs(BlockName);
		if (BlockID_UniformID_DataPtrMap.find(BlockID) == BlockID_UniformID_DataPtrMap.end()) return;
		int UniformID = hs(UniformName);
		if (BlockID_UniformID_DataPtrMap[BlockID].find(UniformID) == BlockID_UniformID_DataPtrMap[BlockID].end()) return;
		memcpy(BlockID_UniformID_DataPtrMap[BlockID][UniformID], &data, sizeof(T));
		MarkDirty(BlockID);
	}

	template<class T>
	void SetBlockUniformArray(int BlockID, int UniformID, T * data, int count)
	{
		if (BlockID_UniformID_DataPtrMap.find(BlockID) == BlockID_UniformID_DataPtrMap.end()) return;
		if (BlockID_UniformID_DataPtrMap[BlockID].find(UniformID) == BlockID_UniformID_DataPtrMap[BlockID].end()) return;
		memcpy(BlockID_UniformID_DataPtrMap[BlockID][UniformID], data, sizeof(T) * count);
		MarkDirty(BlockID);
	}

	template<class T>
	void SetBlockUniformArray(const std::string &BlockName, const std::string &UniformName, T * data, int count)
	{
		int BlockID = hs(BlockName);
		if (BlockID_UniformID_DataPtrMap.find(BlockID) == BlockID_UniformID_DataPtrMap.end()) return;
		int UniformID = hs(UniformName);
		if (BlockID_UniformID_DataPtrMap[BlockID].find(UniformID) == BlockID_UniformID_DataPtrMap[BlockID].end()) return;
		memcpy(BlockID_UniformID_DataPtrMap[BlockID][UniformID], data, sizeof(T) * count);
		MarkDirty(BlockID);
	}

	void SetTextureID(int UniformID, unsigned int data)
	{
		if (TextureUniformID_PtrMap.find(UniformID) == TextureUniformID_PtrMap.end()) return;
		memcpy(TextureUniformID_PtrMap[UniformID], &data, sizeof(unsigned int));
	}

	void SetTextureID(const std::string &UniformName, unsigned int data)
	{
		int ID = hs(UniformName);
		if (TextureUniformID_PtrMap.find(ID) == TextureUniformID_PtrMap.end()) return;
		memcpy(TextureUniformID_PtrMap[ID], &data, sizeof(unsigned int));
	}

private:
	std::map<int, void*> BasicUniformID_PtrMap;
	std::map<int, unsigned int*> TextureUniformID_PtrMap;
	std::map<int, std::map<int, void*>> BlockID_UniformID_DataPtrMap;
	std::shared_ptr<Material> ParentMaterial;
	std::hash<std::string> hs;
	void MarkDirty(int BlockID);
};

