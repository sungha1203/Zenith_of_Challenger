// animation.cpp
#include "animation.h"
#include <DirectXMath.h>
#include <queue>
#include<unordered_set>
#include<functional>

using namespace DirectX;

//XMFLOAT3 QuaternionToYawPitchRoll(const XMVECTOR& q)
//{
//	XMFLOAT4 quat;
//	XMStoreFloat4(&quat, q);
//
//	float ysqr = quat.y * quat.y;
//
//	// roll (x-axis rotation)
//	float t0 = +2.0f * (quat.w * quat.x + quat.y * quat.z);
//	float t1 = +1.0f - 2.0f * (quat.x * quat.x + ysqr);
//	float roll = std::atan2(t0, t1);
//
//	// pitch (y-axis rotation)
//	float t2 = +2.0f * (quat.w * quat.y - quat.z * quat.x);
//	t2 = t2 > 1.0f ? 1.0f : t2;
//	t2 = t2 < -1.0f ? -1.0f : t2;
//	float pitch = std::asin(t2);
//
//	// yaw (z-axis rotation)
//	float t3 = +2.0f * (quat.w * quat.z + quat.x * quat.y);
//	float t4 = +1.0f - 2.0f * (ysqr + quat.z * quat.z);
//	float yaw = std::atan2(t3, t4);
//
//	return XMFLOAT3(pitch, yaw, roll); // Y, Z, X ȸ�� ��
//}
//
//XMMATRIX InterpolateKeyframes(const Keyframe& a, const Keyframe& b, float t, const std::string& currentBoneName, float currentTime)
//{
//	XMVECTOR posA = XMLoadFloat3(&a.position);
//	XMVECTOR posB = XMLoadFloat3(&b.position);
//	XMVECTOR scaleA = XMLoadFloat3(&a.scale);
//	XMVECTOR scaleB = XMLoadFloat3(&b.scale);
//	XMVECTOR rotA = XMQuaternionNormalize(XMLoadFloat4(&a.rotation));
//	XMVECTOR rotB = XMQuaternionNormalize(XMLoadFloat4(&b.rotation));
//
//	XMFLOAT3 eulerA = QuaternionToYawPitchRoll(rotA);
//	XMFLOAT3 eulerB = QuaternionToYawPitchRoll(rotB);
//
//	// ���� �� �� ����
//	float pitchA = XMConvertToDegrees(eulerA.x);
//	float yawA = XMConvertToDegrees(eulerA.y);
//	float rollA = XMConvertToDegrees(eulerA.z);
//
//	float pitchB = XMConvertToDegrees(eulerB.x);
//	float yawB = XMConvertToDegrees(eulerB.y);
//	float rollB = XMConvertToDegrees(eulerB.z);
//
//	// ���� ���� (��360 ����)
//	auto FixAngle = [](float a, float b) -> float {
//		float delta = b - a;
//		if (delta > 180.0f)  b -= 360.0f;
//		else if (delta < -180.0f) b += 360.0f;
//		return b;
//		};
//
//	pitchB = FixAngle(pitchA, pitchB);
//	yawB = FixAngle(yawA, yawB);
//	rollB = FixAngle(rollA, rollB);
//
//	//����
//	float finalPitch = pitchA + (pitchB - pitchA) * t;
//	float finalYaw = yawA + (yawB - yawA) * t;
//	float finalRoll = rollA + (rollB - rollA) * t;
//
//	// �ٽ� Quaternion����
//	XMVECTOR rot = XMQuaternionRotationRollPitchYaw(
//		XMConvertToRadians(finalPitch),
//		XMConvertToRadians(finalYaw),
//		XMConvertToRadians(finalRoll));
//
//	// ��ġ, ������ ����
//	XMVECTOR pos = XMVectorLerp(posA, posB, t);
//	XMVECTOR scale = XMVectorLerp(scaleA, scaleB, t);
//
//	// ���� ��ȯ ����
//	XMMATRIX T = XMMatrixTranslationFromVector(pos);
//	XMMATRIX R = XMMatrixRotationQuaternion(rot);
//	XMMATRIX S = XMMatrixScalingFromVector(scale);
//	return S * R * T;
//}
XMMATRIX InterpolateKeyframes(const Keyframe& a, const Keyframe& b, float t, const std::string& currentBoneName, float currentTime)
{
	float timeGap = b.time - a.time;
	if (fabsf(timeGap) < 0.0001f) {
		char dbg[256];
		sprintf_s(dbg, "[���] %.4f �ʿ��� '%s'�� keyframe �ð� ���� �ſ� ���� �� a=%.4f, b=%.4f\n",
			currentTime, currentBoneName.c_str(), a.time, b.time);
		OutputDebugStringA(dbg);
	}

	// Load values
	XMVECTOR posA = XMLoadFloat3(&a.position);
	XMVECTOR posB = XMLoadFloat3(&b.position);
	XMVECTOR scaleA = XMLoadFloat3(&a.scale);
	XMVECTOR scaleB = XMLoadFloat3(&b.scale);
	XMVECTOR rotA = XMQuaternionNormalize(XMLoadFloat4(&a.rotation));
	XMVECTOR rotB = XMQuaternionNormalize(XMLoadFloat4(&b.rotation));

	// NaN üũ
	if (XMQuaternionIsNaN(rotA) || XMQuaternionIsNaN(rotB)) {
		OutputDebugStringA("[����] NaN Quaternion �߰�\n");
	}

	// Dot product ��� (���� ��� ������)
	float dot = XMVectorGetX(XMVector4Dot(rotA, rotB));
	

	// ����
	XMVECTOR pos = XMVectorLerp(posA, posB, t);
	XMVECTOR scale = XMVectorLerp(scaleA, scaleB, t);

	XMVECTOR rot;
	if (fabs(dot) < 0.1f) {
		rot = XMQuaternionNormalize(XMVectorLerp(rotA, rotB, t)); // LERP fallback
	}
	else {
		rot = XMQuaternionSlerp(rotA, rotB, t); // ������ SLERP
	}

	// ����
	XMMATRIX T = XMMatrixTranslationFromVector(pos);
	XMMATRIX R = XMMatrixRotationQuaternion(rot);
	XMMATRIX S = XMMatrixScalingFromVector(scale);

	return S * R * T;
}
XMMATRIX AnimationClip::GetBoneTransform(const BoneAnimation& boneAnim, float time) const
{
	const auto& keys = boneAnim.keyframes;
	if (keys.empty()) return XMMatrixIdentity();
	if (keys.size() == 1) return InterpolateKeyframes(keys[0], keys[0], 0, boneAnim.boneName, time);

	float clipEndTime = keys.back().time;
	if (time >= clipEndTime - 0.001f)
		time = fmod(time, clipEndTime);

	Keyframe a = keys[0], b = keys[0];
	bool found = false;

	for (size_t i = 0; i < keys.size() - 1; ++i)
	{
		if (time < keys[i + 1].time)
		{
			a = keys[i];
			b = keys[i + 1];
			found = true;
			break;
		}
	}

	if (!found)
	{
		// ���� time�� ������ Ű ���Ķ��, ������ Ű �״�� ����
		return InterpolateKeyframes(keys.back(), keys.back(), 0, boneAnim.boneName, time);
	}

	float duration = b.time - a.time;
	float t = 0.0f;
	if (duration < FLT_EPSILON)
	{
		// ���� ���� ���̰� �ʹ� ª���� ����
		char dbg[256];
		sprintf_s(dbg, "[���] %.4f �ʿ��� '%s'�� keyframe �ð� ���� �ſ� ���� �� a=%.4f, b=%.4f\n",
			time, boneAnim.boneName.c_str(), a.time, b.time);
		OutputDebugStringA(dbg);
		return InterpolateKeyframes(a, a, 0.0f, boneAnim.boneName, time);
	}
	else
	{
		t = (time - a.time) / duration;
	}

	return InterpolateKeyframes(a, b, t, boneAnim.boneName, time);
}

// 1. �θ�-�ڽ� ���� ������� Topological Sort
vector<string> TopologicalSort(const unordered_map<string, string>& boneHierarchy)
{
	std::vector<std::string> result;
	std::unordered_map<std::string, bool> visited;

	// ��� ��带 candidates�� ������
	std::unordered_map<std::string, bool> candidates;
	for (const auto& [node, parent] : boneHierarchy)
	{
		candidates[node] = true;
		candidates[parent] = true;
	}

	// DFS �Լ�
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

	// ��� �ĺ� ��忡 ���� DFS
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
	std::unordered_map<int, XMMATRIX> boneOffsets,
	const std::unordered_map<std::string, XMMATRIX>& initialGlobalTransforms) const
{
	std::vector<XMMATRIX> result;

	// (1) �� �̸� ����
	std::vector<std::string> sortedBones = TopologicalSort(boneHierarchy);

	// (2) boneName �� GlobalTransform
	std::unordered_map<std::string, XMMATRIX> boneGlobalTransforms;
	XMMATRIX globalInverseTransform = XMMatrixIdentity();
	bool inverseSet = false;

	// (3) �۷ι� Ʈ������ ���
	for (const auto& boneName : sortedBones)
	{
		XMMATRIX localTransform = XMMatrixIdentity();

		// �ִϸ��̼� ���� Ȯ��
		if (boneAnimations.contains(boneName))
		{
			localTransform = GetBoneTransform(boneAnimations.at(boneName), time);
		}
		else
		{
			auto iter = initialGlobalTransforms.find(boneName);
			if (iter != initialGlobalTransforms.end())
				localTransform = iter->second;
		}

		// �θ� �� ������ ������ ��
		if (boneHierarchy.contains(boneName))
		{
			const std::string& parent = boneHierarchy.at(boneName);
			boneGlobalTransforms[boneName] = XMMatrixMultiply(localTransform, boneGlobalTransforms[parent]);
		}
		else
		{
			boneGlobalTransforms[boneName] = localTransform;
		}

		// inverse ���� �� ����
		if ((boneName == "RigPelvis" || boneName == "mixamorig:Hips") && !inverseSet)
		{
			globalInverseTransform = XMMatrixInverse(nullptr, boneGlobalTransforms[boneName]);
			inverseSet = true;
		}
	}

	// (4) fallback inverse ���� (�� �������� �� ����� ���)
	if (!inverseSet && !sortedBones.empty())
	{
		const std::string& rootBone = sortedBones[0];
		globalInverseTransform = XMMatrixInverse(nullptr, boneGlobalTransforms[rootBone]);
	}

	// (5) ���� �� ��� ���
	result.resize(boneNameToIndex.size(), XMMatrixIdentity());
	for (const auto& [boneName, vertexIndex] : boneNameToIndex)
	{
		if (boneGlobalTransforms.contains(boneName))
		{
			result[vertexIndex] = boneOffsets[vertexIndex] * boneGlobalTransforms[boneName] * globalInverseTransform;
		}
	}

	return result;
}
