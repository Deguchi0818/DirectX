#pragma once
#include <DirectXMath.h>

// 頂点の構造
struct Vertex {
    float x, y, z;
    float r, g, b, a;
};

// 定数バッファの構造
struct ConstantBufferData {
    DirectX::XMMATRIX wvp;
};

struct ConstantBuffer
{
    DirectX::XMMATRIX worldViewProj; // 3つの行列を掛け合わせたもの
};

struct MyMatrix
{
    float m[4][4];
};
