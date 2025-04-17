#include "MonsterLoader.h"
#include "Monster.h"

void LoadAllMonsters(
    const ComPtr<ID3D12Device>& device,
    const unordered_map<string, shared_ptr<Texture>>& textures,
    const unordered_map<string, shared_ptr<Shader>> shaders,
    unordered_map<string, vector<shared_ptr<Monsters>>>& outMonsterGroups) //변경
{
    // 1. Frightfly 몬스터 로드
    auto frightflyLoader = make_shared<FBXLoader>();
    if (frightflyLoader->LoadFBXModel("Model/Monsters/Frightfly/Frightfly_01.fbx", XMMatrixScaling(0.05f, 0.05f, 0.05f)))
    {
        for (int i = 0; i < 10; ++i)
        {
            auto meshes = frightflyLoader->GetMeshes();
            if (!meshes.empty())
            {
                auto frightfly = make_shared<Frightfly>(device);

                frightfly->SetTexture(textures.at("FrightFly"));
                frightfly->SetTextureIndex(textures.at("FrightFly")->GetTextureIndex());
                frightfly->SetShader(shaders.at("FrightFly"));
                frightfly->SetDebugLineShader(shaders.at("DebugLineShader"));

                for (auto& mesh : meshes)
                    frightfly->AddMesh(mesh);

                frightfly->SetAnimationClips(frightflyLoader->GetAnimationClips());
                frightfly->SetCurrentAnimation("Idle");
                frightfly->SetBoneOffsets(frightflyLoader->GetBoneOffsets());
                frightfly->SetBoneNameToIndex(frightflyLoader->GetBoneNameToIndex());

                BoundingBox frightflyBox;
                frightflyBox.Center = XMFLOAT3{ 0.f, 5.5f, 0.f };
                frightflyBox.Extents = XMFLOAT3{ 1.5f, 1.5f, 1.5f };
                frightfly->SetBoundingBox(frightflyBox);

                auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
                frightfly->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

                // 변경된 부분 
                outMonsterGroups["Frightfly"].push_back(frightfly);

                OutputDebugStringA("[MonsterLoader] Frightfly 로드 완료\n");
            }
            else
            {
                OutputDebugStringA("[MonsterLoader] Frightfly 메시 없음!\n");
            }
        }
    }




    // 2. Flower_Fairy 몬스터 로드
    auto flowerFairyLoader = make_shared<FBXLoader>();
    if (flowerFairyLoader->LoadFBXModel("Model/Monsters/Flower_Fairy/Flower_Fairy.fbx", XMMatrixScaling(0.1f, 0.1f, 0.1f)))
    {
        for (int i = 0; i < 10; ++i)
        {
            auto meshes = flowerFairyLoader->GetMeshes();
            if (!meshes.empty())
            {
                // 몬스터 객체 생성
                auto flowerFairy = make_shared<FlowerFairy>(device); // 추후 FlowerFairy 클래스로 대체 가능

                flowerFairy->SetTexture(textures.at("Flower_Fairy"));
                flowerFairy->SetTextureIndex(textures.at("Flower_Fairy")->GetTextureIndex());
                flowerFairy->SetShader(shaders.at("FrightFly"));
                flowerFairy->SetDebugLineShader(shaders.at("DebugLineShader"));

                for (auto& mesh : meshes)
                    flowerFairy->AddMesh(mesh);

                flowerFairy->SetAnimationClips(flowerFairyLoader->GetAnimationClips());
                flowerFairy->SetCurrentAnimation("Idle");
                flowerFairy->SetBoneOffsets(flowerFairyLoader->GetBoneOffsets());
                flowerFairy->SetBoneNameToIndex(flowerFairyLoader->GetBoneNameToIndex());

                BoundingBox box;
                box.Center = XMFLOAT3{ 0.f, 10.0f, 0.f };
                box.Extents = XMFLOAT3{ 1.5f, 3.5f, 1.5f };
                flowerFairy->SetBoundingBox(box);

                auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
                flowerFairy->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

                outMonsterGroups["Flower_Fairy"].push_back(flowerFairy);

                OutputDebugStringA("[MonsterLoader] Flower Fairy 로드 완료\n");
            }
            else
            {
                OutputDebugStringA("[MonsterLoader] Flower Fairy 메시 없음!\n");
            }
        }
    }

    // 3. Mushroom_Dark 몬스터 로드
    auto mushroomDarkLoader = make_shared<FBXLoader>();
    if (mushroomDarkLoader->LoadFBXModel("Model/Monsters/Mushroom_Dark/Mushroom_Dark.fbx", XMMatrixScaling(0.1f, 0.1f, 0.1f)))
    {
        for (int i = 0; i < 10; ++i)
        {
            auto meshes = mushroomDarkLoader->GetMeshes();
            if (!meshes.empty())
            {
                // 몬스터 객체 생성
                auto mushroomDark = make_shared<MushroomDark>(device); // 추후 MushroomDark 클래스로 대체 가능

                mushroomDark->SetTexture(textures.at("Mushroom_Dark"));
                mushroomDark->SetTextureIndex(textures.at("Mushroom_Dark")->GetTextureIndex());
                mushroomDark->SetShader(shaders.at("FrightFly")); // 캐릭터 셰이더 재사용
                mushroomDark->SetDebugLineShader(shaders.at("DebugLineShader"));

                for (auto& mesh : meshes)
                    mushroomDark->AddMesh(mesh);

                mushroomDark->SetAnimationClips(mushroomDarkLoader->GetAnimationClips());
                mushroomDark->SetCurrentAnimation("Idle");
                mushroomDark->SetBoneOffsets(mushroomDarkLoader->GetBoneOffsets());
                mushroomDark->SetBoneNameToIndex(mushroomDarkLoader->GetBoneNameToIndex());

                BoundingBox box;
                box.Center = XMFLOAT3{ 0.f, 5.0f, 0.f };
                box.Extents = XMFLOAT3{ 3.5f, 6.0f, 3.5f };
                mushroomDark->SetBoundingBox(box);

                auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
                mushroomDark->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

                outMonsterGroups["Mushroom_Dark"].push_back(mushroomDark);

                OutputDebugStringA("[MonsterLoader] Mushroom_Dark 로드 완료\n");
            }
            else
            {
                OutputDebugStringA("[MonsterLoader] Mushroom_Dark 메시 없음!\n");
            }
        }
    }




}

