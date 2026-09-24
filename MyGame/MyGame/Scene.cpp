#include "Scene.h"
#include "ResourceManager.h"
#include "PhysicsEngine.h"

void Scene::UpdateAnimation()
{
    Model* pModel = m_player.pModel;
    if (!pModel) return;

    std::string currentAnim = m_player.GetCurrentAnimName();
    float animTime = m_player.GetAnimTimer();
    bool isLoop = (currentAnim != "Jump");

    pModel->CalculateBoneMatrices(currentAnim, animTime, isLoop,
        m_boneWorlds, m_skinMatrices);
}

void Scene::UpdateTransforms()
{
    // 親 → 子 の順に呼ぶこと
    for (auto& obj : GetTerrain())     obj->transform.UpdateMatrix();
    for (auto& obj : GetGameObjects()) obj->transform.UpdateMatrix();

   GetPlayer().transform.UpdateMatrix();

    // プレイヤーの行列が確定した後に、装着物を更新する
    GetPlayer().UpdateAttachments(m_boneWorlds);
}

void Scene::Draw(ID3D11DeviceContext* context, Shader* shader, ID3D11Buffer* cb,
    const MyMatrix4x4& view, const MyMatrix4x4& proj) 
{
    for (auto& obj : GetGameObjects())
    {
        obj->Draw(context, shader, cb, view, proj);
    }

    for (auto& obj : GetTerrain())
    {
        obj->Draw(context, shader, cb, view, proj);
    }

    GetPlayer().Draw(context, shader, cb, view, proj);
    GetPlayer().DrawAttachments(context, shader, cb, view, proj);

}

void Scene::Create(ResourceManager& resources, PhysicsEngine& physics)
{
    m_gameObjects.clear();
    m_terrain.clear();
    Model* pPlayerModel = resources.GetModel("Player");
    if (pPlayerModel) {
        m_player.Initialize(pPlayerModel);
    }

    // オブジェクトの配置
    auto floor = std::make_unique<GameObject>();
    floor->pModel = resources.GetModel("Plane");
    floor->transform.SetPosition(0, 0, 0);
    floor->transform.SetScale(50.0f, 1.0f, 50.0f);
    floor->isStatic = true;
    floor->AddCollider("floor_main", ColliderType::AABB, { 0, 0, 0 }, { 1.0f, 0.01f, 1.0f });
    m_terrain.push_back(std::move(floor));

    auto wall = std::make_unique<GameObject>();
    wall->pModel = resources.GetModel("Cube");
    wall->transform.SetPosition(5.0f, 1.0f, 0.0f);
    wall->transform.SetScale(1.0f, 7.0f, 5.0f);
    wall->isStatic = true;
    wall->m_isTrigger = false;
    wall->AddCollider("wall_main", ColliderType::AABB, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f });
    wall->m_friction = 1.0f;
    m_gameObjects.push_back(std::move(wall));

    auto block = std::make_unique<GameObject>();
    block->pModel = resources.GetModel("Cube");
    block->transform.SetPosition(0.0f, 3.5f, 0.0f);
    block->transform.SetScale(1.0f, 1.0f, 1.0f);
    block->isStatic = false;
    block->m_useGravity = true;
    block->m_isTrigger = false;
    block->m_showCollider = true;
    block->AddCollider("block", ColliderType::AABB, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f });
    m_gameObjects.push_back(std::move(block));

    auto coin = std::make_unique<Coin>();
    coin->pModel = resources.GetModel("Cube");
    coin->transform.SetPosition(-3.0f, 1.0f, 2.0f);
    coin->transform.SetScale(0.5f, 0.5f, 0.5f);
    coin->isStatic = true;
    coin->m_isTrigger = true;
    coin->m_showCollider = true;
    auto& coinCol = coin->AddCollider("coin", ColliderType::Sphere, { 0,0,0 }, { 1,1,1 });
    coinCol.radius = 0.5f;
    coinCol.isTrigger = true;
    m_gameObjects.push_back(std::move(coin));

    WeaponData swordData;
    swordData.name = "Sword";
    swordData.damage = 10.0f;
    swordData.model = resources.GetModel("Sword");
    swordData.localRotation = { 180.0f, -110.0f, 0.0f };
    m_sword.Initialize(swordData);
    m_sword.transform.SetScale(0.1f, 0.1f, 0.1f);
    m_sword.transform.SetRotation(0.0f, -110.0f, 90.0f);
    m_sword.transform.SetPosition(0.0f, 0.0f, 0.0f);

    auto target = std::make_unique<GameObject>();
    target->pModel = resources.GetModel("Cube");
    target->transform.SetPosition(0.0f, 1.0f, 10.0f);
    target->transform.SetScale(1.0f, 1.0f, 1.0f);
    target->isStatic = true;
    target->m_friction = 10.0f;
    target->m_showCollider = true;
    target->AddCollider("target", ColliderType::AABB, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f });
    m_target = target.get();// ポインタを控えてから
    m_gameObjects.push_back(std::move(target));// 所有権を渡す

    // 基底クラスの機能で装着する(盾や帽子も同じ書き方で増やせる)
    m_player.AttachToBone(&m_sword, "mixamorig:RightHand");
    m_player.SetLockOnTarget(m_target);


    physics.AddDynamicObject(&m_player);
    for (auto& obj : m_terrain)
    {
        physics.AddStaticObject(obj.get());
    }
    for (auto& obj : m_gameObjects)
    {
        if (obj->isStatic)
        {
            physics.AddStaticObject(obj.get());
        }
        else
        {
            physics.AddDynamicObject(obj.get());
        }
    }
}

