#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stdafx.h"
#include "vertex.h"      // ���ؽ� ����ü ����
#include "texture.h"     // �ؽ�ó ���� ó�� ����
#include "Object.h"      // DirectX 12���� ����� ������Ʈ Ŭ���� ����
#include "Mesh.h"        // DirectX 12���� ����� Mesh Ŭ���� ����
#include "GameFramework.h" // DirectX 12���� gGameFramework ����
#include "OtherPlayer.h"

class FBXLoader
{
public:
    FBXLoader() = default;
    ~FBXLoader() = default;

    bool LoadFBXModel(const std::string& filename, const XMMATRIX& rootTransform); // FBX ���� �ε� �Լ�

    // FBX���� �ε��� �޽��� ��ȯ�ϴ� �Լ� �߰�
    vector<shared_ptr<MeshBase>> GetMeshes() { return m_meshes; }

    // FBX���� �ε��� GameObject�� ��ȯ�ϴ� �Լ� �߰�
    vector<shared_ptr<GameObject>> GetGameObjects() { return m_gameObjects; }

    //�ִϸ��̼� �ڵ�
    const std::vector<AnimationClip>& GetAnimationClips() const { return m_animationClips; }

    void SetSharedTexture(std::shared_ptr<Texture> texture) {
        m_sharedFBXTexture = texture;
    }
    unordered_map<string, XMMATRIX> m_staticNodeTransforms;
    const auto& GetBoneOffsets() const { return m_boneOffsets; }
    const auto& GetBoneNameToIndex() const { return m_boneNameToIndex; }
    const auto& GetBoneHierarchy() const { return m_boneHierarchy; }
    const auto& GetStaticNodeTransforms() const { return m_staticNodeTransforms; }
    const auto& GetNodeNameToGlobalTransform() const { return m_nodeNameToLocalTransform; }
    shared_ptr<OtherPlayer> LoadOtherPlayer(
        const ComPtr<ID3D12Device>& device,
        const unordered_map<string, shared_ptr<Texture>>& textures,
        const unordered_map<string, shared_ptr<Shader>>& shaders);
    XMMATRIX m_rootTransform;
    void PrintBoneHierarchy(aiNode* node, int depth = 0);
    void DumpBoneHierarchy(const aiScene* scene);
    std::unordered_map<std::string, XMMATRIX> m_nodeNameToLocalTransform;
private:
    void ProcessNode(aiNode* node, const aiScene* scene, const XMMATRIX& parentTransform); // FBX ��� ó��
    shared_ptr<GameObject> ProcessMesh(aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatrix); // FBX �޽� ó��
    void ProcessAnimations(const aiScene* scene);
    void FBXLoader::BuildBoneHierarchy(aiNode* node, const std::string& parentName, const XMMATRIX& parentTransform);
private:
    vector<shared_ptr<MeshBase>> m_meshes;
    vector<shared_ptr<GameObject>> m_gameObjects; // FBX �𵨿� GameObject ����
    //�ִϸ��̼� �ڵ�
    std::vector<AnimationClip> m_animationClips;

    std::shared_ptr<Texture> m_sharedFBXTexture;
    unordered_map<int, XMMATRIX> m_boneOffsets;
    unordered_map<int, XMMATRIX> m_globalInverseTransform;
    unordered_map<string, int> m_boneNameToIndex;
    unordered_map<string, string> m_boneHierarchy; // child -parent
};
