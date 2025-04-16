#pragma once
#include "Monsters.h"
#include "FBXLoader.h"

using namespace std;

void LoadAllMonsters(
    const ComPtr<ID3D12Device>& device,
    const unordered_map<string, shared_ptr<Texture>>& textures,
    const unordered_map<string, shared_ptr<Shader>> shaders,
    vector<shared_ptr<Monsters>>& outMonsters);