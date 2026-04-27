#pragma once
#include <vector>
#include "GameObject.h"
#include "Collider.h"

class PhysicsEngine
{
public:
	void AddDynamicObject(GameObject* obj){ m_dynamicObjects.push_back(obj); }
    void AddStaticObject(GameObject* obj) { m_staticObjects.push_back(obj); }

    void Update(float dt);

private:
    const float GRAVITY = -9.8f;

    // 動くもの（プレイヤー、敵など）
    std::vector<GameObject*> m_dynamicObjects;
    // 動かないもの（壁、床など）
    std::vector<GameObject*> m_staticObjects;

    // 内部的な物理計算（積分）
    void Integrate(GameObject* obj, float dt);
    // 衝突の解決（めり込み解消）
    void ResolveCollisions();

    void ResolveOverlap(GameObject* a, GameObject* b, const AABB& boxA, const AABB& boxB);

    void ResolveSphereAABBOverlap(GameObject* moveObj, GameObject* staticObj, const Sphere& s, const AABB& b, bool isSphereMove);

    void ApplyImpulse(GameObject* objA, GameObject* objB, const MyVector3& normal, float overlap);
};

