#pragma once
#define NOMINMAX
#include <DirectXMath.h>


// 頂点の構造
struct Vertex {
    float x, y, z;    // 1. 座標 (12バイト)
    float r, g, b, a; // 2. 色 (16バイト) ★ここに移動
    float u, v;       // 3. UV (8バイト) ★ここに移動
    float nx, ny, nz; // 4. 法線 (12バイト) ★ここに移動
    unsigned int BoneIndices[4];
    float BoneWeights[4];

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

// 3次元ベクトル (座標・回転・スケール用)
struct MyVector3 {
    float x, y, z;
};

// 4次元ベクトル (色：R, G, B, A 用)
struct MyVector4 {
    float x, y, z, w; // または r, g, b, a
};

struct Sphere {
    float x, y, z;
    float radius;
};

struct AABB {
    MyVector3 min; // 左下奥の点
    MyVector3 max; // 右上前の点
};