#include "MonsterLoader.h"
#include "Monster.h"

void LoadAllMonsters(
    const ComPtr<ID3D12Device>& device,
    const unordered_map<string, shared_ptr<Texture>>& textures,
    const unordered_map<string, shared_ptr<Shader>>& shaders,
    const unordered_map<string, shared_ptr<MeshBase>>& meshLibrary,
    const unordered_map<string, AnimationClip>& animClipLibrary,
    const unordered_map<string, XMMATRIX>& boneOffsetLibrary,
    const unordered_map<string, int>& boneMap,
    unordered_map<string, vector<shared_ptr<Monsters>>>& outMonsterGroups,
    const shared_ptr<Camera>& camera)
{
    // === 1. Frightfly ���� ���� ===
    if (!meshLibrary.contains("Frightfly")) {
        OutputDebugStringA("[MonsterLoader] Frightfly �޽� ����\n");
        return;
    }

    for (int i = 0; i < 10; ++i)
    {
        auto frightfly = make_shared<Frightfly>(device);

        // [1] �޽�, �ؽ�ó, ���̴� ����
        frightfly->SetCamera(camera);
        frightfly->SetMesh(meshLibrary.at("Frightfly"));
        frightfly->SetTexture(textures.at("FrightFly"));
        frightfly->SetTextureIndex(textures.at("FrightFly")->GetTextureIndex());
        frightfly->SetShader(shaders.at("FrightFly"));
        frightfly->SetDebugLineShader(shaders.at("DebugLineShader"));
        frightfly->SetHealthBarShader(shaders.at("HealthBarShader"));

        // [2] �ִϸ��̼� ����
        vector<AnimationClip> clips;
        for (const auto& [name, clip] : animClipLibrary)
            clips.push_back(clip);

        frightfly->SetAnimationClips(clips);
        frightfly->SetCurrentAnimation("Idle");
        frightfly->SetBoneOffsets(boneOffsetLibrary);
        frightfly->SetBoneNameToIndex(boneMap);

        // [3] �ٿ�� �ڽ�
        BoundingBox box;
        box.Center = XMFLOAT3{ 0.f, 5.5f, 0.f };
        box.Extents = XMFLOAT3{ 1.5f, 1.5f, 1.5f };
        frightfly->SetBoundingBox(box);

        // [4] �� ���� SRV ����
        auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
        frightfly->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

        // [5] �׷쿡 �߰�
        outMonsterGroups["Frightfly"].push_back(frightfly);
    }

    OutputDebugStringA("[MonsterLoader] Frightfly ���� 10���� �ε� �Ϸ�\n");

      //Flower Fairy
    for (int i = 0; i < 10; ++i)
    {
        auto monster = make_shared<FlowerFairy>(device);
        monster->SetCamera(camera);
        monster->SetTexture(textures.at("Flower_Fairy"));
        monster->SetTextureIndex(textures.at("Flower_Fairy")->GetTextureIndex());
        monster->SetShader(shaders.at("FrightFly")); // ���̴� ����
        monster->SetDebugLineShader(shaders.at("DebugLineShader"));
        monster->SetHealthBarShader(shaders.at("HealthBarShader"));

        monster->SetMesh(meshLibrary.at("Flower_Fairy"));
        monster->SetAnimationClips({ animClipLibrary.at("Idle") });
        monster->SetCurrentAnimation("Idle");
        monster->SetBoneOffsets(boneOffsetLibrary);
        monster->SetBoneNameToIndex(boneMap);

        BoundingBox box;
        box.Center = XMFLOAT3{ 0.f, 10.0f, 0.f };
        box.Extents = XMFLOAT3{ 1.5f, 3.5f, 1.5f };
        monster->SetBoundingBox(box);

        auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
        monster->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

        outMonsterGroups["Flower_Fairy"].push_back(monster);
    }

    // [3] Mushroom Dark
    for (int i = 0; i < 10; ++i)
    {
        auto monster = make_shared<MushroomDark>(device);
        monster->SetCamera(camera);
        monster->SetTexture(textures.at("Mushroom_Dark"));
        monster->SetTextureIndex(textures.at("Mushroom_Dark")->GetTextureIndex());
        monster->SetShader(shaders.at("FrightFly")); // ���̴� ����
        monster->SetDebugLineShader(shaders.at("DebugLineShader"));
        monster->SetHealthBarShader(shaders.at("HealthBarShader"));

        monster->SetMesh(meshLibrary.at("Mushroom_Dark"));
        monster->SetAnimationClips({ animClipLibrary.at("Idle") });
        monster->SetCurrentAnimation("Idle");
        monster->SetBoneOffsets(boneOffsetLibrary);
        monster->SetBoneNameToIndex(boneMap);

        BoundingBox box;
        box.Center = XMFLOAT3{ 0.f, 5.0f, 0.f };
        box.Extents = XMFLOAT3{ 3.5f, 6.0f, 3.5f };
        monster->SetBoundingBox(box);

        auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
        monster->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

        outMonsterGroups["Mushroom_Dark"].push_back(monster);
    }

    // [4] Venus_Blue
    for (int i = 0; i < 10; ++i)
    {
        auto monster = make_shared<VenusBlue>(device); // VenusBlue Ŭ���� �ʿ�
        monster->SetCamera(camera);
        monster->SetTexture(textures.at("Venus_Blue"));
        monster->SetTextureIndex(textures.at("Venus_Blue")->GetTextureIndex());
        monster->SetShader(shaders.at("FrightFly")); // ���� ���̴�
        monster->SetDebugLineShader(shaders.at("DebugLineShader"));
        monster->SetHealthBarShader(shaders.at("HealthBarShader"));

        monster->SetMesh(meshLibrary.at("Venus_Blue"));
        monster->SetAnimationClips({ animClipLibrary.at("Idle") });
        monster->SetCurrentAnimation("Idle");
        monster->SetBoneOffsets(boneOffsetLibrary);
        monster->SetBoneNameToIndex(boneMap);

        BoundingBox box;
        box.Center = XMFLOAT3{ 0.f, 5.5f, 0.f };
        box.Extents = XMFLOAT3{ 4.0f, 5.0f, 4.0f };
        monster->SetBoundingBox(box);

        auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
        monster->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

        outMonsterGroups["Venus_Blue"].push_back(monster);

        OutputDebugStringA("[MonsterLoader] Venus_Blue �ε� �Ϸ�\n");
    }


    // [5] Plant_Dionaea
    for (int i = 0; i < 10; ++i)
    {
        auto monster = make_shared<PlantDionaea>(device); // PlantDionaea Ŭ���� �ʿ�
        monster->SetCamera(camera);
        monster->SetTexture(textures.at("Plant_Dionaea"));
        monster->SetTextureIndex(textures.at("Plant_Dionaea")->GetTextureIndex());
        monster->SetShader(shaders.at("FrightFly")); // ���� ���̴� ���
        monster->SetDebugLineShader(shaders.at("DebugLineShader"));
        monster->SetHealthBarShader(shaders.at("HealthBarShader"));

        monster->SetMesh(meshLibrary.at("Plant_Dionaea"));
        monster->SetAnimationClips({ animClipLibrary.at("Idle") });
        monster->SetBoneOffsets(boneOffsetLibrary);
        monster->SetBoneNameToIndex(boneMap);
        monster->SetCurrentAnimation("Idle");

        BoundingBox box;
        box.Center = XMFLOAT3{ 0.f, 4.0f, 0.f };
        box.Extents = XMFLOAT3{ 4.0f, 5.5f, 4.0f }; // ������ ���� �ʿ�
        monster->SetBoundingBox(box);

        auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
        monster->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

        outMonsterGroups["Plant_Dionaea"].push_back(monster);

        OutputDebugStringA("[MonsterLoader] Plant_Dionaea �ε� �Ϸ�\n");
    }


}

