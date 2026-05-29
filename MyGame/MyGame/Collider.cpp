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

MyVector3 Collider::GetClosestPointOnLineSegment(const MyVector3& A, const MyVector3& B, const MyVector3& P) 
{
    // ベクトル AB と AP
	MyVector3 AB = { B.x - A.x, B.y - A.y, B.z - A.z };
	MyVector3 AP = { P.x - A.x, P.y - A.y, P.z - A.z };

    // ABの長さの二乗
	float abSq = AB.x * AB.x + AB.y * AB.y + AB.z * AB.z;
	if (abSq == 0.0f) return A; // A と B が同じ点の場合

    // 内積(AP・AB) を使って、AからBに向かってどれくらい進んだ位置が最短か（割合 t）を求める
	float t = (AP.x * AB.x + AP.y * AB.y + AP.z * AB.z) / abSq;

	t = Clamp(t, 0.0f, 1.0f);

    // 最近接点の座標を計算して返す
	return 
    {
		A.x + AB.x * t,
		A.y + AB.y * t,
		A.z + AB.z * t
	};
}

// Sphereの当たり判定
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

// AABBコライダーの当たり判定
bool Collider::AABBCollider(const AABB& a, const AABB& b) 
{
	if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
	if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
	if (a.max.z < b.min.z || a.min.z > b.max.z) return false;

	return true;
}

// SphereとAABBの当たり判定
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

// カプセルとAABBの当たり判定
bool Collider::CapsuleVsAABB(const Capsule& capsule, const AABB& aabb) 
{
    float minY = std::min(capsule.p1.y, capsule.p2.y);
	float maxY = std::max(capsule.p1.y, capsule.p2.y);
	float aabbCenterY = (aabb.min.y + aabb.max.y) * 0.5f;

	float pY = Clamp(aabbCenterY, minY, maxY);

	MyVector3 p = { capsule.p1.x, pY, capsule.p1.z };

	float qX = Clamp(p.x, aabb.min.x, aabb.max.x);
	float qY = Clamp(p.y, aabb.min.y, aabb.max.y);
	float qZ = Clamp(p.z, aabb.min.z, aabb.max.z);

	float dx = p.x - qX;
	float dy = p.y - qY;
	float dz = p.z - qZ;
	float distanceSq = dx * dx + dy * dy + dz * dz;

	return distanceSq <= (capsule.radius * capsule.radius);
}

// カプセルとSphereの当たり判定
bool Collider::CapsuleVsSphere(const Capsule& capsule, const Sphere& sphere) 
{
    // カプセルの中心線（p1からp2）上で、球の中心に最も近い点を探す
    MyVector3 closestPoint = GetClosestPointOnLineSegment(capsule.p1, capsule.p2, { sphere.x, sphere.y, sphere.z });
    
    // その最近接点から、球の中心までの距離（の二乗）を計算する
    float dx = sphere.x - closestPoint.x;
    float dy = sphere.y - closestPoint.y;
    float dz = sphere.z - closestPoint.z;

    // お互いの半径を足した距離（の二乗）と比較する
    float distanceSq = dx * dx + dy * dy + dz * dz;

    // 実際の距離が、半径の合計よりも短ければ当たっている
    return distanceSq <= (capsule.radius + sphere.radius) * (capsule.radius + sphere.radius);
}