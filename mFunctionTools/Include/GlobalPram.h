#pragma once
#include "SceneManager.h"
#include "BufferManager.h"
#include "SystemTextures.h"


extern std::shared_ptr<SceneManager> _Scene;
extern std::shared_ptr<BufferManager> _GPUBuffers;
extern std::shared_ptr<SystemTextureFactory> GlobalTextures;
extern const unsigned int _ScreenWidth;
extern const unsigned int _ScreenHeight;
//extern std::unordered_map<pvr::Keys, bool> InputKeyPressed;
//extern std::vector<pvr::Keys> InputKeyRecord;
extern unsigned int APP_DeltaTime;
extern unsigned long APP_FrameCount;
//extern pvr::IAssetProvider* App;

extern std::string AssetFolderPath;