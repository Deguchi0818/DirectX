#include "Camera.h"
#include <Windows.h>

void Camera::Update() 
{
	float speed = 0.05f;
	auto pos = m_transform.GetPosition();

	if (GetAsyncKeyState('S') & 0x8000) pos.z -= speed;
	if (GetAsyncKeyState('W') & 0x8000) pos.z += speed;
	if (GetAsyncKeyState('D') & 0x8000) pos.x += speed;
	if (GetAsyncKeyState('A') & 0x8000) pos.x -= speed;

	m_transform.SetPosition(pos.x, pos.y, pos.z);
	m_transform.UpdateMatrix();
}

MyMatrix4x4 Camera::GetViewMatrix() const 
{
	MyMatrix4x4 worud = m_transform.GetWorldMatrix();

	auto pos = m_transform.GetPosition();
	auto rot = m_transform.GetRotation();

	MyMatrix4x4 invTrans = MyMatrix4x4::CreateTranslation(-pos.x, -pos.y, -pos.z);
	MyMatrix4x4 invRot = MyMatrix4x4::CreateRotationX(-rot.x);

	return MyMatrix4x4::Multiply(invTrans, invRot);
}

MyMatrix4x4 Camera::GetProjectionMatrix(float aspect) const
{
	constexpr float fov = std::numbers::pi_v<float> / 4.0f;

	return MyMatrix4x4::CreatePerspective(fov, aspect, 0.1f, 100.0f);
}