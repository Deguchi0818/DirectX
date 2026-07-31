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

// --------------------------------------------------------
// 内部的な物理計算
// --------------------------------------------------------
void PhysicsEngine::Integrate(GameObject* obj, float dt) 
{
	MyVector3 vel = obj->GetVelocity();
	MyVector3 pos = obj->transform.GetPosition();

    if (obj->m_useGravity)
    {
        vel.y += GRAVITY * dt;
    }

	pos.x += vel.x * dt;
	pos.y += vel.y * dt;
	pos.z += vel.z * dt;

    obj->SetVelocity(vel);
    obj->transform.SetPosition(pos.x, pos.y, pos.z);
    obj->transform.UpdateMatrix();
}

// --------------------------------------------------------
// 衝突の解決（総当たりでペアを作り、めり込みを解消）
// --------------------------------------------------------
void PhysicsEngine::ResolveCollisions()
{
    // Dynamic vs Static
    for (auto& a : m_dynamicObjects)
    {
        for (auto& b : m_staticObjects)
        {
            CheckAndResolveCollision(a, b);
        }
    }

    // Dynamic vs Dynamic
    for (size_t i = 0; i < m_dynamicObjects.size(); ++i)
    {
        for (size_t j = i + 1; j < m_dynamicObjects.size(); ++j)
        {
            GameObject* a = m_dynamicObjects[i];
            GameObject* b = m_dynamicObjects[j];

            CheckAndResolveCollision(a, b);
        }
    }
}

// --------------------------------------------------------
// 具体的なペアの当たり判定とイベント発火
// --------------------------------------------------------
void PhysicsEngine::CheckAndResolveCollision(GameObject* a, GameObject* b) 
{
    for (const auto& colA : a->m_colliders)
    {
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
                    if (!colA.isTrigger && !colB.isTrigger)
                    {
                        // true を渡して、A(Sphere)を押し戻す
                        ResolveSphereAABBOverlap(a, b, worldA, worldB, true);
                    }
                }
            }
            // AABB (A) vs Sphere (B)
            else if (colA.type == ColliderType::AABB && colB.type == ColliderType::Sphere)
            {
                AABB worldA = colA.GetWorldAABB(a->transform.GetPosition(), a->transform.GetScale());
                Sphere worldB = colB.GetWorldSphere(b->transform.GetPosition(), b->transform.GetScale());

                if (Collider::SphereVsAABB(worldB, worldA))
                {
                    collisionDetected = true;
                    if (!colA.isTrigger && !colB.isTrigger)
                    {
                        // false を渡して、A(AABB)を逆方向に押し戻す
                        ResolveSphereAABBOverlap(a, b, worldB, worldA, false);
                    }
                }
            }

            // Capsule (A) vs AABB (B)
            else if (colA.type == ColliderType::Capsule && colB.type == ColliderType::AABB)
            {
                Capsule worldCapsule = colA.GetWorldCapsule(a->transform.GetPosition(), a->transform.GetScale());
                AABB worldAABB = colB.GetWorldAABB(b->transform.GetPosition(), b->transform.GetScale());

                if (Collider::CapsuleVsAABB(worldCapsule, worldAABB))
                {
                    collisionDetected = true;
                    if (!colA.isTrigger && !colB.isTrigger)
                    {
                        ResolveCapsuleAABBOverlap(a, b, worldCapsule, worldAABB);
                    }
                }
            }
            // AABB (A) vs Capsule (B) -- 逆パターン
            else if (colA.type == ColliderType::AABB && colB.type == ColliderType::Capsule)
            {
                AABB worldAABB = colA.GetWorldAABB(a->transform.GetPosition(), a->transform.GetScale());
                Capsule worldCapsule = colB.GetWorldCapsule(b->transform.GetPosition(), b->transform.GetScale());

                if (Collider::CapsuleVsAABB(worldCapsule, worldAABB))
                {
                    collisionDetected = true;
                    if (!colA.isTrigger && !colB.isTrigger)
                    {
                        // Bがカプセルなので、引数の順番を(b, a)にする
                        ResolveCapsuleAABBOverlap(b, a, worldCapsule, worldAABB);
                    }
                }
            }

            else if (colA.type == ColliderType::Capsule && colB.type == ColliderType::Sphere)
            {
                Capsule worldCapsule = colA.GetWorldCapsule(a->transform.GetPosition(), a->transform.GetScale());
                Sphere worldSphere = colB.GetWorldSphere(b->transform.GetPosition(), b->transform.GetScale());

                if (Collider::CapsuleVsSphere(worldCapsule, worldSphere))
                {
                    collisionDetected = true;
                    if (!colA.isTrigger && !colB.isTrigger)
                    {
                        ResolveCapsuleSphereOverlap(a, b, worldCapsule, worldSphere);
                    }
                }
            }

            else if (colA.type == ColliderType::Sphere && colB.type == ColliderType::Capsule)
            {
                Capsule worldCapsule = colB.GetWorldCapsule(b->transform.GetPosition(), b->transform.GetScale());
                Sphere worldSphere = colA.GetWorldSphere(a->transform.GetPosition(), a->transform.GetScale());

                if (Collider::CapsuleVsSphere(worldCapsule, worldSphere))
                {
                    collisionDetected = true;
                    if (!colA.isTrigger && !colB.isTrigger)
                    {
                        ResolveCapsuleSphereOverlap(a, b, worldCapsule, worldSphere);
                    }
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

        }
    }
    
}

// --------------------------------------------------------
// AABB同士の押し戻し処理
// --------------------------------------------------------
void PhysicsEngine::ResolveOverlap(GameObject* a, GameObject* b, const AABB& boxA, const AABB& boxB)
{

    float overlapX = (std::min)(boxA.max.x, boxB.max.x) - (std::max)(boxA.min.x, boxB.min.x);
    float overlapY = (std::min)(boxA.max.y, boxB.max.y) - (std::max)(boxA.min.y, boxB.min.y);
    float overlapZ = (std::min)(boxA.max.z, boxB.max.z) - (std::max)(boxA.min.z, boxB.min.z);

    MyVector3 normal = { 0,0,0 };               // 衝突面の法線
    float finalOverlap = 0;

    // 最短距離で押し戻す軸を特定し、法線を設定
    if (overlapX < overlapY && overlapX < overlapZ) 
    {
        float dir = (boxA.max.x < boxB.max.x) ? -1.0f : 1.0f;
        normal.x = dir; finalOverlap = overlapX;
    }
    else if (overlapY < overlapZ)
    {
        float dir = (boxA.max.y < boxB.max.y) ? -1.0f : 1.0f;
        normal.y = dir; finalOverlap = overlapY;
    }
    else 
    {
        float dir = (boxA.max.z < boxB.max.z) ? -1.0f : 1.0f;
        normal.z = dir; finalOverlap = overlapZ;
    }

    ApplyImpulse(a, b, normal, finalOverlap);
}

// --------------------------------------------------------
// SphereとAABBの押し戻し処理
// --------------------------------------------------------
void PhysicsEngine::ResolveSphereAABBOverlap(GameObject* moveObj, GameObject* staticObj, const Sphere& s, const AABB& b, bool isSphereMove)
{
    float closestX = std::clamp(s.x, b.min.x, b.max.x);
    float closestY = std::clamp(s.y, b.min.y, b.max.y);
    float closestZ = std::clamp(s.z, b.min.z, b.max.z);

    float dx = s.x - closestX;
    float dy = s.y - closestY;
    float dz = s.z - closestZ;

    float dist = sqrtf(dx * dx + dy * dy + dz * dz);
    if (dist < 0.0001f) return;

    float overlap = s.radius - dist;
    if (overlap <= 0) return;

   float moveDir = isSphereMove ? 1.0f : -1.0f;
    MyVector3 normal = { dx / dist * moveDir, dy / dist * moveDir, dz / dist * moveDir };

    ApplyImpulse(moveObj, staticObj, normal, overlap);
}

// --------------------------------------------------------
// CapsuleとAABBの押し戻し処理
// --------------------------------------------------------
void PhysicsEngine::ResolveCapsuleAABBOverlap(GameObject* capObj, GameObject* aabbObj, const Capsule& cap, const AABB& aabb)
{
    // カプセルの上下の線分の中で、AABBの高さ(Y)に一番近い場所を探す
    float minY = (std::min)(cap.p1.y, cap.p2.y);
    float maxY = (std::max)(cap.p1.y, cap.p2.y);
    float aabbCenterY = (aabb.min.y + aabb.max.y) * 0.5f;

    // カプセルの線分上で、AABBに一番近い高さ（pY）
    float pY = std::clamp(aabbCenterY, minY, maxY);
    MyVector3 P = { cap.p1.x, pY, cap.p1.z }; // これがカプセル側の最近接点 P

    // 2点Pに対して、AABBの表面面や辺で一番近い場所を探す
    float qX = std::clamp(P.x, aabb.min.x, aabb.max.x);
    float qY = std::clamp(P.y, aabb.min.y, aabb.max.y);
    float qZ = std::clamp(P.z, aabb.min.z, aabb.max.z);


    // カプセルの点P と AABBの表面の点Q の距離を計算する
    float dx = P.x - qX;
    float dy = P.y - qY;
    float dz = P.z - qZ;
    float dist = sqrtf(dx * dx + dy * dy + dz * dz);

    MyVector3 normal = { 0, 0, 0 };
    float overlap = 0.0f;

    if (dist > 0.0001f)
    {
        // 通常の衝突外側から壁や床にぶつかっている
        overlap = cap.radius - dist;
        if (overlap <= 0.0f) return; // 届いていなければ何もしない

        normal = { dx / dist, dy / dist, dz / dist };
    }
    else
    {
        // AABBの中心から外側へ向けて、一番近い面（前後左右上下）を探して押し出す
        float dx_to_max = aabb.max.x - P.x;
        float dx_to_min = P.x - aabb.min.x;
        float dy_to_max = aabb.max.y - P.y;
        float dy_to_min = P.y - aabb.min.y;
        float dz_to_max = aabb.max.z - P.z;
        float dz_to_min = P.z - aabb.min.z;

        float minOverlap = dx_to_max; normal = { 1, 0, 0 }; // まずXのプラス方向を仮の最短とする

        if (dx_to_min < minOverlap) { minOverlap = dx_to_min; normal = { -1, 0, 0 }; }
        if (dy_to_max < minOverlap) { minOverlap = dy_to_max; normal = { 0, 1, 0 }; }
        if (dy_to_min < minOverlap) { minOverlap = dy_to_min; normal = { 0, -1, 0 }; }
        if (dz_to_max < minOverlap) { minOverlap = dz_to_max; normal = { 0, 0, 1 }; }
        if (dz_to_min < minOverlap) { minOverlap = dz_to_min; normal = { 0, 0, -1 }; }

        // 面までの距離 ＋ カプセルの半径分 を押し出す
        overlap = minOverlap + cap.radius;
    }

    // カプセルを押し出す！
    ApplyImpulse(capObj, aabbObj, normal, overlap);
}

// --------------------------------------------------------
// CapsuleとSphereの押し戻し処理
// --------------------------------------------------------
void PhysicsEngine::ResolveCapsuleSphereOverlap(GameObject* capObj, GameObject* sphereObj, const Capsule& cap, const Sphere& sph)
{
    // カプセル上の最近接点を探す
    MyVector3 closestPoint = Collider::GetClosestPointOnLineSegment(cap.p1, cap.p2, { sph.x, sph.y, sph.z });
    
	// 最近接点と球の中心の距離を計算
    float dx = closestPoint.x - sph.x;
    float dy = closestPoint.y - sph.y;
    float dz = closestPoint.z - sph.z;
    float dist = sqrtf(dx * dx + dy * dy + dz * dz);
    
	// 距離が非常に小さい場合は、重なりが大きすぎて正確な法線が計算できないため、処理をスキップ
    if (dist < 0.0001f) return;
    
    // めり込み量
    float overlap = (cap.radius + sph.radius) - dist;
    if (overlap <= 0) return;
    
	// カプセルを押し戻す場合は法線の向きをそのまま、球を押し戻す場合は逆向きにする
    MyVector3 normal = { dx / dist, dy / dist, dz / dist };

    ApplyImpulse(capObj, sphereObj, normal, overlap);
}

// --------------------------------------------------------
// 物理法則（めり込み解消・反発・摩擦）の適用
// --------------------------------------------------------
void PhysicsEngine::ApplyImpulse(GameObject* objA, GameObject* objB, const MyVector3& normal, float overlap)
{
    // isStaticがtrueの場合は質量無限,大絶対に動かないとして扱うため、逆数を0にする
    float invMassA = objA->isStatic ? 0.0f : (1.0f / objA->m_mass);
    float invMassB = objB->isStatic ? 0.0f : (1.0f / objB->m_mass);

    // 両方とも動かない物体なら計算をスキップ
    float totalInvMass = invMassA + invMassB;
    if (totalInvMass == 0.0f) return;

    MyVector3 posA = objA->transform.GetPosition();
    MyVector3 posB = objB->transform.GetPosition();

    posA.x += normal.x * overlap * (invMassA / totalInvMass);
    posA.y += normal.y * overlap * (invMassA / totalInvMass);
    posA.z += normal.z * overlap * (invMassA / totalInvMass);

    posB.x -= normal.x * overlap * (invMassB / totalInvMass);
    posB.y -= normal.y * overlap * (invMassB / totalInvMass);
    posB.z -= normal.z * overlap * (invMassB / totalInvMass);

    objA->transform.SetPosition(posA.x, posA.y, posA.z);
    objB->transform.SetPosition(posB.x, posB.y, posB.z);


    MyVector3 velA = objA->GetVelocity();
    MyVector3 velB = objB->GetVelocity();
    // 相対速度の法線成分を計算
    MyVector3 relativeVel = { velA.x - velB.x, velA.y - velB.y , velA.z - velB.z };
    float relativeVelNormal = relativeVel.x * normal.x + relativeVel.y * normal.y + relativeVel.z * normal.z;

    // 物体が壁に向かっている場合のみ跳ね返らせる
    if (relativeVelNormal < 0)
    {
        float e = std::min(objA->m_restitution, objB->m_restitution);

        // 反発係数を使った計算
        float j = -(1.0f + e) * relativeVelNormal;
        j /= totalInvMass;

        // 速度にインパルスを適用
        velA.x += j * normal.x * invMassA;
        velA.y += j * normal.y * invMassA;
        velA.z += j * normal.z * invMassA;

        velB.x -= j * normal.x * invMassB;
        velB.y -= j * normal.y * invMassB;
        velB.z -= j * normal.z * invMassB;

        // 衝突面に沿った接線ベクトルTangentを求める
        MyVector3 tangent = {
            relativeVel.x - relativeVelNormal * normal.x,
            relativeVel.y - relativeVelNormal * normal.y,
            relativeVel.z - relativeVelNormal * normal.z
        };

        // 接線ベクトルの長さを計算
        float tangentLen = sqrtf(tangent.x * tangent.x + tangent.y * tangent.y + tangent.z * tangent.z);

        // ほぼ停止していない場合のみ摩擦をかける
        if (tangentLen > 0.0001f) 
        {
            // 正規化
            tangent.x /= tangentLen;
            tangent.y /= tangentLen;
            tangent.z /= tangentLen;

            // 接線方向の速度を求める
            float relativeVelTangent = relativeVel.x * tangent.x + relativeVel.y * tangent.y + relativeVel.z * tangent.z;

            // 摩擦係数を合成
            float friction = objA->m_friction * objB->m_friction;

            // 滑りを完全に止めるために必要な摩擦インパルス (jt)
            float jt = -relativeVelTangent;
            jt /= totalInvMass;

            // クーロンの法則：摩擦力は垂直に押し付ける力(j) × 摩擦係数を超えられない
            float maxFriction = j * friction;

            // 必要な摩擦力が上限を超えていたらクランプする
            if (jt > maxFriction) jt = maxFriction;
            if (jt < -maxFriction)jt = -maxFriction;

            // 摩擦インパルスの適用
            velA.x += jt * tangent.x * invMassA;
            velA.y += jt * tangent.y * invMassA;
            velA.z += jt * tangent.z * invMassA;

            velB.x -= jt * tangent.x * invMassB;
            velB.y -= jt * tangent.y * invMassB;
            velB.z -= jt * tangent.z * invMassB;
        }
    }

    objA->SetVelocity(velA); // 修正した速度を反映
    objB->SetVelocity(velB);
    objA->transform.UpdateMatrix();
    objB->transform.UpdateMatrix();
}