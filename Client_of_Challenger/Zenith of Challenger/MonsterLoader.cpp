#include "MonsterLoader.h"
#include "Monster.h"

void LoadAllMonsters(const ComPtr<ID3D12Device>& device, const unordered_map<string, shared_ptr<Texture>>& textures, vector<shared_ptr<Monsters>>& outMonsters)
{
    // 1. Frightfly 몬스터 로드
    auto frightflyLoader = make_shared<FBXLoader>();
    if (frightflyLoader->LoadFBXModel("Model/Monsters/Frightfly/Frightfly_01.fbx", XMMatrixScaling(0.05f, 0.05f, 0.05f)))
    {
        auto meshes = frightflyLoader->GetMeshes();
        if (!meshes.empty())
        {
            auto frightfly = make_shared<Frightfly>(device);
            frightfly->SetPosition(XMFLOAT3{ -185.f, 53.f, 177.f });

            frightfly->SetTexture(textures.at("CHARACTER"));
            frightfly->SetTextureIndex(textures.at("CHARACTER")->GetTextureIndex());

            for (auto& mesh : meshes)
                frightfly->AddMesh(mesh);

            frightfly->SetAnimationClips(frightflyLoader->GetAnimationClips());
            frightfly->SetCurrentAnimation("Idle");
            frightfly->SetBoneOffsets(frightflyLoader->GetBoneOffsets());
            frightfly->SetBoneNameToIndex(frightflyLoader->GetBoneNameToIndex());

            auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
            frightfly->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

            outMonsters.push_back(frightfly);
            OutputDebugStringA("[MonsterLoader] Frightfly 로드 완료\n");
        }
        else
        {
            OutputDebugStringA("[MonsterLoader] Frightfly 메시 없음!\n");
        }
    }
    else
    {
        OutputDebugStringA("[MonsterLoader] Frightfly FBX 로드 실패!\n");
    }

}
