#include "FBXLoader.h"
#include"OtherPlayer.h"
#include<set>
#define DEG_TO_RAD (XM_PI / 180.0f)
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

	OutputDebugStringA("[FBXLoader] ===== LoadFBXModel =====\n");


	ProcessNode(scene->mRootNode, scene, rootTransform);
	BuildBoneHierarchy(scene->mRootNode, "", XMMatrixIdentity());

    // 메시 수 출력
    {
        char buffer[128];
        sprintf_s(buffer, "[FBXLoader] 메시 개수: %llu\n", m_meshes.size());
        //OutputDebugStringA(buffer);
    }

    // 본 인덱스 매핑 로그 출력
    //OutputDebugStringA("[FBXLoader] 본 인덱스 매핑 결과:\n");
    for (const auto& [name, index] : m_boneNameToIndex)
    {
        std::string log = "  [Index " + std::to_string(index) + "] : " + name + "\n";
        //OutputDebugStringA(log.c_str());
    }

    // 애니메이션 정보 출력
    if (scene->HasAnimations())
    {
        char buffer[256];
        sprintf_s(buffer, "[FBXLoader] 애니메이션 개수: %d\n", (int)scene->mNumAnimations);
        //OutputDebugStringA(buffer);

		for (UINT i = 0; i < scene->mNumAnimations; ++i)
		{
			aiAnimation* anim = scene->mAnimations[i];

            sprintf_s(buffer, "[FBXLoader]  - [%d] 이름: %s | Duration: %.2f | 채널 수: %d\n",
                i, anim->mName.C_Str(), anim->mDuration, (int)anim->mNumChannels);
            //OutputDebugStringA(buffer);

			for (UINT j = 0; j < anim->mNumChannels; ++j)
			{
				std::string rawName = anim->mChannels[j]->mNodeName.C_Str();

				std::string cleanName = rawName.substr(rawName.find_last_of('|') + 1);
				std::replace(cleanName.begin(), cleanName.end(), ':', '_');

                std::string log = "    [채널 " + std::to_string(j) + "] " + cleanName + "\n";
                //OutputDebugStringA(log.c_str());
            }
        }
    }
    else
    {
        OutputDebugStringA("[FBXLoader] 애니메이션 없음\n");
    }

	ProcessAnimations(scene);
	return true;
}



shared_ptr<OtherPlayer> FBXLoader::LoadOtherPlayer(const ComPtr<ID3D12Device>& device, const unordered_map<string, shared_ptr<Texture>>& textures, const unordered_map<string, shared_ptr<Shader>>& shaders)
{
	//XMMATRIX playerTransform = XMMatrixScaling(0.05f, 0.05f, 0.05);

	auto otherPlayer = make_shared<FBXLoader>();

	//if (otherPlayer->LoadFBXModel("Model/Player/ExportCharacter_AddRunning.fbx", XMMatrixIdentity()))
	//if (otherPlayer->LoadFBXModel("Model/Player/ExportCharacter_AddKick.fbx", XMMatrixIdentity()))
	if (otherPlayer->LoadFBXModel("Model/Player/ExportCharacter_AddHook.fbx", XMMatrixIdentity()))
	{
		auto& meshes = otherPlayer->GetMeshes();

		auto player = make_shared<OtherPlayer>(device);

		//player->SetScale(XMFLOAT3{ 0.0005,0.0005,0.0005 });
		player->SetRotationY(0.f);                  

		for (int i = 0; i < meshes.size(); ++i)
		{
			player->AddMesh(meshes[i]);
		}

		player->SetAnimationClips(otherPlayer->GetAnimationClips());
		player->SetCurrentAnimation("Idle");
		//player->SetCurrentAnimation("Walking");
		player->SetBoneOffsets(otherPlayer->GetBoneOffsets());
		player->SetBoneNameToIndex(otherPlayer->GetBoneNameToIndex());
		player->SetBoneHierarchy(otherPlayer->GetBoneHierarchy());
		player->SetstaticNodeTransforms(otherPlayer->GetStaticNodeTransforms());

		player->SetTexture(textures.at("CHARACTER"));
		player->SetTextureIndex(textures.at("CHARACTER")->GetTextureIndex());
		player->SetShader(shaders.at("CHARACTER")); 
		player->SetDebugLineShader(shaders.at("DebugLineShader"));

		BoundingBox playerBox;
		playerBox.Center = XMFLOAT3{ 0.f, 4.0f, 0.f };
		playerBox.Extents = { 1.0f, 4.0f, 1.0f }; 
		player->SetBoundingBox(playerBox);
		

		auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
		player->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

		return player;
	}
	else
	{
		OutputDebugStringA("[ERROR]\n");
	}

}

void FBXLoader::PrintBoneHierarchy(aiNode* node, int depth)
{
	for (int i = 0; i < depth; ++i)
		OutputDebugStringA("  ");

	OutputDebugStringA(("[Node] " + std::string(node->mName.C_Str()) + "\n").c_str());

	for (UINT i = 0; i < node->mNumChildren; ++i)
	{
		PrintBoneHierarchy(node->mChildren[i], depth + 1);
	}
}

void FBXLoader::DumpBoneHierarchy(const aiScene* scene)
{
	if (!scene || !scene->mRootNode) return;

	OutputDebugStringA("================ Bone Hierarchy Dump ================\n");
	PrintBoneHierarchy(scene->mRootNode);
	OutputDebugStringA("=====================================================\n");
}


void FBXLoader::ProcessNode(aiNode* node, const aiScene* scene, const XMMATRIX& parentTransform)
{

	aiMatrix4x4 mat = node->mTransformation; 
	aiMatrix4x4 transpose = mat;
	transpose.Transpose(); 

	XMMATRIX nodeLocalTransform = XMLoadFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&transpose));
	XMMATRIX globalTransform = XMMatrixMultiply(nodeLocalTransform, parentTransform);

	for (UINT i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		auto object = ProcessMesh(mesh, scene, globalTransform);
		m_gameObjects.push_back(object);
	}

	for (UINT i = 0; i < node->mNumChildren; ++i)
	{
		ProcessNode(node->mChildren[i], scene, globalTransform);
	}
}

shared_ptr<GameObject> FBXLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene, const XMMATRIX& globalTransform)
{
	float scaleFactor = 1.0f;
	auto device = gGameFramework->GetDevice();
	auto commandList = gGameFramework->GetCommandList();

	if (mesh->HasBones())
	{
		std::vector<SkinnedVertex> vertices(mesh->mNumVertices);

		for (UINT i = 0; i < mesh->mNumVertices; ++i)
		{
			XMFLOAT3 pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertices[i].position = pos; 

			vertices[i].normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z }; 
			vertices[i].uv = mesh->HasTextureCoords(0) ?
				XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) :
				XMFLOAT2(0.0f, 0.0f);

			vertices[i].boneIndices = XMUINT4(0, 0, 0, 0);
			vertices[i].boneWeights = XMFLOAT4(0.f, 0.f, 0.f, 0.f);

		}

		for (UINT i = 0; i < mesh->mNumBones; ++i)
		{
			aiBone* bone = mesh->mBones[i];
			string boneName = bone->mName.C_Str();

			aiMatrix4x4 offset = bone->mOffsetMatrix;
			offset.Transpose();
			XMMATRIX offsetMatrix = XMLoadFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&offset));



			if (!m_boneNameToIndex.contains(boneName))
			{
				m_boneNameToIndex[boneName] = static_cast<int>(m_boneNameToIndex.size());
				m_boneOffsets[m_boneNameToIndex[boneName]] = offsetMatrix;
			}

			int boneIndex = m_boneNameToIndex[boneName];
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
		//DumpBoneHierarchy(scene);
		return gameObject;
	}
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

		return gameObject;
	}
}


//void FBXLoader::ProcessAnimations(const aiScene* scene)
//{
//	if (!scene->HasAnimations()) return;
//
//	for (UINT i = 0; i < scene->mNumAnimations; ++i)
//	{
//		aiAnimation* aiAnim = scene->mAnimations[i];
//
//		AnimationClip clip;
//		clip.name = aiAnim->mName.C_Str();
//		clip.duration = static_cast<float>(aiAnim->mDuration);
//		clip.ticksPerSecond = static_cast<float>(aiAnim->mTicksPerSecond != 0 ? aiAnim->mTicksPerSecond : 30.0f);
//
//		for (UINT j = 0; j < aiAnim->mNumChannels; ++j)//j 4 k 42
//		{
//			aiNodeAnim* aiNodeAnim = aiAnim->mChannels[j];
//			BoneAnimation boneAnim;
//			boneAnim.boneName = aiNodeAnim->mNodeName.C_Str();
//
//			UINT keyCount = std::max({ aiNodeAnim->mNumPositionKeys, aiNodeAnim->mNumRotationKeys, aiNodeAnim->mNumScalingKeys });
//
//			for (UINT k = 0; k < keyCount; ++k)
//			{
//				Keyframe keyframe;
//
//				//keyframe.time = static_cast<float>(aiNodeAnim->mPositionKeys[k].mTime) / clip.ticksPerSecond;
//				if (k < aiNodeAnim->mNumPositionKeys)
//				{
//					keyframe.time = (float)aiNodeAnim->mPositionKeys[k].mTime;
//					keyframe.position = {
//						aiNodeAnim->mPositionKeys[k].mValue.x,
//						aiNodeAnim->mPositionKeys[k].mValue.y,
//						aiNodeAnim->mPositionKeys[k].mValue.z
//					};
//				}
//
//				if (k < aiNodeAnim->mNumRotationKeys)
//				{
//					keyframe.time = (float)aiNodeAnim->mRotationKeys[k].mTime;
//					keyframe.rotation = {
//						aiNodeAnim->mRotationKeys[k].mValue.x,
//						aiNodeAnim->mRotationKeys[k].mValue.y,
//						aiNodeAnim->mRotationKeys[k].mValue.z,
//						aiNodeAnim->mRotationKeys[k].mValue.w
//					};
//				}
//
//				if (k < aiNodeAnim->mNumScalingKeys)
//				{
//					keyframe.time = (float)aiNodeAnim->mScalingKeys[k].mTime;
//					keyframe.scale = {
//						aiNodeAnim->mScalingKeys[k].mValue.x,
//						aiNodeAnim->mScalingKeys[k].mValue.y,
//						aiNodeAnim->mScalingKeys[k].mValue.z
//					};
//
//				}
//
//				boneAnim.keyframes.push_back(keyframe);
//			}
//
//			clip.boneAnimations[boneAnim.boneName] = boneAnim;
//		}
//
//		m_animationClips.push_back(clip);
//	}
//}
void FixQuaternionSequenceSigns(std::vector<Keyframe>& keyframes)
{
	for (size_t i = 1; i < keyframes.size(); ++i)
	{
		const XMFLOAT4& prevQuat = keyframes[i - 1].rotation;
		XMFLOAT4& currQuat = keyframes[i].rotation;

		// Dot Product 계산
		float dot =
			prevQuat.x * currQuat.x +
			prevQuat.y * currQuat.y +
			prevQuat.z * currQuat.z +
			prevQuat.w * currQuat.w;

		// dot < 0 이면 부호 반전
		if (dot < 0.0f)
		{
			char msg[256];
			sprintf_s(msg, "[FixQuat] Flip at frame %zu → dot=%.4f\n", i, dot);
			OutputDebugStringA(msg);
			currQuat.x = -currQuat.x;
			currQuat.y = -currQuat.y;
			currQuat.z = -currQuat.z;
			currQuat.w = -currQuat.w;
		}
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
		clip.ticksPerSecond = (aiAnim->mTicksPerSecond != 0.0) ? static_cast<float>(aiAnim->mTicksPerSecond) : 30.0f;
		// 디버그 출력
		char debugBuffer[256];
		sprintf_s(debugBuffer,
			"[FBXLoader] Animation %u Name: '%s' | Duration: %.4f | TicksPerSecond: %.4f\n",
			i,
			clip.name.c_str(),
			clip.duration,
			clip.ticksPerSecond);
		OutputDebugStringA(debugBuffer);

		for (UINT j = 0; j < aiAnim->mNumChannels; ++j)
		{
			aiNodeAnim* aiNodeAnim = aiAnim->mChannels[j];
			BoneAnimation boneAnim;
			boneAnim.boneName = aiNodeAnim->mNodeName.C_Str();

			UINT numRotKeys = aiNodeAnim->mNumRotationKeys;

			for (UINT k = 0; k < numRotKeys; ++k)
			{
				const aiQuatKey& key = aiNodeAnim->mRotationKeys[k];
				if (isnan(key.mValue.w) || isnan(key.mValue.x) || isnan(key.mValue.y) || isnan(key.mValue.z)) {
					OutputDebugStringA("[경고] NaN 쿼터니언 키 발견!\n");
				}
				aiQuaternion q = aiNodeAnim->mRotationKeys[k].mValue;
				if (fabs(q.w) < 0.01f || isnan(q.w))
				{
					char msg[256];
					sprintf_s(msg, "[의심 회전값] Bone: %s | time=%.4f | quat=(%.4f, %.4f, %.4f, %.4f)\n",
						aiNodeAnim->mNodeName.C_Str(),
						(float)aiNodeAnim->mRotationKeys[k].mTime,
						q.x, q.y, q.z, q.w);
					OutputDebugStringA(msg);
				}
				Keyframe keyframe;

				//1. Rotation 기준 시간
				keyframe.time = static_cast<float>(aiNodeAnim->mRotationKeys[k].mTime);

				//2. Rotation
				//const auto& rot = aiNodeAnim->mRotationKeys[k].mValue;
				//keyframe.rotation = XMFLOAT4(rot.x, rot.y, rot.z, rot.w);
				// 추가: 정규화 및 보정
				//aiQuaternion q = key.mValue;
				q.Normalize();
				
				keyframe.rotation = XMFLOAT4(q.x, q.y, q.z, q.w);
				//keyframe.rotation = XMFLOAT4((float)rot.x, (float)rot.y, (float)rot.z, (float)rot.w); // 만약 XMQuaternion 쪽이 (x,y,z,w) 기반이라면 OK				
				//3. Position: 가장 가까운 시간 찾기
				if (aiNodeAnim->mNumPositionKeys > 0)
				{
					

					double minDiff = DBL_MAX;
					UINT bestIndex = 0;
					for (UINT p = 0; p < aiNodeAnim->mNumPositionKeys; ++p)
					{
						double diff = fabs(aiNodeAnim->mPositionKeys[p].mTime - keyframe.time);
						if (diff < minDiff)
						{
							minDiff = diff;
							bestIndex = p;
						}
					}
					const auto& pos = aiNodeAnim->mPositionKeys[bestIndex].mValue;
					keyframe.position = XMFLOAT3(pos.x, pos.y, pos.z);
				}
				else
				{
					keyframe.position = XMFLOAT3(0, 0, 0); // fallback
				}

				//4. Scale: 가장 가까운 시간 찾기
				if (aiNodeAnim->mNumScalingKeys > 0)
				{
					double minDiff = DBL_MAX;
					UINT bestIndex = 0;
					for (UINT s = 0; s < aiNodeAnim->mNumScalingKeys; ++s)
					{
						double diff = fabs(aiNodeAnim->mScalingKeys[s].mTime - keyframe.time);
						if (diff < minDiff)
						{
							minDiff = diff;
							bestIndex = s;
						}
					}
					const auto& scl = aiNodeAnim->mScalingKeys[bestIndex].mValue;
					keyframe.scale = XMFLOAT3(scl.x, scl.y, scl.z);
				}
				else
				{
					keyframe.scale = XMFLOAT3(1, 1, 1); // fallback
				}
				boneAnim.keyframes.push_back(keyframe);
			}

			FixQuaternionSequenceSigns(boneAnim.keyframes);
			clip.boneAnimations[boneAnim.boneName] = boneAnim;
		}

		m_animationClips.push_back(clip);
	}
}

void FBXLoader::BuildBoneHierarchy(aiNode* node, const std::string& parentName, const XMMATRIX& accumulatedTransform)
{
	std::string nodeName = node->mName.C_Str();

	aiMatrix4x4 localMat = node->mTransformation;
	localMat.Transpose(); 
	XMMATRIX localTransform = XMLoadFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&localMat));

	XMMATRIX combinedTransform = XMMatrixMultiply(localTransform, accumulatedTransform);

	if (nodeName.find("_$AssimpFbx$") != std::string::npos)
	{
		for (UINT i = 0; i < node->mNumChildren; ++i)
			BuildBoneHierarchy(node->mChildren[i], parentName, combinedTransform);
		return;
	}

	m_nodeNameToLocalTransform[nodeName] = combinedTransform;

	if (!parentName.empty())
		m_boneHierarchy[nodeName] = parentName;

	for (UINT i = 0; i < node->mNumChildren; ++i)
		BuildBoneHierarchy(node->mChildren[i], nodeName, XMMatrixIdentity()); 
}
