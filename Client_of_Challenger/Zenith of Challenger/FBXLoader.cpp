#include "FBXLoader.h"
#include"OtherPlayer.h"
bool FBXLoader::LoadFBXModel(const std::string& filename, const XMMATRIX& rootTransform)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate |
        aiProcess_ConvertToLeftHanded |
        aiProcess_GenNormals |
        aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::string errorMsg = "[Assimp] Load Failed: ";
        errorMsg += importer.GetErrorString();
        OutputDebugStringA(errorMsg.c_str());
        return false;
    }

    OutputDebugStringA("[FBXLoader] ===== LoadFBXModel 시작 =====\n");

    // 노드 및 메시 파싱
    ProcessNode(scene->mRootNode, scene, rootTransform);

    // 메시 수 출력
    {
        char buffer[128];
        sprintf_s(buffer, "[FBXLoader] 메시 개수: %llu\n", m_meshes.size());
        OutputDebugStringA(buffer);
    }

    // 본 인덱스 매핑 로그 출력
    OutputDebugStringA("[FBXLoader] 본 인덱스 매핑 결과:\n");
    for (const auto& [name, index] : m_boneNameToIndex)
    {
        std::string log = "  [Index " + std::to_string(index) + "] : " + name + "\n";
        OutputDebugStringA(log.c_str());
    }

    // 애니메이션 정보 출력
    if (scene->HasAnimations())
    {
        char buffer[256];
        sprintf_s(buffer, "[FBXLoader] 애니메이션 개수: %d\n", (int)scene->mNumAnimations);
        OutputDebugStringA(buffer);

        for (UINT i = 0; i < scene->mNumAnimations; ++i)
        {
            aiAnimation* anim = scene->mAnimations[i];

            sprintf_s(buffer, "[FBXLoader]  - [%d] 이름: %s | Duration: %.2f | 채널 수: %d\n",
                i, anim->mName.C_Str(), anim->mDuration, (int)anim->mNumChannels);
            OutputDebugStringA(buffer);

            for (UINT j = 0; j < anim->mNumChannels; ++j)
            {
                std::string rawName = anim->mChannels[j]->mNodeName.C_Str();

                // 포맷 정제: 네임스페이스, 경로 제거
                std::string cleanName = rawName.substr(rawName.find_last_of('|') + 1);
                std::replace(cleanName.begin(), cleanName.end(), ':', '_');

                std::string log = "    [채널 " + std::to_string(j) + "] " + cleanName + "\n";
                OutputDebugStringA(log.c_str());
            }
        }
    }
    else
    {
        OutputDebugStringA("[FBXLoader] 애니메이션 없음\n");
    }

    // 애니메이션 데이터 파싱
    ProcessAnimations(scene);

    OutputDebugStringA("[FBXLoader] ===== LoadFBXModel 완료 =====\n");
    return true;
}



shared_ptr<OtherPlayer> FBXLoader::LoadOtherPlayer(const ComPtr<ID3D12Device>& device, const unordered_map<string, shared_ptr<Texture>>& textures, const unordered_map<string, shared_ptr<Shader>>& shaders)
{    
    // [1] 플레이어 모델용 스케일 행렬 설정 (크기 조절)
    XMMATRIX playerTransform = XMMatrixScaling(0.05f, 0.05f, 0.05);

    // [2] FBX 로더 생성 및 모델 로드
    auto otherPlayer = make_shared<FBXLoader>();
    cout << "캐릭터 로드 중!!!!" << endl;

    if (otherPlayer->LoadFBXModel("Model/Player/Player2.fbx", playerTransform))
    {
        auto& meshes = otherPlayer->GetMeshes();
        if (meshes.empty()) {
            OutputDebugStringA("[ERROR] FBX에서 메시를 찾을 수 없습니다.\n");
            return NULL; 
        }

        // [3] Player 객체 생성
        auto player = make_shared<OtherPlayer>(device); 


        player->SetScale(XMFLOAT3{ 1.f, 1.f, 1.f }); // 기본값 확정
        player->SetRotationY(0.f);                  // 정면을 보게 초기화

        // [4] 위치 및 스케일 설정
        //player->SetPosition(XMFLOAT3{ 40.f, 1.7f, -50.f });
        player->SetPosition(gGameFramework->g_pos2);

        // [5] FBX 메시 전부 등록
        for (int i = 0; i < meshes.size(); ++i)
        {
            player->AddMesh(meshes[i]);
        }

        // [6] 애니메이션 클립 및 본 정보 설정
        player->SetAnimationClips(otherPlayer->GetAnimationClips());
        player->SetCurrentAnimation("Idle");
        player->SetBoneOffsets(otherPlayer->GetBoneOffsets());
        player->SetBoneNameToIndex(otherPlayer->GetBoneNameToIndex());

        // [7] 텍스처, 머티리얼 설정
        player->SetTexture(textures.at("CHARACTER"));
        //player->SetTextureIndex(textures.at("CHARACTER")->GetTextureIndex()); 
        //player->SetMaterial(materials.at("CHARACTER")); // 없으면 생성 필요
        player->SetShader(shaders.at("CHARACTER")); // 없으면 생성 필요
        player->SetDebugLineShader(shaders.at("DebugLineShader"));

        // m_player 생성 이후 위치
        BoundingBox playerBox;
        playerBox.Center = XMFLOAT3{ 0.f, 4.0f, 0.f };
        playerBox.Extents = { 1.0f, 4.0f, 1.0f }; // 스케일링된 값
        player->SetBoundingBox(playerBox);

        // [8] 본 행렬 StructuredBuffer용 SRV 생성
        auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
        player->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

        // [9] Player 등록 및 GameScene 내부에 저장
        //gGameFramework->SetPlayer(player);
        //m_player = gGameFramework->GetPlayer();

        return player;
    }
    else
    {
        OutputDebugStringA("[ERROR] 플레이어 FBX 로드 실패!\n");
    }

}

void FBXLoader::ProcessNode(aiNode* node, const aiScene* scene, const XMMATRIX& parentTransform)
{
    // 노드의 로컬 행렬을 XMMATRIX로 변환
    aiMatrix4x4 mat = node->mTransformation;
    aiMatrix4x4 transpose = mat; // Assimp는 row-major
    transpose.Transpose();       // DirectX는 column-major이므로 전치 필요

    XMMATRIX nodeLocalTransform = XMLoadFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&transpose));

    // 글로벌 변환 = 부모 * 로컬
    XMMATRIX globalTransform = XMMatrixMultiply(nodeLocalTransform, parentTransform);

    // 메시가 존재하면 처리
    for (UINT i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        //std::cout << "[Mesh] Name: " << mesh->mName.C_Str() << std::endl;

        auto object = ProcessMesh(mesh, scene, globalTransform);
        m_gameObjects.push_back(object);
    }

    // 자식 노드 순회
    for (UINT i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, globalTransform);
    }
}

shared_ptr<GameObject> FBXLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene, const XMMATRIX& globalTransform)
{
    float scaleFactor = 1.0f;
    auto device = gGameFramework->GetDevice();
    auto commandList = gGameFramework->GetCommandList();

    // [1] 애니메이션 본이 있는 경우
    if (mesh->HasBones())
    {
        std::vector<SkinnedVertex> vertices(mesh->mNumVertices);

        for (UINT i = 0; i < mesh->mNumVertices; ++i)
        {
            XMFLOAT3 pos = { -mesh->mVertices[i].x * 0.9f, mesh->mVertices[i].y * 0.9f, mesh->mVertices[i].z * 0.9f };
            XMStoreFloat3(&vertices[i].position, XMVector3Transform(XMLoadFloat3(&pos), globalTransform));

            XMFLOAT3 normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
            XMStoreFloat3(&vertices[i].normal, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&normal), globalTransform)));

            vertices[i].uv = mesh->HasTextureCoords(0) ?
                XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) :
                XMFLOAT2(0.0f, 0.0f);

            vertices[i].boneIndices = XMUINT4(0, 0, 0, 0);
            vertices[i].boneWeights = XMFLOAT4(0.f, 0.f, 0.f, 0.f);
        }

        // Bone 정보 저장
        for (UINT i = 0; i < mesh->mNumBones; ++i)
        {
            aiBone* bone = mesh->mBones[i];
            string boneName = bone->mName.C_Str();

            aiMatrix4x4 offset = bone->mOffsetMatrix;
            offset.Transpose();
            XMMATRIX offsetMatrix = XMLoadFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&offset));

            // 잘못된 인덱스 → 전역 인덱스 부여 방식으로 수정
            if (!m_boneNameToIndex.contains(boneName))
            {
                m_boneNameToIndex[boneName] = static_cast<int>(m_boneNameToIndex.size());
                m_boneOffsets[boneName] = offsetMatrix;
            }

            int boneIndex = m_boneNameToIndex[boneName];

            // 본 영향 반영
            for (UINT j = 0; j < bone->mNumWeights; ++j)
            {
                UINT vertexId = bone->mWeights[j].mVertexId;
                float weight = bone->mWeights[j].mWeight;

                auto& vertex = vertices[vertexId];

                if (vertex.boneWeights.x == 0.0f) {
                    vertex.boneIndices.x = boneIndex;
                    vertex.boneWeights.x = weight;
                }
                else if (vertex.boneWeights.y == 0.0f) {
                    vertex.boneIndices.y = boneIndex;
                    vertex.boneWeights.y = weight;
                }
                else if (vertex.boneWeights.z == 0.0f) {
                    vertex.boneIndices.z = boneIndex;
                    vertex.boneWeights.z = weight;
                }
                else if (vertex.boneWeights.w == 0.0f) {
                    vertex.boneIndices.w = boneIndex;
                    vertex.boneWeights.w = weight;
                }
            }
        }

        auto meshPtr = std::make_shared<Mesh<SkinnedVertex>>(device, commandList, vertices);
        m_meshes.push_back(meshPtr);

        auto gameObject = std::make_shared<GameObject>(device);
        gameObject->SetMesh(meshPtr);
        gameObject->SetWorldMatrix(globalTransform);
        return gameObject;
    }
    // [2] 일반 메시 (텍스처 있음)
    else
    {
        std::vector<TextureVertex> vertices;
        for (UINT i = 0; i < mesh->mNumVertices; ++i)
        {
            TextureVertex vertex{};
            XMFLOAT3 pos = { mesh->mVertices[i].x * scaleFactor, mesh->mVertices[i].y * scaleFactor, mesh->mVertices[i].z * scaleFactor };
            XMStoreFloat3(&vertex.position, XMVector3Transform(XMLoadFloat3(&pos), globalTransform));

            XMFLOAT3 normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
            XMStoreFloat3(&vertex.normal, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&normal), globalTransform)));

            vertex.uv = mesh->HasTextureCoords(0) ?
                XMFLOAT2(mesh->mTextureCoords[0][i].x, 1.0f - mesh->mTextureCoords[0][i].y) :
                XMFLOAT2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        auto meshPtr = std::make_shared<Mesh<TextureVertex>>(device, commandList, vertices);
        m_meshes.push_back(meshPtr);


        auto gameObject = std::make_shared<GameObject>(device);
        gameObject->SetMesh(meshPtr);
        gameObject->SetBaseColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
        gameObject->SetWorldMatrix(globalTransform);

        // 텍스처는 GameScene에서 일괄 할당
        return gameObject;
    }
}


void FBXLoader::ProcessAnimations(const aiScene* scene)
{
    if (!scene->HasAnimations()) return;

    for (UINT i = 0; i < scene->mNumAnimations; ++i)
    {
        aiAnimation* aiAnim = scene->mAnimations[i];

        AnimationClip clip;
        clip.name = aiAnim->mName.C_Str();
        clip.duration = static_cast<float>(aiAnim->mDuration);
        clip.ticksPerSecond = static_cast<float>(aiAnim->mTicksPerSecond != 0 ? aiAnim->mTicksPerSecond : 30.0f);

        for (UINT j = 0; j < aiAnim->mNumChannels; ++j)
        {
            aiNodeAnim* aiNodeAnim = aiAnim->mChannels[j];
            BoneAnimation boneAnim;
            boneAnim.boneName = aiNodeAnim->mNodeName.C_Str();

            for (UINT k = 0; k < aiNodeAnim->mNumPositionKeys; ++k)
            {
                Keyframe keyframe;
                keyframe.time = static_cast<float>(aiNodeAnim->mPositionKeys[k].mTime);
                keyframe.position = XMFLOAT3(
                    aiNodeAnim->mPositionKeys[k].mValue.x,
                    aiNodeAnim->mPositionKeys[k].mValue.y,
                    aiNodeAnim->mPositionKeys[k].mValue.z);

                if (k < aiNodeAnim->mNumRotationKeys)
                {
                    keyframe.rotation = XMFLOAT4(
                        aiNodeAnim->mRotationKeys[k].mValue.x,
                        aiNodeAnim->mRotationKeys[k].mValue.y,
                        aiNodeAnim->mRotationKeys[k].mValue.z,
                        aiNodeAnim->mRotationKeys[k].mValue.w);
                }

                if (k < aiNodeAnim->mNumScalingKeys)
                {
                    keyframe.scale = XMFLOAT3(
                        aiNodeAnim->mScalingKeys[k].mValue.x,
                        aiNodeAnim->mScalingKeys[k].mValue.y,
                        aiNodeAnim->mScalingKeys[k].mValue.z);
                }

                boneAnim.keyframes.push_back(keyframe);
            }

            clip.boneAnimations[boneAnim.boneName] = boneAnim;
        }

        m_animationClips.push_back(clip);
    }
}
