#include "Camera.h"
#include "Input.h"
#include <algorithm>
#include <Windows.h>

void Camera::Update(float dt) 
{

}

void Camera::Rotate(float dx, float dy) 
{
	m_yaw += dx * sensitivity;
	m_pitch += dy * sensitivity;

	m_yaw += Input::GetRightAxisX() * rightStickSensitivity; // 右スティックの水平入力でヨーを直接
	m_pitch += Input::GetRightAxisZ() * rightStickSensitivity; // 右スティックの垂直入力でピッチを直接

	// 真上や真下を見た時に画面がひっくり返らないように制限
	m_pitch = std::clamp(m_pitch, -1.5f, 1.5f);
}

MyMatrix4x4 Camera::GetViewMatrix() const 
{
	MyVector3 pos = m_transform.GetPosition();
	MyMatrix4x4 matTrans = MyMatrix4x4::CreateTranslation(-pos.x, -pos.y, -pos.z);

	MyMatrix4x4 matRotY = MyMatrix4x4::CreateRotationY(-m_yaw);
	MyMatrix4x4 matRotX = MyMatrix4x4::CreateRotationX(-m_pitch);
	MyMatrix4x4 matRot = MyMatrix4x4::Multiply(matRotY, matRotX);

	return MyMatrix4x4::Multiply(matTrans, matRot);
}

MyMatrix4x4 Camera::GetProjectionMatrix(float aspect) const
{
	constexpr float fov = std::numbers::pi_v<float> / 4.0f;

	return MyMatrix4x4::CreatePerspective(fov, aspect, 0.1f, 100.0f);
}

void Camera::UpdateTPS(const MyVector3& targetPos)
{
	float distance = 8.0f; // キャラクターとの距離
	float height = 1.5f;   // キャラクターの高さ

	// マウスの回転角(Yaw, Pitch)から、プレイヤーから見たカメラの相対位置を計算
	float dx = sinf(m_yaw) * cosf(m_pitch) * distance;
	float dy = sinf(m_pitch) * distance;
	float dz = cosf(m_yaw) * cosf(m_pitch) * distance;

	// カメラの新しい座標 = プレイヤーの座標 - (向きベクトル * 距離) + 高さ
	MyVector3 camPos;
	camPos.x = targetPos.x - dx;
	camPos.y = targetPos.y + height + dy;
	camPos.z = targetPos.z - dz;

	m_transform.SetPosition(camPos.x, camPos.y, camPos.z);
	m_transform.UpdateMatrix();
}