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
                if (isResolved) break;
                for (const auto& colB : b->m_colliders) 
                {
                    AABB worldA = colA.GetWorldAABB(a->transform.GetPosition(), a->transform.GetScale());
                    AABB worldB = colB.GetWorldAABB(b->transform.GetPosition(), b->transform.GetScale());

                    if (Collider::AABBCollider(worldA, worldB)) 
                    {
                        if (colA.isTrigger || colB.isTrigger)
                        {
                            a->OnTriggerEnter(b);
                        }
                        else 
                        {
                            ResolveOverlap(a, b, worldA, worldB);
                            a->OnCollisionEnter(colA.name, b, colB.name);
                            isResolved = true;
                            break;
                        }
                    }
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

    a->SetVelocity(vel); // C³‚µ‚½‘¬“x‚ð”½‰f
    a->transform.SetPosition(pos.x, pos.y, pos.z);
    a->transform.UpdateMatrix();
}