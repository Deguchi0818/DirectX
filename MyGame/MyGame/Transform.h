#pragma once
#include "MyMatrix4x4.h"
#include<DirectXMath.h>

class Transform
{
public:
	Transform();

	void UpdateMatrix();

	const MyMatrix4x4& GetWorldMatrix() const { return m_worldMatrix; }

	void SetPosition(float x, float y, float z) { m_position = { x, y, z }; }
	void SetRotation(float x, float y, float z) { m_rotation = { x, y, z }; }
	void SetScale(float x, float y, float z) { m_scale = { x, y, z }; }

private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3  m_rotation;
	DirectX::XMFLOAT3 m_scale;

	MyMatrix4x4  m_worldMatrix;
};

