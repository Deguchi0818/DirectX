#include "Collider.h"

AABB ColliderComponent::GetWorldAABB(const MyVector3& parentPos, const MyVector3& parentScale) const {
    // 親のスケールを考慮してオフセットとサイズを計算
    MyVector3 worldCenter = {
        parentPos.x + offset.x * parentScale.x,
        parentPos.y + offset.y * parentScale.y,
        parentPos.z + offset.z * parentScale.z
    };

    MyVector3 worldHalfSize = {
        (scale.x * parentScale.x) * 0.5f,
        (scale.y * parentScale.y) * 0.5f,
        (scale.z * parentScale.z) * 0.5f
    };

    return {
        .min = { worldCenter.x - worldHalfSize.x, worldCenter.y - worldHalfSize.y, worldCenter.z - worldHalfSize.z },
        .max = { worldCenter.x + worldHalfSize.x, worldCenter.y + worldHalfSize.y, worldCenter.z + worldHalfSize.z }
    };
}

bool Collider::SphereCollider(const Sphere& a, const Sphere& b) 
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;

	// 中心点間の距離を二乗
	float distanceSq = dx * dx + dy * dy + dz * dz;

	// 半径を足したものを二乗
	float radiusSum = a.radius + b.radius;
	float radiusSumSq = radiusSum * radiusSum;

	// 満たしていれば接触している
	return distanceSq <= radiusSumSq;
}

bool Collider::AABBCollider(const AABB& a, const AABB& b) 
{
	if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
	if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
	if (a.max.z < b.min.z || a.min.z > b.max.z) return false;

	return true;
}