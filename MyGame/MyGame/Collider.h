#pragma once
#include "Common.h"
#include <string>
#include <vector>

enum class ColliderType 
{
    AABB,
    Sphere
};

struct ColliderComponent {
    std::string name;       // "Head", "Body" など
    ColliderType type;      // AABB か Sphere か
    MyVector3 offset;       // 親の中心からの相対座標
    MyVector3 scale;        // このコライダー自体の大きさ
    bool isTrigger;         // このパーツだけ通り抜け可能にするか

    // 親の Transform を考慮した最終的な AABB を計算する
    AABB GetWorldAABB(const MyVector3& parentPos, const MyVector3& parentScale) const;
};

class Collider
{
public:
	static bool SphereCollider(const Sphere& a, const Sphere& b);
	static bool AABBCollider(const AABB& a, const AABB& b);
};

