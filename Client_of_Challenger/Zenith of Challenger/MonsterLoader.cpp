#include "MonsterLoader.h"
#include "Monster.h"

void LoadAllMonsters(const ComPtr<ID3D12Device>& device,
    const unordered_map<string, shared_ptr<Texture>>& textures,
    const unordered_map<string, shared_ptr<Shader>> shaders,
    vector<shared_ptr<Monsters>>& outMonsters)
{
    // 1. Frightfly ���� �ε�
    auto frightflyLoader = make_shared<FBXLoader>();
    if (frightflyLoader->LoadFBXModel("Model/Monsters/Frightfly/Frightfly_01.fbx", XMMatrixScaling(0.05f, 0.05f, 0.05f)))
    {
        for (int i = 0; i < 10; ++i) // ���� ���� �� �غ�
        {

            auto meshes = frightflyLoader->GetMeshes();
            if (!meshes.empty())
            {
                auto frightfly = make_shared<Frightfly>(device);
                //frightfly->SetPosition(XMFLOAT3{ -185.f, 48.f, 107.f });

                frightfly->SetTexture(textures.at("FrightFly"));
                frightfly->SetTextureIndex(textures.at("FrightFly")->GetTextureIndex());
                frightfly->SetShader(shaders.at("FrightFly")); // ������ ���� �ʿ�
                frightfly->SetDebugLineShader(shaders.at("DebugLineShader"));

                for (auto& mesh : meshes)
                    frightfly->AddMesh(mesh);

                frightfly->SetAnimationClips(frightflyLoader->GetAnimationClips());
                frightfly->SetCurrentAnimation("Idle");
                frightfly->SetBoneOffsets(frightflyLoader->GetBoneOffsets());
                frightfly->SetBoneNameToIndex(frightflyLoader->GetBoneNameToIndex());

                // frightfly ���� ���� ��ġ
                BoundingBox frightflyBox;
                frightflyBox.Center = XMFLOAT3{ 0.f, 5.5f, 0.f };
                frightflyBox.Extents = XMFLOAT3{ 1.5f, 1.5f, 1.5f }; // �����ϸ��� ��
                frightfly->SetBoundingBox(frightflyBox);

                auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
                frightfly->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

                outMonsters.push_back(frightfly);
                OutputDebugStringA("[MonsterLoader] Frightfly �ε� �Ϸ�\n");
            }
            else
            {
                OutputDebugStringA("[MonsterLoader] Frightfly �޽� ����!\n");
            }
        }
    }
}
