#pragma once
#include "GameObject.h"
#include "Camera.h"

class Player
{
public:
	Player() = default;

	void Initialize(Model* model);

	void Update(float dt, float camYaw);

	void Draw(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer,
		const MyMatrix4x4& view, const MyMatrix4x4& projection);

	MyVector3 GetPosition() const { return m_object.transform.GetPosition(); }

	AABB GetAABB() const { return m_object.GetAABB(); }

	void SetPosition(const MyVector3& pos) {
		m_object.transform.SetPosition(pos.x, pos.y, pos.z);
		m_object.transform.UpdateMatrix();
	}

	GameObject& GetGameObject() { return m_object; }

private:
	GameObject m_object; // ÉvÉåÉCÉÑÅ[ÇÃé¿ëÃ
	float m_moveSpeed = 5.0f;
	float m_turnSpeed = 10.0f;
};

