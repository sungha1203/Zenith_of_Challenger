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

    OutputDebugStringA("[FBXLoader] ===== LoadFBXModel ���� =====\n");

    // ��� �� �޽� �Ľ�
    ProcessNode(scene->mRootNode, scene, rootTransform);

    // �޽� �� ���
    {
        char buffer[128];
        sprintf_s(buffer, "[FBXLoader] �޽� ����: %llu\n", m_meshes.size());
        OutputDebugStringA(buffer);
    }

    // �� �ε��� ���� �α� ���
    OutputDebugStringA("[FBXLoader] �� �ε��� ���� ���:\n");
    for (const auto& [name, index] : m_boneNameToIndex)
    {
        std::string log = "  [Index " + std::to_string(index) + "] : " + name + "\n";
        OutputDebugStringA(log.c_str());
    }

    // �ִϸ��̼� ���� ���
    if (scene->HasAnimations())
    {
        char buffer[256];
        sprintf_s(buffer, "[FBXLoader] �ִϸ��̼� ����: %d\n", (int)scene->mNumAnimations);
        OutputDebugStringA(buffer);

        for (UINT i = 0; i < scene->mNumAnimations; ++i)
        {
            aiAnimation* anim = scene->mAnimations[i];

            sprintf_s(buffer, "[FBXLoader]  - [%d] �̸�: %s | Duration: %.2f | ä�� ��: %d\n",
                i, anim->mName.C_Str(), anim->mDuration, (int)anim->mNumChannels);
            OutputDebugStringA(buffer);

            for (UINT j = 0; j < anim->mNumChannels; ++j)
            {
                std::string rawName = anim->mChannels[j]->mNodeName.C_Str();

                // ���� ����: ���ӽ����̽�, ��� ����
                std::string cleanName = rawName.substr(rawName.find_last_of('|') + 1);
                std::replace(cleanName.begin(), cleanName.end(), ':', '_');

                std::string log = "    [ä�� " + std::to_string(j) + "] " + cleanName + "\n";
                OutputDebugStringA(log.c_str());
            }
        }
    }
    else
    {
        OutputDebugStringA("[FBXLoader] �ִϸ��̼� ����\n");
    }

    // �ִϸ��̼� ������ �Ľ�
    ProcessAnimations(scene);

    OutputDebugStringA("[FBXLoader] ===== LoadFBXModel �Ϸ� =====\n");
    return true;
}



shared_ptr<OtherPlayer> FBXLoader::LoadOtherPlayer(const ComPtr<ID3D12Device>& device, const unordered_map<string, shared_ptr<Texture>>& textures, const unordered_map<string, shared_ptr<Shader>>& shaders)
{    
    // [1] �÷��̾� �𵨿� ������ ��� ���� (ũ�� ����)
    XMMATRIX playerTransform = XMMatrixScaling(0.05f, 0.05f, 0.05);

    // [2] FBX �δ� ���� �� �� �ε�
    auto otherPlayer = make_shared<FBXLoader>();
    cout << "ĳ���� �ε� ��!!!!" << endl;

    if (otherPlayer->LoadFBXModel("Model/Player/Player2.fbx", playerTransform))
    {
        auto& meshes = otherPlayer->GetMeshes();
        if (meshes.empty()) {
            OutputDebugStringA("[ERROR] FBX���� �޽ø� ã�� �� �����ϴ�.\n");
            return NULL; 
        }

        // [3] Player ��ü ����
        auto player = make_shared<OtherPlayer>(device); 


        player->SetScale(XMFLOAT3{ 1.f, 1.f, 1.f }); // �⺻�� Ȯ��
        player->SetRotationY(0.f);                  // ������ ���� �ʱ�ȭ

        // [4] ��ġ �� ������ ����
        //player->SetPosition(XMFLOAT3{ 40.f, 1.7f, -50.f });
        player->SetPosition(gGameFramework->g_pos2);

        // [5] FBX �޽� ���� ���
        for (int i = 0; i < meshes.size(); ++i)
        {
            player->AddMesh(meshes[i]);
        }

        // [6] �ִϸ��̼� Ŭ�� �� �� ���� ����
        player->SetAnimationClips(otherPlayer->GetAnimationClips());
        player->SetCurrentAnimation("Idle");
        player->SetBoneOffsets(otherPlayer->GetBoneOffsets());
        player->SetBoneNameToIndex(otherPlayer->GetBoneNameToIndex());

        // [7] �ؽ�ó, ��Ƽ���� ����
        player->SetTexture(textures.at("CHARACTER"));
        //player->SetTextureIndex(textures.at("CHARACTER")->GetTextureIndex()); 
        //player->SetMaterial(materials.at("CHARACTER")); // ������ ���� �ʿ�
        player->SetShader(shaders.at("CHARACTER")); // ������ ���� �ʿ�
        player->SetDebugLineShader(shaders.at("DebugLineShader"));

        // m_player ���� ���� ��ġ
        BoundingBox playerBox;
        playerBox.Center = XMFLOAT3{ 0.f, 4.0f, 0.f };
        playerBox.Extents = { 1.0f, 4.0f, 1.0f }; // �����ϸ��� ��
        player->SetBoundingBox(playerBox);

        // [8] �� ��� StructuredBuffer�� SRV ����
        auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
        player->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

        // [9] Player ��� �� GameScene ���ο� ����
        //gGameFramework->SetPlayer(player);
        //m_player = gGameFramework->GetPlayer();

        return player;
    }
    else
    {
        OutputDebugStringA("[ERROR] �÷��̾� FBX �ε� ����!\n");
    }

}

void FBXLoader::ProcessNode(aiNode* node, const aiScene* scene, const XMMATRIX& parentTransform)
{
    // ����� ���� ����� XMMATRIX�� ��ȯ
    aiMatrix4x4 mat = node->mTransformation;
    aiMatrix4x4 transpose = mat; // Assimp�� row-major
    transpose.Transpose();       // DirectX�� column-major�̹Ƿ� ��ġ �ʿ�

    XMMATRIX nodeLocalTransform = XMLoadFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&transpose));

    // �۷ι� ��ȯ = �θ� * ����
    XMMATRIX globalTransform = XMMatrixMultiply(nodeLocalTransform, parentTransform);

    // �޽ð� �����ϸ� ó��
    for (UINT i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        //std::cout << "[Mesh] Name: " << mesh->mName.C_Str() << std::endl;

        auto object = ProcessMesh(mesh, scene, globalTransform);
        m_gameObjects.push_back(object);
    }

    // �ڽ� ��� ��ȸ
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

    // [1] �ִϸ��̼� ���� �ִ� ���
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

        // Bone ���� ����
        for (UINT i = 0; i < mesh->mNumBones; ++i)
        {
            aiBone* bone = mesh->mBones[i];
            string boneName = bone->mName.C_Str();

            aiMatrix4x4 offset = bone->mOffsetMatrix;
            offset.Transpose();
            XMMATRIX offsetMatrix = XMLoadFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&offset));

            // �߸��� �ε��� �� ���� �ε��� �ο� ������� ����
            if (!m_boneNameToIndex.contains(boneName))
            {
                m_boneNameToIndex[boneName] = static_cast<int>(m_boneNameToIndex.size());
                m_boneOffsets[boneName] = offsetMatrix;
            }

            int boneIndex = m_boneNameToIndex[boneName];

            // �� ���� �ݿ�
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
    // [2] �Ϲ� �޽� (�ؽ�ó ����)
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

        // �ؽ�ó�� GameScene���� �ϰ� �Ҵ�
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
