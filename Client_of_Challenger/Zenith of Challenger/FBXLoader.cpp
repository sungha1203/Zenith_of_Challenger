#include "FBXLoader.h"

bool FBXLoader::LoadFBXModel(const std::string& filename, const XMMATRIX& rootTransform)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded |
        aiProcess_GenNormals | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return false;
    }

    if (scene->HasAnimations()) {
        std::cout << "[FBXLoader] 애니메이션 있음! (" << scene->mNumAnimations << "개)" << std::endl;
    }
    else {
        std::cout << "[FBXLoader] 애니메이션 없음!" << std::endl;
    }

    // 1. 애니메이션 채널 확인 (한 번만 출력하면 되니까 한 곳에만 넣자)
    for (UINT i = 0; i < scene->mNumAnimations; ++i)
    {
        aiAnimation* anim = scene->mAnimations[i];
        std::cout << "애니메이션 [" << i << "] 채널 수: " << anim->mNumChannels << std::endl;

        for (UINT j = 0; j < anim->mNumChannels; ++j)
        {
            std::cout << "  Anim channel: " << anim->mChannels[j]->mNodeName.C_Str() << std::endl;
        }
    }

    // 2. 메시 본 이름 확인 (모든 메시의 본 이름 출력)
    //for (UINT i = 0; i < scene->mNumMeshes; ++i)
    //{
    //    aiMesh* mesh = scene->mMeshes[i];

    //    for (UINT j = 0; j < mesh->mNumBones; ++j)
    //    {
    //        std::cout << "  Bone name: " << mesh->mBones[j]->mName.C_Str() << std::endl;
    //    }
    //}

    ProcessNode(scene->mRootNode, scene, rootTransform);
    ProcessAnimations(scene);

    return true;
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

        // 본 정보 저장
        for (UINT i = 0; i < mesh->mNumBones; ++i)
        {
            aiBone* bone = mesh->mBones[i];
            int boneIndex = i;

            // [1] 본 이름
            string boneName = bone->mName.C_Str();

            // [2] 오프셋 행렬 (Inverse BindPose)
            aiMatrix4x4 offset = bone->mOffsetMatrix;
            offset.Transpose(); // Assimp는 row-major, DirectX는 column-major
            XMMATRIX offsetMatrix = XMLoadFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&offset));

            // 저장: 본 이름 -> 오프셋 행렬, 인덱스
            m_boneOffsets[boneName] = offsetMatrix;
            m_boneNameToIndex[boneName] = boneIndex;

            // [3] 본이 영향을 주는 정점 정보
            for (UINT j = 0; j < bone->mNumWeights; ++j)
            {
                UINT vertexId = bone->mWeights[j].mVertexId;
                float weight = bone->mWeights[j].mWeight;

                if (vertices[vertexId].boneWeights.x == 0.0f) {
                    vertices[vertexId].boneIndices.x = boneIndex;
                    vertices[vertexId].boneWeights.x = weight;
                }
                else if (vertices[vertexId].boneWeights.y == 0.0f) {
                    vertices[vertexId].boneIndices.y = boneIndex;
                    vertices[vertexId].boneWeights.y = weight;
                }
                else if (vertices[vertexId].boneWeights.z == 0.0f) {
                    vertices[vertexId].boneIndices.z = boneIndex;
                    vertices[vertexId].boneWeights.z = weight;
                }
                else if (vertices[vertexId].boneWeights.w == 0.0f) {
                    vertices[vertexId].boneIndices.w = boneIndex;
                    vertices[vertexId].boneWeights.w = weight;
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
