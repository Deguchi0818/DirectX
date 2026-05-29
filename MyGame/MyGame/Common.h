#pragma once
#define NOMINMAX
#include <DirectXMath.h>


// 頂点の構造
struct Vertex {
    float x, y, z;      // 12
    float nx, ny, nz;   // 12
    float u, v;         // 8
    float r, g, b, a;   // 16
    unsigned int BoneIndices[4]; // 16
    float BoneWeights[4];        // 16

};

struct MaterialData
{
    int useTexture; 
    float padding[3];
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

struct Capsule {
    MyVector3 p1;     // 上の半球の中心点
    MyVector3 p2;     // 下の半球の中心点
    float radius;     // 半径
};