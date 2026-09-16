#include "Collider.h"
#include <cmath>
namespace {
    // 点をワールド行列で変換する(行ベクトル規約)
    MyVector3 TransformPoint(const MyVector3& p, const MyMatrix4x4& m)
    {
        return {
            p.x * m.m[0][0] + p.y * m.m[1][0] + p.z * m.m[2][0] + m.m[3][0],
            p.x * m.m[0][1] + p.y * m.m[1][1] + p.z * m.m[2][1] + m.m[3][1],
            p.x * m.m[0][2] + p.y * m.m[1][2] + p.z * m.m[2][2] + m.m[3][2]
        };
    }

    // ワールド行列からスケール成分を取り出す(各行の長さ)
    MyVector3 ExtractScale(const MyMatrix4x4& m)
    {
        auto len = [](const float r[4]) {
            return sqrtf(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
            };
        return { len(m.m[0]), len(m.m[1]), len(m.m[2]) };
    }
}

AABB ColliderComponent::GetWorldAABB(const MyMatrix4x4& world) const
{
    // 中心は行列で変換する(回転・親の影響もここに含まれる)
    MyVector3 center = TransformPoint(offset, world);

    // AABBは軸平行なので、サイズにはスケールのみを反映する
    MyVector3 ws = ExtractScale(world);
    MyVector3 half = {
        (scale.x * ws.x) * 0.5f,
        (scale.y * ws.y) * 0.5f,
        (scale.z * ws.z) * 0.5f
    };

    return {
        .min = { center.x - half.x, center.y - half.y, center.z - half.z },
        .max = { center.x + half.x, center.y + half.y, center.z + half.z }
    };
}

Sphere ColliderComponent::GetWorldSphere(const MyMatrix4x4& world) const
{
    MyVector3 center = TransformPoint(offset, world);
    MyVector3 ws = ExtractScale(world);

    return {
        .x = center.x,
        .y = center.y,
        .z = center.z,
        .radius = radius * ws.x
    };
}

Capsule ColliderComponent::GetWorldCapsule(const MyMatrix4x4& world) const
{
    // ローカル空間で上下の端点を作ってから、まとめて行列変換する
    MyVector3 localP1 = { offset.x, offset.y + height * 0.5f, offset.z };
    MyVector3 localP2 = { offset.x, offset.y - height * 0.5f, offset.z };

    MyVector3 ws = ExtractScale(world);

    return {
        .p1 = TransformPoint(localP1, world),
        .p2 = TransformPoint(localP2, world),
        .radius = radius * ws.x
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