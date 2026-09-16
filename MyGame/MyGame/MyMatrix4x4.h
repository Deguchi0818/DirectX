#pragma once
#include <DirectXMath.h>
class MyMatrix4x4
{
public:
	float m[4][4];

	MyMatrix4x4();

	// 基本演算
	static MyMatrix4x4 Multiply(const MyMatrix4x4& a, const MyMatrix4x4& b);
	MyMatrix4x4 Transpose() const;

	// アフィン変換行列の生成
	static MyMatrix4x4 CreateTranslation(float x, float y, float z);
	static MyMatrix4x4 CreateScale(float x, float y, float z);
	static MyMatrix4x4 CreateRotationX(float angle); // ピッチ（上下の首振り）
	static MyMatrix4x4 CreateRotationY(float angle); // ヨー（左右の旋回）
	static MyMatrix4x4 CreateRotationZ(float angle); // ロール（プロペラ回転）

	// 投影変換行列の生成
	static MyMatrix4x4 CreatePerspective(float fov, float aspect, float nearZ, float farZ);

};


inline DirectX::XMMATRIX ToXM(const MyMatrix4x4& mat)
{
	return DirectX::XMLoadFloat4x4(
		reinterpret_cast<const DirectX::XMFLOAT4X4*>(mat.m));
}

inline MyMatrix4x4 FromXM(const DirectX::XMMATRIX& xm)
{
	MyMatrix4x4 out;
	DirectX::XMStoreFloat4x4(
		reinterpret_cast<DirectX::XMFLOAT4X4*>(out.m), xm);
	return out;
}
