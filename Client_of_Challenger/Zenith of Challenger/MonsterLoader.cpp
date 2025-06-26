#include "MonsterLoader.h"
#include "Monster.h"

void LoadAllMonsters(
    const ComPtr<ID3D12Device>& device,
    const unordered_map<string, shared_ptr<Texture>>& textures,
    const unordered_map<string, shared_ptr<Shader>>& shaders,
    const unordered_map<string, shared_ptr<MeshBase>>& meshLibrary,
    const unordered_map<string, unordered_map<string, AnimationClip>>& animClipLibrary,
    const unordered_map<string, unordered_map<int, XMMATRIX>>& boneOffsetLibrary,
    const unordered_map<string, unordered_map<string, int>>& boneMap,
    const unordered_map<string, unordered_map<string, string>>& boneHierarchy,
    const unordered_map<string, unordered_map<string, XMMATRIX>>& NodeNameToGlobalTransform,
    unordered_map<string, vector<shared_ptr<Monsters>>>& outMonsterGroups,
    const shared_ptr<Camera>& camera)
{
    // === 1. Frightfly 몬스터 생성 ===
    if (!meshLibrary.contains("FrightFly")) {
        OutputDebugStringA("[MonsterLoader] Frightfly 메쉬 없음\n");
        return;
    }

    for (int i = 0; i < 10; ++i)
    {
        auto frightfly = make_shared<Frightfly>(device);

        // [1] 메쉬, 텍스처, 셰이더 설정
        frightfly->SetCamera(camera);
        frightfly->SetMesh(meshLibrary.at("FrightFly"));
        frightfly->SetTexture(textures.at("FrightFly"));
        frightfly->SetTextureIndex(textures.at("FrightFly")->GetTextureIndex());
        frightfly->SetShader(shaders.at("FrightFly"));
        frightfly->SetDebugLineShader(shaders.at("DebugLineShader"));
        frightfly->SetHealthBarShader(shaders.at("HealthBarShader"));
        frightfly->m_scale = XMFLOAT3{ 0.05, 0.05, 0.05 };
        frightfly->SetScale(frightfly->m_scale);
        frightfly->m_monNum = 0;

        // [2] 애니메이션 설정
        vector<AnimationClip> clips;
        for (const auto& [name, clip] : animClipLibrary.at("FrightFly"))
            clips.push_back(clip);

        frightfly->SetAnimationClips(clips);
        frightfly->SetCurrentAnimation("Polygonal_Frightfly_01__2_|Idle|Animation Base Layer");
        //frightfly->SetCurrentAnimation("Die");
        frightfly->SetBoneOffsets(boneOffsetLibrary.at("FrightFly"));
        frightfly->SetBoneNameToIndex(boneMap.at("FrightFly"));
        frightfly->SetBoneHierarchy(boneHierarchy.at("FrightFly"));
        frightfly->SetNodeNameToGlobalTransform(NodeNameToGlobalTransform.at("FrightFly")); 

        // [3] 바운딩 박스
        BoundingBox box;
        box.Center = XMFLOAT3{ 0.f, 5.5f, 0.f };
        box.Extents = XMFLOAT3{ 1.5f, 1.5f, 1.5f };
        frightfly->SetFlyFrightBoundingBox(box);

        // [4] 본 버퍼 SRV 생성
        auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
        int k = 0;
         k = gGameFramework->GetCurrentSRVOffset();
        frightfly->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

        // [5] 그룹에 추가
        outMonsterGroups["FrightFly"].push_back(frightfly);
    }

    OutputDebugStringA("[MonsterLoader] FrightFly 몬스터 1마리 로드 완료\n");

      //Flower Fairy
    for (int i = 0; i < 10; ++i)
    {
        auto monster = make_shared<FlowerFairy>(device);
        monster->SetCamera(camera);
        // [1] 메쉬, 텍스처, 셰이더 설정
        monster->SetMesh(meshLibrary.at("Flower_Fairy"));
        monster->SetTexture(textures.at("Flower_Fairy"));
        monster->SetTextureIndex(textures.at("Flower_Fairy")->GetTextureIndex());
        monster->SetShader(shaders.at("FrightFly"));
        monster->SetDebugLineShader(shaders.at("DebugLineShader"));
        monster->SetHealthBarShader(shaders.at("HealthBarShader"));
        monster->m_scale = XMFLOAT3{ 0.1, 0.1, 0.1 };
        monster->SetScale(monster->m_scale);
        monster->m_monNum = 1;
        // [2] 애니메이션 설정
        vector<AnimationClip> clips;
        for (const auto& [name, clip] : animClipLibrary.at("Flower_Fairy"))
            clips.push_back(clip);

        monster->SetAnimationClips(clips);
        monster->SetCurrentAnimation("Polygonal_Flower_Fairy_Yellow|Polygonal_Flower_Fairy_Yellow|Idle|Animation Base Layer");
        //monster->SetCurrentAnimation("Die");
        monster->SetBoneOffsets(boneOffsetLibrary.at("Flower_Fairy"));
        monster->SetBoneNameToIndex(boneMap.at("Flower_Fairy"));
        monster->SetBoneHierarchy(boneHierarchy.at("Flower_Fairy"));
        monster->SetNodeNameToGlobalTransform(NodeNameToGlobalTransform.at("Flower_Fairy"));

        // [3] 바운딩 박스
        BoundingBox box;
        box.Center = XMFLOAT3{ 0.f, 5.5f, 0.f };
        box.Extents = XMFLOAT3{ 1.5f, 4.0f, 1.5f };
        monster->SetMonstersBoundingBox(box);

        // [4] 본 버퍼 SRV 생성
        auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
        int k = 0;
        k = gGameFramework->GetCurrentSRVOffset();
        monster->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

        // [5] 그룹에 추가
        outMonsterGroups["Flower_Fairy"].push_back(monster);
    }

    //// [3] Mushroom Dark
    for (int i = 0; i < 10; ++i)
    {
        auto monster = make_shared<MushroomDark>(device);
        monster->SetCamera(camera);
        // [1] 메쉬, 텍스처, 셰이더 설정
        monster->SetMesh(meshLibrary.at("Mushroom_Dark"));
        monster->SetTexture(textures.at("Mushroom_Dark"));
        monster->SetTextureIndex(textures.at("Mushroom_Dark")->GetTextureIndex());
        monster->SetShader(shaders.at("FrightFly"));
        monster->SetDebugLineShader(shaders.at("DebugLineShader"));
        monster->m_scale = XMFLOAT3{ 0.1, 0.1, 0.1 };
        monster->SetScale(monster->m_scale);
        monster->SetHealthBarShader(shaders.at("HealthBarShader"));
        monster->m_monNum = 2; 
        // [2] 애니메이션 설정
        vector<AnimationClip> clips;
        for (const auto& [name, clip] : animClipLibrary.at("Mushroom_Dark"))
            clips.push_back(clip);

        monster->SetAnimationClips(clips);
        monster->SetCurrentAnimation("Polygonal_Mushroom_Dark__1_|Polygonal_Mushroom_Dark__1_|Idle|Animation Base Layer");
        //monster->SetCurrentAnimation("Die");
        monster->SetBoneOffsets(boneOffsetLibrary.at("Mushroom_Dark"));
        monster->SetBoneNameToIndex(boneMap.at("Mushroom_Dark"));
        monster->SetBoneHierarchy(boneHierarchy.at("Mushroom_Dark"));
        monster->SetNodeNameToGlobalTransform(NodeNameToGlobalTransform.at("Mushroom_Dark"));

        // [3] 바운딩 박스
        BoundingBox box;
        box.Center = XMFLOAT3{ 0.f, 0.0f, 0.f };
        box.Extents = XMFLOAT3{ 4.0f, 10.0f, 4.0f };
        monster->SetMonstersBoundingBox(box);

        // [4] 본 버퍼 SRV 생성
        auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
        int k = 0;
        k = gGameFramework->GetCurrentSRVOffset();
        monster->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

        // [5] 그룹에 추가
        outMonsterGroups["Mushroom_Dark"].push_back(monster);
    }

    //// [4] Venus_Blue
    for (int i = 0; i < 10; ++i)
    {
        auto monster = make_shared<VenusBlue>(device); // VenusBlue 클래스 필요
        monster->SetCamera(camera);
        // [1] 메쉬, 텍스처, 셰이더 설정
        monster->SetMesh(meshLibrary.at("Venus_Blue"));
        monster->SetTexture(textures.at("Venus_Blue"));
        monster->SetTextureIndex(textures.at("Venus_Blue")->GetTextureIndex());
        monster->SetShader(shaders.at("FrightFly"));
        monster->SetDebugLineShader(shaders.at("DebugLineShader"));
        monster->SetHealthBarShader(shaders.at("HealthBarShader"));
        monster->m_scale = XMFLOAT3{ 0.1, 0.1, 0.1 };
        monster->SetScale(monster->m_scale);
        monster->m_monNum = 3;
        // [2] 애니메이션 설정
        vector<AnimationClip> clips;
        for (const auto& [name, clip] : animClipLibrary.at("Venus_Blue"))
            clips.push_back(clip);

        monster->SetAnimationClips(clips);
        monster->SetCurrentAnimation("Polygonal_Plant_Venus_Blue|Polygonal_Plant_Venus_Blue|Idle|Animation Base Layer");
        //monster->SetCurrentAnimation("Die");
        monster->SetBoneOffsets(boneOffsetLibrary.at("Venus_Blue"));
        monster->SetBoneNameToIndex(boneMap.at("Venus_Blue"));
        monster->SetBoneHierarchy(boneHierarchy.at("Venus_Blue"));
        monster->SetNodeNameToGlobalTransform(NodeNameToGlobalTransform.at("Venus_Blue"));

        // [3] 바운딩 박스
        BoundingBox box;
        box.Center = XMFLOAT3{ 0.f, 5.5f, 0.f };
        box.Extents = XMFLOAT3{ 4.5f, 9.5f, 5.5f };
        monster->SetMonstersBoundingBox(box);

        // [4] 본 버퍼 SRV 생성
        auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
        int k = 0;
        k = gGameFramework->GetCurrentSRVOffset();
        monster->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

        // [5] 그룹에 추가
        outMonsterGroups["Venus_Blue"].push_back(monster);
    }


    //// [5] Plant_Dionaea
    for (int i = 0; i < 10; ++i)
    {
        auto monster = make_shared<PlantDionaea>(device); // PlantDionaea 클래스 필요
        // [1] 메쉬, 텍스처, 셰이더 설정
        monster->SetMesh(meshLibrary.at("Plant_Dionaea"));
        monster->SetCamera(camera);
        monster->SetTexture(textures.at("Plant_Dionaea"));
        monster->SetTextureIndex(textures.at("Plant_Dionaea")->GetTextureIndex());
        monster->SetShader(shaders.at("FrightFly"));
        monster->SetDebugLineShader(shaders.at("DebugLineShader"));
        monster->m_scale = XMFLOAT3{ 0.1, 0.1, 0.1 };
        monster->SetScale(monster->m_scale);
        monster->m_monNum = 4;
        // [2] 애니메이션 설정
        vector<AnimationClip> clips;
        for (const auto& [name, clip] : animClipLibrary.at("Plant_Dionaea"))
            clips.push_back(clip);
        monster->SetHealthBarShader(shaders.at("HealthBarShader"));

        monster->SetAnimationClips(clips);
        monster->SetCurrentAnimation("Polygonal_Plant_Dionaea_Green|Polygonal_Plant_Dionaea_Green|Idle|Animation Base Layer");
        //monster->SetCurrentAnimation("Die");
        monster->SetBoneOffsets(boneOffsetLibrary.at("Plant_Dionaea"));
        monster->SetBoneNameToIndex(boneMap.at("Plant_Dionaea"));
        monster->SetBoneHierarchy(boneHierarchy.at("Plant_Dionaea"));
        monster->SetNodeNameToGlobalTransform(NodeNameToGlobalTransform.at("Plant_Dionaea"));

        // [3] 바운딩 박스
        BoundingBox box;
        box.Center = XMFLOAT3{ 0.f, 5.5f, 0.f };
        box.Extents = XMFLOAT3{ 5.5f, 9.5f, 5.5f };
        monster->SetMonstersBoundingBox(box);

        // [4] 본 버퍼 SRV 생성
        auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
        int k = 0;
        k = gGameFramework->GetCurrentSRVOffset();
        monster->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

        // [5] 그룹에 추가
        outMonsterGroups["Plant_Dionaea"].push_back(monster);
    }


    //// [6] Metalon 보스
    for (int i = 0; i < 1; ++i)
    {
        auto monster = make_shared<Boss>(device); // PlantDionaea 클래스 필요
        // [1] 메쉬, 텍스처, 셰이더 설정
        monster->SetMesh(meshLibrary.at("Metalon"));
        monster->SetCamera(camera);
        monster->SetTexture(textures.at("Metalon"));
        monster->SetTextureIndex(textures.at("Metalon")->GetTextureIndex());
        monster->SetShader(shaders.at("FrightFly"));
        monster->SetDebugLineShader(shaders.at("DebugLineShader"));
        monster->m_scale = XMFLOAT3{ 0.2, 0.2, 0.2 };
        monster->SetScale(monster->m_scale);
        monster->m_monNum = 5;
        // [2] 애니메이션 설정
        vector<AnimationClip> clips;
        for (const auto& [name, clip] : animClipLibrary.at("Metalon"))
            clips.push_back(clip);
        monster->SetHealthBarShader(shaders.at("HealthBarShader"));

        monster->SetAnimationClips(clips);
        monster->SetCurrentAnimation("Polygonal_Metalon_Purple|Polygonal_Metalon_Purple|Idle|Animation Base Layer");
        //monster->SetCurrentAnimation("Polygonal_Metalon_Purple.001|Polygonal_Metalon_Purple.001|Idle|Animation Base Layer");
        //monster->SetCurrentAnimation("Die");
        monster->SetBoneOffsets(boneOffsetLibrary.at("Metalon"));
        monster->SetBoneNameToIndex(boneMap.at("Metalon"));
        monster->SetBoneHierarchy(boneHierarchy.at("Metalon"));
        monster->SetNodeNameToGlobalTransform(NodeNameToGlobalTransform.at("Metalon"));

        // [3] 바운딩 박스
        BoundingBox box;
        box.Center = XMFLOAT3{ 0.f, 5.5f, 0.f };
        box.Extents = XMFLOAT3{ 5.5f, 9.5f, 5.5f };
        monster->SetMonstersBoundingBox(box);

        // [4] 본 버퍼 SRV 생성
        auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
        int k = 0;
        k = gGameFramework->GetCurrentSRVOffset();
        monster->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

        // [5] 그룹에 추가
        outMonsterGroups["Metalon"].push_back(monster);
    }

}

