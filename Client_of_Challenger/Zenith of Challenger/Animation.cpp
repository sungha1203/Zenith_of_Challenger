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
	// 보간 전 dot 확인
	float dot = XMVectorGetX(XMVector4Dot(rotA, rotB));
	if (dot < 0.0f) {
		rotB = XMVectorNegate(rotB);
		dot = -dot;
	}
		char dbg[256];
		XMFLOAT4 ra, rb;
		XMStoreFloat4(&ra, rotA);
		XMStoreFloat4(&rb, rotB);
		sprintf_s(dbg, "dot=%.4f | A(%.3f,%.3f,%.3f,%.3f) → B(%.3f,%.3f,%.3f,%.3f)\n",
			dot, ra.x, ra.y, ra.z, ra.w, rb.x, rb.y, rb.z, rb.w);
		OutputDebugStringA(dbg);
	//if (dot < -0.98f) // 유사 180도 회전
	//{
	//}
	//if (XMVectorGetX(XMVector4Dot(rotA, rotB)) < 0.0f)
	//{
	//	// rotB의 부호 반전 (같은 회전을 더 짧은 경로로)
	//	rotB = XMVectorNegate(rotB);
	//
	//}
	XMVECTOR rot;
	if (dot < -0.9999f)
	{
		// 완벽한 반대 방향이므로 SLERP 불가 → arbitrary한 직선 보간 or 고정
		// 보통은 LERP 후 normalize, 또는 rotA 고정
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
	// 보간 전 dot 확인 및 보정
	//if (dot < 0.0f)
	//{
	//	rotB = XMVectorNegate(rotB);
	//	dot = -dot;
	//}
	//
	//// dot == 1.0 → same rotation, dot == 0 → orthogonal, dot == -1.0 → opposite
	//if (dot > 0.9995f)
	//{
	//	// 거의 같은 회전이므로 LERP로 보간 후 normalize
	//	rot = XMVectorLerp(rotA, rotB, t);
	//	rot = XMQuaternionNormalize(rot);
	//}
	//else if (dot < 0.0001f)
	//{
	//	// 거의 반대 방향 → LERP fallback
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
		rot = rotA; // 방어: slerp가 터졌으면 이전 회전 유지
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
//		sprintf_s(dbg, "[경고] %.4f 초에서 '%s'의 keyframe 시간 차이 매우 작음 → a=%.4f, b=%.4f\n",
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
//	// 혹시 잘못된 회전값이 처음부터 들어오는 건 아닌지?
//	if (XMQuaternionIsNaN(rotA) || XMQuaternionIsNaN(rotB))
//	{
//		OutputDebugStringA("[오류] NaN Quaternion 발견\n");
//	}
//	// 보간 전 dot 확인
//	float dot = XMVectorGetX(XMVector4Dot(rotA, rotB));
//	if (fabs(dot) < 0.1f)
//	{
//		char debug[256];
//		sprintf_s(debug, "[Intpolate] TUM? bone=%s t=%.3f dot=%.4f\n", currentBoneName.c_str(), t, dot);
//		OutputDebugStringA(debug);
//	}
//	//XMVECTOR rot;
//	// dot이 -1이면 완전히 반대 방향 → 절대 보간하지 말고 rotA로 고정
//	//if (dot < -0.9999f) {
//	//	char debug[128];
//	//	// SLERP 위험 구간 - Debug 출력
//	//	XMFLOAT4 tempRotA, tempRotB;
//	//	XMStoreFloat4(&tempRotA, rotA);
//	//	XMStoreFloat4(&tempRotB, rotB);
//	//	sprintf_s(debug, "[보간 Skip] dot=%.4f A=(%.3f,%.3f,%.3f,%.3f) B=(%.3f,%.3f,%.3f,%.3f)\n", dot,
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
//	//	sprintf_s(dbg, "[dot < 0.1f → LERP 대체] time=%.4f | bone=%s | dot=%.4f\n", currentTime, currentBoneName.c_str(), dot);
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
//	//		"[SLERP 점검] time=%.4f | bone=%-25s | t=%.4f | dot=%.4f\n",
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
		sprintf_s(dbg, "[경고] %.4f 초에서 '%s'의 keyframe 시간 차이 매우 작음 → a=%.4f, b=%.4f\n",
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

	// NaN 체크
	if (XMQuaternionIsNaN(rotA) || XMQuaternionIsNaN(rotB)) {
		OutputDebugStringA("[오류] NaN Quaternion 발견\n");
	}

	// Dot product 계산 (보간 방식 결정용)
	float dot = XMVectorGetX(XMVector4Dot(rotA, rotB));
	if (fabs(dot) < 0.05f) {
		char debug[256];
		sprintf_s(debug, "[Interpolate] TUM? bone=%s t=%.3f dot=%.4f\n", currentBoneName.c_str(), currentTime, dot);
		OutputDebugStringA(debug);
	}

	// 보간
	XMVECTOR pos = XMVectorLerp(posA, posB, t);
	XMVECTOR scale = XMVectorLerp(scaleA, scaleB, t);

	XMVECTOR rot;
	if (fabs(dot) < 0.1f) {
		rot = XMQuaternionNormalize(XMVectorLerp(rotA, rotB, t)); // LERP fallback
	}
	else {
		rot = XMQuaternionSlerp(rotA, rotB, t); // 안정된 SLERP
	}

	// 조합
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
		// 만약 time이 마지막 키 이후라면, 마지막 키 그대로 적용
		return InterpolateKeyframes(keys.back(), keys.back(), 0, boneAnim.boneName, time);
	}

	float duration = b.time - a.time;
	float t = 0.0f;
	if (duration < FLT_EPSILON)
	{
		// 보간 구간 길이가 너무 짧으면 고정
		char dbg[256];
		sprintf_s(dbg, "[경고] %.4f 초에서 '%s'의 keyframe 시간 차이 매우 작음 → a=%.4f, b=%.4f\n",
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
//	// 루프 애니메이션을 위한 시간 보정
//	if (time >= clipEndTime)
//	{
//		//time = fmod(time, clipEndTime);
//		const Keyframe& last = keys.back();
//		return InterpolateKeyframes(last, last, 0, boneAnim.boneName, time);
//	}
//
//	// 구간 탐색
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
//			//sprintf_s(dbg, "[보간 탐지] bone='%s' time=%.4f, a=%.4f, b=%.4f, b-a=%.6f\n",
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
//	// 기본은 마지막 두 키프레임을 a, b로 해두자 (fallback)
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
//		lerpFactor = 0.0f; // 방어: 동일한 키프레임 시간일 경우
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
		if (boneName == "RigPelvis"|| boneName == "mixamorig:Hips")
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