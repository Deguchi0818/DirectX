#pragma once
#include "MyMatrix4x4.h"
#include<DirectXMath.h>
#include "Common.h"

class Transform
{
public:
	Transform();

	// ----------------------------------------------------
	// äKëw
	// ----------------------------------------------------
	void SetParent(Transform* parent) { m_parent = parent; }
	Transform* GetParent() const { return m_parent; }

	void UpdateMatrix();

	const MyMatrix4x4& GetLocalMatrix() const { return m_localMatrix; }
	const MyMatrix4x4& GetWorldMatrix() const { return m_worldMatrix; }

	void SetLocalPosition(float x, float y, float z) { m_localPosition = { x, y, z }; }
	void SetLocalRotation(float x, float y, float z) { m_localRotation = { x, y, z }; }
	void SetLocalScale(float x, float y, float z) { m_localScale = { x, y, z }; }

	MyVector3 GetLocalPosition() const { return m_localPosition; }
	MyVector3 GetLocalRotation() const { return m_localRotation; }
	MyVector3 GetLocalScale()    const { return m_localScale; }

	void SetPosition(float x, float y, float z) { SetLocalPosition(x, y, z); }
	void SetRotation(float x, float y, float z) { SetLocalRotation(x, y, z); }
	void SetScale(float x, float y, float z) { SetLocalScale(x, y, z); }

	MyVector3 GetPosition() const { return m_localPosition; }
	MyVector3 GetRotation() const { return m_localRotation; }
	MyVector3 GetScale()    const { return m_localScale; }

	MyVector3 GetWorldPosition() const;
	MyVector3 GetWorldScale()    const;

	void SetWorldMatrixDirect(const MyMatrix4x4& world) { m_worldMatrix = world; }

private:
	MyVector3 m_localPosition{ 0.0f, 0.0f, 0.0f };
	MyVector3 m_localRotation{ 0.0f, 0.0f, 0.0f };   // ìxêîñ@(degree)
	MyVector3 m_localScale{ 1.0f, 1.0f, 1.0f };

	MyMatrix4x4 m_localMatrix;
	MyMatrix4x4 m_worldMatrix;

	Transform* m_parent = nullptr;
};

