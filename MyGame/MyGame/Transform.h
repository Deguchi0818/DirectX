#pragma once
#include "MyMatrix4x4.h"
#include<DirectXMath.h>
#include "Common.h"

class Transform
{
public:
	Transform();

	void UpdateMatrix();

	const MyMatrix4x4& GetWorldMatrix() const { return m_worldMatrix; }

	void SetPosition(float x, float y, float z) { m_position = { x, y, z }; }
	void SetRotation(float x, float y, float z) { m_rotation = { x, y, z }; }
	void SetScale(float x, float y, float z) { m_scale = { x, y, z }; }

	MyVector3 GetPosition() const { return m_position; }
	MyVector3 GetRotation() const { return m_rotation; }
	MyVector3 GetScale() const { return m_scale; }

private:
	MyVector3 m_position{ 0.0f, 0.0f, 0.0f };
	MyVector3 m_rotation{ 0.0f, 0.0f, 0.0f };
	MyVector3 m_scale{ 1.0f, 1.0f, 1.0f };

	MyMatrix4x4  m_worldMatrix;
};

