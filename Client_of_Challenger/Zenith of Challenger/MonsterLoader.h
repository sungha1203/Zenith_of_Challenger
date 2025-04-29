#pragma once
#include "Monsters.h"
#include "FBXLoader.h"

using namespace std;

void LoadAllMonsters(
    const ComPtr<ID3D12Device>& device,
    const unordered_map<string, shared_ptr<Texture>>& textures,
    const unordered_map<string, shared_ptr<Shader>>& shaders,
    const unordered_map<string, shared_ptr<MeshBase>>& meshLibrary,       
    const unordered_map<string, AnimationClip>& animClipLibrary,          
    const unordered_map<string, XMMATRIX>& boneOffsetLibrary,             
    const unordered_map<string, int>& boneMap,                            
    unordered_map<string, vector<shared_ptr<Monsters>>>& outMonsterGroups,
    const shared_ptr<Camera>& camera);