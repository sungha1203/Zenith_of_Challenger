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
	// ���� �� dot Ȯ��
	float dot = XMVectorGetX(XMVector4Dot(rotA, rotB));
	if (dot < 0.0f) {
		rotB = XMVectorNegate(rotB);
		dot = -dot;
	}
		char dbg[256];
		XMFLOAT4 ra, rb;
		XMStoreFloat4(&ra, rotA);
		XMStoreFloat4(&rb, rotB);
		sprintf_s(dbg, "dot=%.4f | A(%.3f,%.3f,%.3f,%.3f) �� B(%.3f,%.3f,%.3f,%.3f)\n",
			dot, ra.x, ra.y, ra.z, ra.w, rb.x, rb.y, rb.z, rb.w);
		OutputDebugStringA(dbg);
	//if (dot < -0.98f) // ���� 180�� ȸ��
	//{
	//}
	//if (XMVectorGetX(XMVector4Dot(rotA, rotB)) < 0.0f)
	//{
	//	// rotB�� ��ȣ ���� (���� ȸ���� �� ª�� ��η�)
	//	rotB = XMVectorNegate(rotB);
	//
	//}
	XMVECTOR rot;
	if (dot < -0.9999f)
	{
		// �Ϻ��� �ݴ� �����̹Ƿ� SLERP �Ұ� �� arbitrary�� ���� ���� or ����
		// ������ LERP �� normalize, �Ǵ� rotA ����
		/*rot = XMVectorLerp(rotA, rotB, t);
		rot = XMQuaternionNormalize(rot);*/
		rot = rotA;
	}
	else if (dot > 0.9995f)
	{
		rot = XMVectorLerp(rotA, rotB, t);
		rot = XMQuaternionNormalize(rot);
	}
	else
	{
		rot = XMQuaternionSlerp(rotA, rotB, t);
	}
	// ���� �� dot Ȯ�� �� ����
	//if (dot < 0.0f)
	//{
	//	rotB = XMVectorNegate(rotB);
	//	dot = -dot;
	//}
	//
	//// dot == 1.0 �� same rotation, dot == 0 �� orthogonal, dot == -1.0 �� opposite
	//if (dot > 0.9995f)
	//{
	//	// ���� ���� ȸ���̹Ƿ� LERP�� ���� �� normalize
	//	rot = XMVectorLerp(rotA, rotB, t);
	//	rot = XMQuaternionNormalize(rot);
	//}
	//else if (dot < 0.0001f)
	//{
	//	// ���� �ݴ� ���� �� LERP fallback
	//	rot = XMVectorLerp(rotA, rotB, t);
	//	rot = XMQuaternionNormalize(rot);
	//}
	//else
	//{
	//	rot = XMQuaternionSlerp(rotA, rotB, t);
	//}
	if (
		isnan(XMVectorGetX(rot)) || isnan(XMVectorGetY(rot)) ||
		isnan(XMVectorGetZ(rot)) || isnan(XMVectorGetW(rot)) ||
		XMVectorGetX(XMVector4LengthSq(rot)) < 0.0001f
		)
	{
		rot = rotA; // ���: slerp�� �������� ���� ȸ�� ����
	}
	XMVECTOR pos = XMVectorLerp(posA, posB, t);
	XMVECTOR scale = XMVectorLerp(scaleA, scaleB, t);
	//XMVECTOR rot = XMQuaternionSlerp(rotA, rotB, t);

	XMMATRIX T = XMMatrixTranslationFromVector(pos);
	XMMATRIX R = XMMatrixRotationQuaternion(rot);
	XMMATRIX S = XMMatrixScalingFromVector(scale);

	return S * R * T;

}

//XMMATRIX InterpolateKeyframes(const Keyframe& a, const Keyframe& b, float t, const std::string& currentBoneName, float currentTime)
//{
//	float timeGap = b.time - a.time;
//	if (fabsf(timeGap) < 0.0001f) {
//		char dbg[256];
//		sprintf_s(dbg, "[���] %.4f �ʿ��� '%s'�� keyframe �ð� ���� �ſ� ���� �� a=%.4f, b=%.4f\n",
//			currentTime, currentBoneName.c_str(), a.time, b.time);
//		OutputDebugStringA(dbg);
//	}
//	XMVECTOR posA = XMLoadFloat3(&a.position);
//	XMVECTOR posB = XMLoadFloat3(&b.position);
//	XMVECTOR scaleA = XMLoadFloat3(&a.scale);
//	XMVECTOR scaleB = XMLoadFloat3(&b.scale);
//
//	XMVECTOR rotA = XMQuaternionNormalize(XMLoadFloat4(&a.rotation));
//	XMVECTOR rotB = XMQuaternionNormalize(XMLoadFloat4(&b.rotation));
//	// Ȥ�� �߸��� ȸ������ ó������ ������ �� �ƴ���?
//	if (XMQuaternionIsNaN(rotA) || XMQuaternionIsNaN(rotB))
//	{
//		OutputDebugStringA("[����] NaN Quaternion �߰�\n");
//	}
//	// ���� �� dot Ȯ��
//	float dot = XMVectorGetX(XMVector4Dot(rotA, rotB));
//	if (fabs(dot) < 0.1f)
//	{
//		char debug[256];
//		sprintf_s(debug, "[Intpolate] TUM? bone=%s t=%.3f dot=%.4f\n", currentBoneName.c_str(), t, dot);
//		OutputDebugStringA(debug);
//	}
//	//XMVECTOR rot;
//	// dot�� -1�̸� ������ �ݴ� ���� �� ���� �������� ���� rotA�� ����
//	//if (dot < -0.9999f) {
//	//	char debug[128];
//	//	// SLERP ���� ���� - Debug ���
//	//	XMFLOAT4 tempRotA, tempRotB;
//	//	XMStoreFloat4(&tempRotA, rotA);
//	//	XMStoreFloat4(&tempRotB, rotB);
//	//	sprintf_s(debug, "[���� Skip] dot=%.4f A=(%.3f,%.3f,%.3f,%.3f) B=(%.3f,%.3f,%.3f,%.3f)\n", dot,
//	//		tempRotA.x, tempRotA.y, tempRotA.z, tempRotA.w,
//	//		tempRotB.x, tempRotB.y, tempRotB.z, tempRotB.w);
//	//	OutputDebugStringA(debug);
//	//	rot = rotA;
//	//}
//	//else if (dot > 0.9995f) {
//	//	rot = XMQuaternionNormalize(XMVectorLerp(rotA, rotB, t));
//	//}
//	//else if (dot < 0.1f) {
//	//	char dbg[128];
//	//	sprintf_s(dbg, "[dot < 0.1f �� LERP ��ü] time=%.4f | bone=%s | dot=%.4f\n", currentTime, currentBoneName.c_str(), dot);
//	//	OutputDebugStringA(dbg);
//	//	rot = XMQuaternionNormalize(XMVectorLerp(rotA, rotB, t)); 
//	//}
//	//else {
//	//	// SLERP with sign correction
//	//	//if (dot < 0.0f)
//	//	//{
//	//	//	rotB = XMVectorNegate(rotB);
//	//	//	dot = -dot;
//	//	//}
//	//	rot = XMQuaternionSlerp(rotA, rotB, t);
//	//}
//	//if (
//	//	isnan(XMVectorGetX(rot)) || isnan(XMVectorGetY(rot)) ||
//	//	isnan(XMVectorGetZ(rot)) || isnan(XMVectorGetW(rot)) ||
//	//	XMVectorGetX(XMVector4LengthSq(rot)) < 0.0001f
//	//	) {
//	//	rot = rotA; // fallback
//	//}
//	//XMFLOAT4 dbgRotA, dbgRotB;
//	//XMStoreFloat4(&dbgRotA, rotA);
//	//XMStoreFloat4(&dbgRotB, rotB);
//	//
//	//if (fabs(dot) < 0.1f || currentBoneName == "RigMouth" || currentBoneName == "RigRHeadDownJaw")
//	//{
//	//	char debug[256];
//	//	sprintf_s(debug,
//	//		"[SLERP ����] time=%.4f | bone=%-25s | t=%.4f | dot=%.4f\n",
//	//		currentTime, currentBoneName.c_str(), t, dot);
//	//	OutputDebugStringA(debug);
//	//}
//	XMVECTOR pos = XMVectorLerp(posA, posB, t);
//	XMVECTOR scale = XMVectorLerp(scaleA, scaleB, t);
//	XMVECTOR rot = XMQuaternionSlerp(rotA, rotB, t);
//
//	XMMATRIX T = XMMatrixTranslationFromVector(pos);
//	XMMATRIX R = XMMatrixRotationQuaternion(rot);
//	XMMATRIX S = XMMatrixScalingFromVector(scale);
//
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
	if (fabs(dot) < 0.05f) {
		char debug[256];
		sprintf_s(debug, "[Interpolate] TUM? bone=%s t=%.3f dot=%.4f\n", currentBoneName.c_str(), currentTime, dot);
		OutputDebugStringA(debug);
	}

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
	if (time >= clipEndTime)
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
//XMMATRIX AnimationClip::GetBoneTransform(const BoneAnimation& boneAnim, float time) const
//{
//	const auto& keys = boneAnim.keyframes;
//	if (keys.empty()) return XMMatrixIdentity();
//	if (keys.size() == 1) return InterpolateKeyframes(keys[0], keys[0], 0, boneAnim.boneName, time);
//
//	float clipEndTime = keys.back().time;
//
//	// ���� �ִϸ��̼��� ���� �ð� ����
//	if (time >= clipEndTime)
//	{
//		//time = fmod(time, clipEndTime);
//		const Keyframe& last = keys.back();
//		return InterpolateKeyframes(last, last, 0, boneAnim.boneName, time);
//	}
//
//	// ���� Ž��
//	Keyframe a = keys[0];
//	Keyframe b = keys[0];
//
//	for (size_t i = 0; i < keys.size() - 1; ++i)
//	{
//		if (time < keys[i + 1].time)
//		{
//			a = keys[i];
//			b = keys[i + 1];
//			char dbg[256];
//			//sprintf_s(dbg, "[���� Ž��] bone='%s' time=%.4f, a=%.4f, b=%.4f, b-a=%.6f\n",
//			//	boneAnim.boneName.c_str(), time, a.time, b.time, b.time - a.time);
//			//OutputDebugStringA(dbg);
//			break;
//		}
//	}
//
//	float duration = b.time - a.time;
//	float lerpFactor = 0.0f;
//
//	if (duration > FLT_EPSILON)
//		lerpFactor = (time - a.time) / duration;
//
//	return InterpolateKeyframes(a, b, lerpFactor, boneAnim.boneName, time);
//}
//XMMATRIX AnimationClip::GetBoneTransform(const BoneAnimation& boneAnim, float time) const
//{
//	const auto& keys = boneAnim.keyframes;
//	if (keys.empty()) return XMMatrixIdentity();
//	if (keys.size() == 1) return InterpolateKeyframes(keys[0], keys[0], 0);
//
//	Keyframe a = keys[0];
//	Keyframe b = keys[0];
//	
//	// �⺻�� ������ �� Ű�������� a, b�� �ص��� (fallback)
//	a = keys[keys.size() - 2];
//	b = keys[keys.size() - 1];
//
//	for (size_t i = 0; i < keys.size() - 1; ++i)
//	{
//		if (time < keys[i + 1].time)
//		{
//			a = keys[i];
//			b = keys[i + 1];
//			break;
//		}
//	}
//
//	//float lerpFactor = (time - a.time) / (b.time - a.time);
//	float duration = b.time - a.time;
//	float lerpFactor = 0.0f;
//
//	if (duration > FLT_EPSILON)
//		lerpFactor = (time - a.time) / duration;
//	else
//		lerpFactor = 0.0f; // ���: ������ Ű������ �ð��� ���
//
//	//{
//	//	char dbg[128];
//	//	sprintf_s(dbg, "[%s] time: %.3f | a: %.3f, b: %.3f, lerp: %.3f\n", boneAnim.boneName.c_str(), time, a.time, b.time, lerpFactor);
//	//	OutputDebugStringA(dbg);
//	//}
//
//	XMMATRIX localTransform = InterpolateKeyframes(a, b, lerpFactor,boneAnim.boneName,time);
//	return localTransform;
//}

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
	}

	// ���� BoneMatrices �����
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
	unordered_map<int, XMMATRIX> boneOffsets,
	const std::unordered_map<std::string, XMMATRIX>& initialGlobalTransforms) const
{
	std::vector<XMMATRIX> result;

	// (2) bone �̸� ���� (topological sort)
	std::vector<std::string> sortedBones = TopologicalSort(boneHierarchy);
	// (3) �̸� �� �۷ι� ��ȯ ��Ʈ����
	std::unordered_map<std::string, XMMATRIX> boneGlobalTransforms;

	XMMATRIX globalinverseTransform = XMMatrixIdentity();


	// (4) ���� �۷ι� ��ȯ ���
	for (const auto& boneName : sortedBones)
	{
		XMMATRIX localTransform = XMMatrixIdentity();
		if (boneAnimations.contains(boneName))
		{
			localTransform = GetBoneTransform(boneAnimations.at(boneName), time);
		}
		else
		{
			// �ִϸ��̼� ������ ���� �۷ι� Ʈ������ ��������
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
			// �θ� ���� ��Ʈ��� (ex: RigPelvis)
			boneGlobalTransforms[boneName] = localTransform;
		}
		if (boneName == "RigPelvis"|| boneName == "mixamorig:Hips")
			globalinverseTransform = XMMatrixInverse(nullptr, localTransform);
	}
	// (5) ���� BoneMatrices �ϼ�
	result.resize(boneNameToIndex.size(), XMMatrixIdentity());

	for (const auto& [boneName, vertexIndex] : boneNameToIndex)
	{
		if (boneGlobalTransforms.contains(boneName))
		{         
			result[vertexIndex] = boneOffsets[vertexIndex] * boneGlobalTransforms[boneName] * globalinverseTransform;//Ȯ��            
		}	

	}

	return result;
}