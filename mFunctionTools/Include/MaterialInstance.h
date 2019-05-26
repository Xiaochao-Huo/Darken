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
	static Int32 GetID(const std::string &UniformName);

	template<class T>
	void SetUniform(Int32 UniformID, const T &data)
	{
		if (BasicUniformID_PtrMap.find(UniformID) == BasicUniformID_PtrMap.end()) return;
		memcpy(BasicUniformID_PtrMap[UniformID], &data, sizeof(T));
	}

	template<class T>
	void SetUniform(const std::string &UniformName, const T &data)
	{
		Int32 ID = (Int32) hs(UniformName);
		if (BasicUniformID_PtrMap.find(ID) == BasicUniformID_PtrMap.end()) return;
		memcpy(BasicUniformID_PtrMap[ID], &data, sizeof(T));
	}

	template<class T>
	void SetUniformArray(Int32 UniformID, T * data, Int32 count)
	{
		if (BasicUniformID_PtrMap.find(UniformID) == BasicUniformID_PtrMap.end()) return;
		memcpy(BasicUniformID_PtrMap[UniformID], data, sizeof(T) * count);
	}

	template<class T>
	void SetUniformArray(const std::string &UniformName, T * data, Int32 count)
	{
		Int32 ID = (Int32)hs(UniformName);
		if (BasicUniformID_PtrMap.find(ID) == BasicUniformID_PtrMap.end()) return;
		memcpy(BasicUniformID_PtrMap[ID], data, sizeof(T) * count);
	}

	template<class T>
	void SetBlockUniform(Int32 BlockID, Int32 UniformID, const T &data)
	{
		if (BlockID_UniformID_DataPtrMap.find(BlockID) == BlockID_UniformID_DataPtrMap.end()) return;
		if (BlockID_UniformID_DataPtrMap[BlockID].find(UniformID) == BlockID_UniformID_DataPtrMap[BlockID].end()) return;
		memcpy(BlockID_UniformID_DataPtrMap[BlockID][UniformID], &data, sizeof(T));
		MarkDirty(BlockID);
	}

	template<class T>
	void SetBlockUniform(const std::string &BlockName, const std::string &UniformName, const T &data)
	{
		Int32 BlockID = (Int32)hs(BlockName);
		if (BlockID_UniformID_DataPtrMap.find(BlockID) == BlockID_UniformID_DataPtrMap.end()) return;
		Int32 UniformID = (Int32)hs(UniformName);
		if (BlockID_UniformID_DataPtrMap[BlockID].find(UniformID) == BlockID_UniformID_DataPtrMap[BlockID].end()) return;
		memcpy(BlockID_UniformID_DataPtrMap[BlockID][UniformID], &data, sizeof(T));
		MarkDirty(BlockID);
	}

	template<class T>
	void SetBlockUniformArray(Int32 BlockID, Int32 UniformID, T * data, Int32 count)
	{
		if (BlockID_UniformID_DataPtrMap.find(BlockID) == BlockID_UniformID_DataPtrMap.end()) return;
		if (BlockID_UniformID_DataPtrMap[BlockID].find(UniformID) == BlockID_UniformID_DataPtrMap[BlockID].end()) return;
		memcpy(BlockID_UniformID_DataPtrMap[BlockID][UniformID], data, sizeof(T) * count);
		MarkDirty(BlockID);
	}

	template<class T>
	void SetBlockUniformArray(const std::string &BlockName, const std::string &UniformName, T * data, Int32 count)
	{
		Int32 BlockID = hs(BlockName);
		if (BlockID_UniformID_DataPtrMap.find(BlockID) == BlockID_UniformID_DataPtrMap.end()) return;
		Int32 UniformID = hs(UniformName);
		if (BlockID_UniformID_DataPtrMap[BlockID].find(UniformID) == BlockID_UniformID_DataPtrMap[BlockID].end()) return;
		memcpy(BlockID_UniformID_DataPtrMap[BlockID][UniformID], data, sizeof(T) * count);
		MarkDirty(BlockID);
	}

	void SetTextureID(Int32 UniformID, UInt32 data)
	{
		if (TextureUniformID_PtrMap.find(UniformID) == TextureUniformID_PtrMap.end()) return;
		memcpy(TextureUniformID_PtrMap[UniformID], &data, sizeof(UInt32));
	}

	void SetTextureID(const std::string &UniformName, UInt32 data)
	{
		Int32 ID = (Int32)hs(UniformName);
		if (TextureUniformID_PtrMap.find(ID) == TextureUniformID_PtrMap.end()) return;
		memcpy(TextureUniformID_PtrMap[ID], &data, sizeof(UInt32));
	}

private:
	std::map<Int32, void*> BasicUniformID_PtrMap;
	std::map<Int32, UInt32*> TextureUniformID_PtrMap;
	std::map<Int32, std::map<Int32, void*>> BlockID_UniformID_DataPtrMap;
	std::shared_ptr<Material> ParentMaterial;
	std::hash<std::string> hs;
	void MarkDirty(Int32 BlockID);
};

