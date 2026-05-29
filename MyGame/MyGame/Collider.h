#pragma once
#include "Common.h"
#include <string>
#include <vector>

enum class ColliderType 
{
    AABB,
    Sphere,
    Capsule
};

struct ColliderComponent 
{
    std::string name;       // "Head", "Body" など
    ColliderType type;      // AABB か Sphere か
    MyVector3 offset;       // 親の中心からの相対座標
    MyVector3 scale;        // このコライダー自体の大きさ
    float radius = 0.0f;
    float height = 0.0f;           // カプセル用に高さを追加p1とp2の距離
    bool isTrigger = false;         // このパーツだけ通り抜け可能にするか

    // 親のTransformを考慮した最終的な計算をする
    AABB GetWorldAABB(const MyVector3& parentPos, const MyVector3& parentScale) const;
    Sphere GetWorldSphere(const MyVector3& parentPos, const MyVector3& parentScale) const;
    Capsule GetWorldCapsule(const MyVector3& parentPos, const MyVector3& parentScale) const;
};

class Collider
{
public:
    // 当たり判定の処理
	static bool SphereCollider(const Sphere& a, const Sphere& b);
	static bool AABBCollider(const AABB& a, const AABB& b);
    static bool SphereVsAABB(const Sphere& a, const AABB& b);
	static bool CapsuleVsAABB(const Capsule& capsule, const AABB& aabb);
	static bool CapsuleVsSphere(const Capsule& capsule, const Sphere& sphere);
    static MyVector3 GetClosestPointOnLineSegment(const MyVector3& A, const MyVector3& B, const MyVector3& P);
};

