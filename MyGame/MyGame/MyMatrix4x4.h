#pragma once
class MyMatrix4x4
{
public:
	float m[4][4];

	MyMatrix4x4();

	static MyMatrix4x4 Multiply(const MyMatrix4x4& a, const MyMatrix4x4& b);

	static MyMatrix4x4 CreateTranslation(float x, float y, float z);
	static MyMatrix4x4 CreateScale(float x, float y, float z);

	static MyMatrix4x4 CreateRotationX(float angle); // ピッチ（上下の首振り）
	static MyMatrix4x4 CreateRotationY(float angle); // ヨー（左右の旋回）
	static MyMatrix4x4 CreateRotationZ(float angle); // ロール（プロペラ回転）

	static MyMatrix4x4 CreatePerspective(float fov, float aspect, float nearZ, float farZ);

	MyMatrix4x4 Transpose() const;
};

