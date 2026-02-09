#pragma once
#include "Mesh.h"
#include "Common.h"

class Model {
public:
    // 立方体モデルを作成
    void CreateCube(ID3D11Device* device, float size, const MyVector4& color);

    // 平面モデルを作成
    void CreatePlane(ID3D11Device* device, float width, float depth, const MyVector4& color);

    // メッシュを描画する
    void Draw(ID3D11DeviceContext* context);

private:
    Mesh m_mesh;
};