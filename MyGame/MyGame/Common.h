#pragma once
#define NOMINMAX
#include <DirectXMath.h>


// 頂点の構造
struct Vertex {
    float x, y, z;
    float r, g, b, a;
    // テクスチャの座標 (UV座標)
    float u, v;
    float nx, ny, nz;
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