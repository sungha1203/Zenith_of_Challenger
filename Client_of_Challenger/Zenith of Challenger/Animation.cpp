// animation.cpp
#include "animation.h"
#include <DirectXMath.h>
#include <queue>
#include<unordered_set>
#include<functional>

using namespace DirectX;

XMMATRIX InterpolateKeyframes(const Keyframe& a, const Keyframe& b, float t)
{
	XMVECTOR posA = XMLoadFloat3(&a.position);
	XMVECTOR posB = XMLoadFloat3(&b.position);
	XMVECTOR scaleA = XMLoadFloat3(&a.scale);
	XMVECTOR scaleB = XMLoadFloat3(&b.scale);

	XMVECTOR rotA = XMQuaternionNormalize(XMLoadFloat4(&a.rotation));
	XMVECTOR rotB = XMQuaternionNormalize(XMLoadFloat4(&b.rotation));

	XMVECTOR pos = XMVectorLerp(posA, posB, t);
	XMVECTOR scale = XMVectorLerp(scaleA, scaleB, t);
	XMVECTOR rot = XMQuaternionSlerp(rotA, rotB, t);

	XMMATRIX T = XMMatrixTranslationFromVector(pos);
	XMMATRIX R = XMMatrixRotationQuaternion(rot);
	XMMATRIX S = XMMatrixScalingFromVector(scale);

	return S * R * T;

}

XMMATRIX AnimationClip::GetBoneTransform(const BoneAnimation& boneAnim, float time) const
{
	const auto& keys = boneAnim.keyframes;
	if (keys.empty()) return XMMatrixIdentity();
	if (keys.size() == 1) return InterpolateKeyframes(keys[0], keys[0], 0);

	Keyframe a = keys[0];
	Keyframe b = keys[0];

	for (size_t i = 0; i < keys.size() - 1; ++i)
	{
		if (time < keys[i + 1].time)
		{
			a = keys[i];
			b = keys[i + 1];
			break;
		}
	}

	float lerpFactor = (time - a.time) / (b.time - a.time);
	XMMATRIX localTransform = InterpolateKeyframes(a, b, lerpFactor);
	return localTransform;
}

std::pair<std::vector<XMMATRIX>, std::unordered_map<std::string, int>> AnimationClip::GetBoneTransforms(float time, unordered_map<std::string, int>boneNametoIndex, unordered_map<std::string, string>boneHierarchy, unordered_map<std::string, XMMATRIX>staticNodeTransforms) const
{
	std::pair<std::vector<XMMATRIX>, std::unordered_map<std::string, int>> p;
	std::unordered_map<std::string, int> animBoneIndex;
	int nextIndex = 0;

	for (const auto& [boneName, _] : boneAnimations)
	{
		animBoneIndex[boneName] = nextIndex++;
	}
	// Topological sort
	vector<string> sortedBones = TopologicalSort(boneHierarchy);

	// boneName -> globalTransform
	unordered_map<string, XMMATRIX> boneGlobalTransforms;

	for (const auto& boneName : sortedBones)
	{
		XMMATRIX localTransform = XMMatrixIdentity();
		if (boneAnimations.contains(boneName))
			localTransform = GetBoneTransform(boneAnimations.at(boneName), time);
		else if (staticNodeTransforms.contains(boneName))
			localTransform = staticNodeTransforms.at(boneName);

		if (boneName == "RigSpine0")
		{
			localTransform = XMMatrixIdentity();
		}

		if (boneHierarchy.contains(boneName))
		{
			const string& parent = boneHierarchy.at(boneName);
			boneGlobalTransforms[boneName] = XMMatrixMultiply(localTransform, boneGlobalTransforms[parent]);
		}
		else
		{
			boneGlobalTransforms[boneName] = localTransform;
		}
		{
			XMFLOAT4X4 mat;
			XMStoreFloat4x4(&mat, boneGlobalTransforms[boneName]);
			OutputDebugStringA(std::format(
				"[BoneAccumulation] {} -> T({:.3f}, {:.3f}, {:.3f})\n",
				boneName, mat._41, mat._42, mat._43
			).c_str());
		}
		if (boneName == "RigSpine0")
		{
			XMFLOAT4X4 local;
			XMStoreFloat4x4(&local, localTransform);
			OutputDebugStringA(std::format("[RigSpine0 Local] T({:.3f}, {:.3f}, {:.3f})\n", local._41, local._42, local._43).c_str());
		}
	}

	// 최종 BoneMatrices 만들기
	vector<XMMATRIX> result(boneNametoIndex.size(), XMMatrixIdentity());

	for (const auto& [boneName, vertexIndex] : boneNametoIndex)
	{
		if (boneGlobalTransforms.contains(boneName))
		{
			//result[vertexIndex] = XMMatrixMultiply(boneGlobalTransforms[boneName], boneoffset.at(vertexIndex));
			result[vertexIndex] = boneGlobalTransforms[boneName];
		}
	}

	p.first = result;
	p.second = animBoneIndex;
	return p;
}
// 1. 부모-자식 관계 기반으로 Topological Sort
vector<string> TopologicalSort(const unordered_map<string, string>& boneHierarchy)
{
	std::vector<std::string> result;
	std::unordered_map<std::string, bool> visited;

	// 모든 노드를 candidates에 모은다
	std::unordered_map<std::string, bool> candidates;
	for (const auto& [node, parent] : boneHierarchy)
	{
		candidates[node] = true;
		candidates[parent] = true;
	}

	// DFS 함수
	std::function<void(const std::string&)> dfs = [&](const std::string& nodeName)
		{
			if (visited[nodeName]) return;
			visited[nodeName] = true;

			for (const auto& [child, parent] : boneHierarchy)
			{
				if (parent == nodeName)
				{
					dfs(child);
				}
			}
			result.push_back(nodeName);
		};

	// 모든 후보 노드에 대해 DFS
	for (const auto& [node, _] : candidates)
	{
		if (!visited[node])
		{
			dfs(node);
		}
	}

	std::reverse(result.begin(), result.end());
	return result;
}

std::vector<XMMATRIX> AnimationClip::GetBoneTransforms(
	float time,
	const std::unordered_map<std::string, int>& boneNameToIndex,
	const std::unordered_map<std::string, std::string>& boneHierarchy,
	unordered_map<int, XMMATRIX> boneOffsets,
	const std::unordered_map<std::string, XMMATRIX>& initialGlobalTransforms) const
{
	std::vector<XMMATRIX> result;

	// (2) bone 이름 정렬 (topological sort)
	std::vector<std::string> sortedBones = TopologicalSort(boneHierarchy);
	// (3) 이름 → 글로벌 변환 매트릭스
	std::unordered_map<std::string, XMMATRIX> boneGlobalTransforms;

	XMMATRIX globalinverseTransform = XMMatrixIdentity();


	// (4) 본별 글로벌 변환 계산
	for (const auto& boneName : sortedBones)
	{
		XMMATRIX localTransform = XMMatrixIdentity();
		if (boneAnimations.contains(boneName))
		{
			localTransform = GetBoneTransform(boneAnimations.at(boneName), time);
		}
		else
		{
			// 애니메이션 없으면 최초 글로벌 트랜스폼 기준으로
			auto iter = initialGlobalTransforms.find(boneName);
			if (iter != initialGlobalTransforms.end())
			{
				localTransform = iter->second;				
			}
			else
				localTransform = XMMatrixIdentity();
		}

		if (boneHierarchy.contains(boneName))
		{
			const std::string& parent = boneHierarchy.at(boneName);			
			boneGlobalTransforms[boneName] = XMMatrixMultiply(localTransform, boneGlobalTransforms[parent]);
		}
		else
		{
			// 부모가 없는 루트노드 (ex: RigPelvis)
			boneGlobalTransforms[boneName] = localTransform;
		}
		if (boneName == "RigPelvis")
			globalinverseTransform = XMMatrixInverse(nullptr, localTransform);
	}
	// (5) 최종 BoneMatrices 완성
	result.resize(boneNameToIndex.size(), XMMatrixIdentity());

	for (const auto& [boneName, vertexIndex] : boneNameToIndex)
	{
		if (boneGlobalTransforms.contains(boneName))
		{         
			result[vertexIndex] = boneOffsets[vertexIndex] * boneGlobalTransforms[boneName] * globalinverseTransform;//확정            
		}		
	}

	return result;
}