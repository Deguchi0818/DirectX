#include "MyMatrix4x4.h"
#include <cmath>
#include <cstring>

MyMatrix4x4::MyMatrix4x4() 
{
	// ’PˆÊs—ñ‚Æ‚µ‚Ä‰Šú‰»
	memset(m, 0, sizeof(m));
	m[0][0] = 1.0f;
	m[1][1] = 1.0f;
	m[2][2] = 1.0f;
	m[3][3] = 1.0f;
}

MyMatrix4x4 MyMatrix4x4::Multiply(const MyMatrix4x4& a, const MyMatrix4x4& b) 
{
	MyMatrix4x4 res;
	for (int i = 0; i < 4; i++) 
	{
		for (int j = 0; j < 4; j++) 
		{
			float sum = 0;
			for (int k = 0; k < 4; k++) 
			{
				sum += a.m[i][k] * b.m[k][j];
			}
			res.m[i][j] = sum;
		}
	}
	return res;
}

MyMatrix4x4 MyMatrix4x4::CreateTranslation(float x, float y, float z) 
{
	MyMatrix4x4 res;
	res.m[3][0] = x;
	res.m[3][1] = y;
	res.m[3][2] = z;
	return res;
}

MyMatrix4x4 MyMatrix4x4::CreateScale(float x, float y, float z) 
{
	MyMatrix4x4 res;
	res.m[0][0] = x;
	res.m[1][1] = y;
	res.m[2][2] = z;
	return res;
}

MyMatrix4x4 MyMatrix4x4::CreateRotationX(float angle) 
{
	MyMatrix4x4 res;
	float s = sinf(angle);
	float c = cosf(angle);
	res.m[1][1] = c;
	res.m[1][2] = s;
	res.m[2][1] = -s;
	res.m[2][2] = c;
	return res;
}

MyMatrix4x4 MyMatrix4x4::CreateRotationY(float angle)
{
	MyMatrix4x4 res;
	float s = sinf(angle);
	float c = cosf(angle);
	res.m[0][0] = c;
	res.m[0][2] = -s;
	res.m[2][0] = s;
	res.m[2][2] = c;
	return res;
}

MyMatrix4x4 MyMatrix4x4::CreateRotationZ(float angle)
{
	MyMatrix4x4 res;
	float s = sinf(angle);
	float c = cosf(angle);
	res.m[0][0] = c;
	res.m[0][1] = s;
	res.m[1][0] = -s;
	res.m[1][1] = c;
	return res;
}

MyMatrix4x4 MyMatrix4x4::CreatePerspective(float fov, float aspect, float nearZ, float farZ) 
{
	MyMatrix4x4 res;
	for (int i = 0; i < 4; i++) 
	{
		for (int j = 0; j < 4; j++) 
		{
			res.m[i][j] = 0;
		}
	}

	float h = 1.0f / tanf(fov * 0.5f);
	float w = h / aspect;
	float a = farZ / (farZ - nearZ);
	float b = (-nearZ * farZ) / (farZ - nearZ);

	res.m[0][0] = w;
	res.m[1][1] = h;
	res.m[2][2] = a;
	res.m[2][3] = 1.0f;
	res.m[3][2] = b;
	return res;
}

MyMatrix4x4 MyMatrix4x4::Transpose() const 
{
	MyMatrix4x4 res;
	for (int i = 0; i < 4; i++) 
	{
		for (int j = 0; j < 4; j++) 
		{
			res.m[i][j] = m[j][i];
		}
	}
	return res;
}