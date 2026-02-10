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

private:
	GameObject m_object; // ÉvÉåÉCÉÑÅ[ÇÃé¿ëÃ
	float m_moveSpeed = 5.0f;
	float m_turnSpeed = 10.0f;
};

