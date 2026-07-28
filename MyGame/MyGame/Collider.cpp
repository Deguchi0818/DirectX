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

// --------------------------------------------------------
// 線分上の最近接点算出
// --------------------------------------------------------
MyVector3 Collider::GetClosestPointOnLineSegment(const MyVector3& A, const MyVector3& B, const MyVector3& P) 
{
	MyVector3 AB = { B.x - A.x, B.y - A.y, B.z - A.z };
	MyVector3 AP = { P.x - A.x, P.y - A.y, P.z - A.z };

	float abSq = AB.x * AB.x + AB.y * AB.y + AB.z * AB.z;
	if (abSq == 0.0f) return A; // 線分が点の場合のゼロ除算防止

    // 内積(AP・AB) を使って、AからBに向かってどれくらい進んだ位置が最短か（割合 t）を求める
	float t = (AP.x * AB.x + AP.y * AB.y + AP.z * AB.z) / abSq;

	t = Clamp(t, 0.0f, 1.0f);   // 割合を0〜1に収め、線分の外側にはみ出さないようにする

	return 
    {
		A.x + AB.x * t,
		A.y + AB.y * t,
		A.z + AB.z * t
	};
}

// --------------------------------------------------------
// 球体同士の判定
// --------------------------------------------------------
bool Collider::SphereCollider(const Sphere& a, const Sphere& b) 
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;

    // 平方根(sqrt)は計算負荷が高いため、距離も半径も「二乗」のまま比較する
	float distanceSq = dx * dx + dy * dy + dz * dz;
	float radiusSum = a.radius + b.radius;
	float radiusSumSq = radiusSum * radiusSum;

	return distanceSq <= radiusSumSq;
}

// --------------------------------------------------------
// AABB と AABB の判定
// --------------------------------------------------------
bool Collider::AABBCollider(const AABB& a, const AABB& b) 
{
	if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
	if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
	if (a.max.z < b.min.z || a.min.z > b.max.z) return false;

	return true;
}

// --------------------------------------------------------
// Sphere と AABB の判定
// --------------------------------------------------------
bool Collider::SphereVsAABB(const Sphere& sphere, const AABB& aabb) 
{
    // 球の中心座標を、AABBの箱の範囲内にクランプ(押し込む)して最近接点を出す
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

// --------------------------------------------------------
// カプセルとAABBの当たり判定
// --------------------------------------------------------
bool Collider::CapsuleVsAABB(const Capsule& capsule, const AABB& aabb) 
{
    float minY = std::min(capsule.p1.y, capsule.p2.y);
	float maxY = std::max(capsule.p1.y, capsule.p2.y);
	float aabbCenterY = (aabb.min.y + aabb.max.y) * 0.5f;

	float pY = Clamp(aabbCenterY, minY, maxY);

	MyVector3 p = { capsule.p1.x, pY, capsule.p1.z };

	return SphereVsAABB({ p.x, p.y, p.z, capsule.radius }, aabb);
}

// --------------------------------------------------------
// カプセルとSphereの当たり判定
// --------------------------------------------------------
bool Collider::CapsuleVsSphere(const Capsule& capsule, const Sphere& sphere) 
{
    // カプセルの中心線（p1からp2）上で、球の中心に最も近い点を探す
    MyVector3 closestPoint = GetClosestPointOnLineSegment(capsule.p1, capsule.p2, { sphere.x, sphere.y, sphere.z });

    return SphereCollider({ closestPoint.x, closestPoint.y, closestPoint.z, capsule.radius }, sphere);
}