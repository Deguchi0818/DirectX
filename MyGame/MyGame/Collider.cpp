#include "Collider.h"

AABB ColliderComponent::GetWorldAABB(const MyVector3& parentPos, const MyVector3& parentScale) const 
{
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

Sphere ColliderComponent::GetWorldSphere(const MyVector3& parentPos, const MyVector3& parentScale) const
{
    return {
        .x = parentPos.x + offset.x * parentScale.x,
        .y = parentPos.y + offset.y * parentScale.y,
        .z = parentPos.z + offset.z * parentScale.z,
        .radius = radius * parentScale.x
    };
}

Capsule ColliderComponent::GetWorldCapsule(const MyVector3& parentPos, const MyVector3& parentScale) const
{
    MyVector3 worldP1 = {
        parentPos.x + offset.x * parentScale.x,
        parentPos.y + (offset.y + height * 0.5f) * parentScale.y,
        parentPos.z + offset.z * parentScale.z
    };
    MyVector3 worldP2 = {
        parentPos.x + offset.x * parentScale.x,
        parentPos.y + (offset.y - height * 0.5f) * parentScale.y,
        parentPos.z + offset.z * parentScale.z
    };
    return {
        .p1 = worldP1,
        .p2 = worldP2,
        .radius = radius * parentScale.x
    };
}

float Clamp(float n, float min, float max) 
{
    if (n < min) return min;
    if (n > max) return max;
    return n;
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

bool Collider::SphereVsAABB(const Sphere& sphere, const AABB& aabb) 
{
    float closestX = Clamp(sphere.x, aabb.min.x, aabb.max.x);
    float closestY = Clamp(sphere.y, aabb.min.y, aabb.max.y);
    float closestZ = Clamp(sphere.z, aabb.min.z, aabb.max.z);

    float dx = sphere.x - closestX;
    float dy = sphere.y - closestY;
    float dz = sphere.z - closestZ;

    float distanceSq = dx * dx + dy * dy + dz * dz;

    // 半径の二乗と比較: d^2 <= r^2
    return distanceSq <= (sphere.radius * sphere.radius);
}