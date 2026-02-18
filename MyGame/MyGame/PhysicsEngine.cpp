#include "PhysicsEngine.h"
#include <algorithm>

void PhysicsEngine::Update(float dt) 
{
    for (auto& obj : m_dynamicObjects)
    {
        Integrate(obj, dt);
    }

	ResolveCollisions();
}

void PhysicsEngine::Integrate(GameObject* obj, float dt) 
{
	MyVector3 vel = obj->GetVelocity();
	MyVector3 pos = obj->transform.GetPosition();

	vel.y += GRAVITY * dt;

	pos.x += vel.x * dt;
	pos.y += vel.y * dt;
	pos.z += vel.z * dt;

    obj->SetVelocity(vel);
    obj->transform.SetPosition(pos.x, pos.y, pos.z);
    obj->transform.UpdateMatrix();
}

void PhysicsEngine::ResolveCollisions()
{
    for (auto& a : m_dynamicObjects)
    {
        for (auto& b : m_staticObjects)
        {
            bool isResolved = false;
            for (const auto& colA : a->m_colliders)
            {
                if (isResolved) break; // このペアの衝突が解決済みなら次のコライダーへ
                for (const auto& colB : b->m_colliders)
                {
                    bool collisionDetected = false;

                    // --- 組み合わせごとの判定ロジック ---

                    // AABB vs AABB
                    if (colA.type == ColliderType::AABB && colB.type == ColliderType::AABB)
                    {
                        AABB worldA = colA.GetWorldAABB(a->transform.GetPosition(), a->transform.GetScale());
                        AABB worldB = colB.GetWorldAABB(b->transform.GetPosition(), b->transform.GetScale());

                        if (Collider::AABBCollider(worldA, worldB))
                        {
                            collisionDetected = true;
                            // トリガーでなければ押し戻しを行う
                            if (!colA.isTrigger && !colB.isTrigger) {
                                ResolveOverlap(a, b, worldA, worldB);
                                isResolved = true;
                            }
                        }
                    }
                    // Sphere vs Sphere
                    else if (colA.type == ColliderType::Sphere && colB.type == ColliderType::Sphere)
                    {
                        Sphere worldA = colA.GetWorldSphere(a->transform.GetPosition(), a->transform.GetScale());
                        Sphere worldB = colB.GetWorldSphere(b->transform.GetPosition(), b->transform.GetScale());

                        if (Collider::SphereCollider(worldA, worldB))
                        {
                            collisionDetected = true;
                        }
                    }
                    // Sphere (A) vs AABB (B)
                    else if (colA.type == ColliderType::Sphere && colB.type == ColliderType::AABB)
                    {
                        Sphere worldA = colA.GetWorldSphere(a->transform.GetPosition(), a->transform.GetScale());
                        AABB worldB = colB.GetWorldAABB(b->transform.GetPosition(), b->transform.GetScale());

                        if (Collider::SphereVsAABB(worldA, worldB))
                        {
                            collisionDetected = true;
                        }
                    }
                    // AABB (A) vs Sphere (B)
                    else if (colA.type == ColliderType::AABB && colB.type == ColliderType::Sphere)
                    {
                        AABB worldA = colA.GetWorldAABB(a->transform.GetPosition(), a->transform.GetScale());
                        Sphere worldB = colB.GetWorldSphere(b->transform.GetPosition(), b->transform.GetScale());

                        // 引数を入れ替えて SphereVsAABB を呼び出す
                        if (Collider::SphereVsAABB(worldB, worldA))
                        {
                            collisionDetected = true;
                        }
                    }

                    // --- 衝突後のイベント通知 ---
                    if (collisionDetected)
                    {
                        if (colA.isTrigger || colB.isTrigger)
                        {
                            a->OnTriggerEnter(b);
                        }
                        else
                        {
                            a->OnCollisionEnter(colA.name, b, colB.name);
                        }
                    }

                    if (isResolved) break;
                }
            }
        }
    }
}

void PhysicsEngine::ResolveOverlap(GameObject* a, GameObject* b, const AABB& boxA, const AABB& boxB)
{

    float overlapX = (std::min)(boxA.max.x, boxB.max.x) - (std::max)(boxA.min.x, boxB.min.x);
    float overlapY = (std::min)(boxA.max.y, boxB.max.y) - (std::max)(boxA.min.y, boxB.min.y);
    float overlapZ = (std::min)(boxA.max.z, boxB.max.z) - (std::max)(boxA.min.z, boxB.min.z);

    MyVector3 pos = a->transform.GetPosition();
    MyVector3 vel = a->GetVelocity();

    if (overlapX < overlapY && overlapX < overlapZ) {
        float dir = (boxA.max.x < boxB.max.x) ? -overlapX : overlapX;
        pos.x += dir;
        vel.x = 0; 
    }
    else if (overlapY < overlapZ)
    {
        float dir = (boxA.max.y < boxB.max.y) ? -overlapY : overlapY;
        pos.y += dir;
        if (dir > 0) 
        {
            vel.y = 0;
        }
    }
    else 
    {
        float dir = (boxA.max.z < boxB.max.z) ? -overlapZ : overlapZ;
        pos.z += dir;
        vel.z = 0;
    }

    a->SetVelocity(vel); // 修正した速度を反映
    a->transform.SetPosition(pos.x, pos.y, pos.z);
    a->transform.UpdateMatrix();
}