#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stdafx.h"
#include "vertex.h"      // 버텍스 구조체 포함
#include "texture.h"     // 텍스처 관련 처리 포함
#include "Object.h"      // DirectX 12에서 사용할 오브젝트 클래스 포함
#include "Mesh.h"        // DirectX 12에서 사용할 Mesh 클래스 포함
#include "GameFramework.h" // DirectX 12에서 gGameFramework 참조

class FBXLoader
{
public:
    FBXLoader() = default;
    ~FBXLoader() = default;

    bool LoadFBXModel(const std::string& filename, const XMMATRIX& rootTransform); // FBX 파일 로드 함수

    // FBX에서 로드한 메쉬를 반환하는 함수 추가
    vector<shared_ptr<MeshBase>> GetMeshes() { return m_meshes; }

    // FBX에서 로드한 GameObject를 반환하는 함수 추가
    vector<shared_ptr<GameObject>> GetGameObjects() { return m_gameObjects; }

    //애니메이션 코드
    const std::vector<AnimationClip>& GetAnimationClips() const { return m_animationClips; }

    void SetSharedTexture(std::shared_ptr<Texture> texture) {
        m_sharedFBXTexture = texture;
    }

    const auto& GetBoneOffsets() const { return m_boneOffsets; }
    const auto& GetBoneNameToIndex() const { return m_boneNameToIndex; }

private:
    void ProcessNode(aiNode* node, const aiScene* scene, const XMMATRIX& parentTransform); // FBX 노드 처리
    shared_ptr<GameObject> ProcessMesh(aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatrix); // FBX 메시 처리
    void ProcessAnimations(const aiScene* scene);

private:
    vector<shared_ptr<MeshBase>> m_meshes;
    vector<shared_ptr<GameObject>> m_gameObjects; // FBX 모델용 GameObject 저장
    //애니메이션 코드
    std::vector<AnimationClip> m_animationClips;

    std::shared_ptr<Texture> m_sharedFBXTexture;
    unordered_map<string, XMMATRIX> m_boneOffsets;
    unordered_map<string, int> m_boneNameToIndex;
};
