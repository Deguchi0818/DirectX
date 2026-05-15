#include "DebugRenderer.h"
#include <cmath>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

struct DebugCB 
{
    DirectX::XMMATRIX viewProj;
};

void DebugRenderer::AddLine(const MyVector3& p1, const MyVector3& p2, const MyVector4& color)
{
    // 線分は2つの頂点で構成される
    m_vertices.push_back({ p1, color });
    m_vertices.push_back({ p2, color });
}

void DebugRenderer::AddAABB(const AABB& aabb, const MyVector4& color)
{
    // 箱の8つの角（頂点）を定義
    MyVector3 v[8] = {
        { aabb.min.x, aabb.min.y, aabb.min.z }, // 0
        { aabb.max.x, aabb.min.y, aabb.min.z }, // 1
        { aabb.max.x, aabb.max.y, aabb.min.z }, // 2
        { aabb.min.x, aabb.max.y, aabb.min.z }, // 3
        { aabb.min.x, aabb.min.y, aabb.max.z }, // 4
        { aabb.max.x, aabb.min.y, aabb.max.z }, // 5
        { aabb.max.x, aabb.max.y, aabb.max.z }, // 6
        { aabb.min.x, aabb.max.y, aabb.max.z }  // 7
    };

    // 底面の4本
    AddLine(v[0], v[1], color); AddLine(v[1], v[2], color);
    AddLine(v[2], v[3], color); AddLine(v[3], v[0], color);
    // 天面の4本
    AddLine(v[4], v[5], color); AddLine(v[5], v[6], color);
    AddLine(v[6], v[7], color); AddLine(v[7], v[4], color);
    // 縦の4本
    AddLine(v[0], v[4], color); AddLine(v[1], v[5], color);
    AddLine(v[2], v[6], color); AddLine(v[3], v[7], color);
}

void DebugRenderer::AddSphere(const Sphere& sphere, const MyVector4& color)
{
    // 球は X, Y, Z軸それぞれの平面上に「円」を描いて表現する
    const int segments = 16; // 円の滑らかさ（16角形）
    const float step = (3.14159265f * 2.0f) / segments;

    for (int i = 0; i < segments; ++i)
    {
        float theta1 = i * step;
        float theta2 = (i + 1) * step;

        // XY平面の円
        AddLine(
            { sphere.x + cosf(theta1) * sphere.radius, sphere.y + sinf(theta1) * sphere.radius, sphere.z },
            { sphere.x + cosf(theta2) * sphere.radius, sphere.y + sinf(theta2) * sphere.radius, sphere.z }, color);

        // XZ平面の円
        AddLine(
            { sphere.x + cosf(theta1) * sphere.radius, sphere.y, sphere.z + sinf(theta1) * sphere.radius },
            { sphere.x + cosf(theta2) * sphere.radius, sphere.y, sphere.z + sinf(theta2) * sphere.radius }, color);

        // YZ平面の円
        AddLine(
            { sphere.x, sphere.y + cosf(theta1) * sphere.radius, sphere.z + sinf(theta1) * sphere.radius },
            { sphere.x, sphere.y + cosf(theta2) * sphere.radius, sphere.z + sinf(theta2) * sphere.radius }, color);
    }
}

void DebugRenderer::AddCapsule(const Capsule& capsule, const MyVector4& color)
{
    // 上と下の半球（の代わりの球）を描画
    Sphere topSphere = { capsule.p1.x, capsule.p1.y, capsule.p1.z, capsule.radius };
    Sphere bottomSphere = { capsule.p2.x, capsule.p2.y, capsule.p2.z, capsule.radius };

    AddSphere(topSphere, color);
    AddSphere(bottomSphere, color);

    // 側面をつなぐ4本の縦線
    float r = capsule.radius;

    // X軸側の側面ライン
    AddLine({ capsule.p1.x + r, capsule.p1.y, capsule.p1.z }, { capsule.p2.x + r, capsule.p2.y, capsule.p2.z }, color);
    AddLine({ capsule.p1.x - r, capsule.p1.y, capsule.p1.z }, { capsule.p2.x - r, capsule.p2.y, capsule.p2.z }, color);

    // Z軸側の側面ライン
    AddLine({ capsule.p1.x, capsule.p1.y, capsule.p1.z + r }, { capsule.p2.x, capsule.p2.y, capsule.p2.z + r }, color);
    AddLine({ capsule.p1.x, capsule.p1.y, capsule.p1.z - r }, { capsule.p2.x, capsule.p2.y, capsule.p2.z - r }, color);
}

bool DebugRenderer::Initialize(ID3D11Device* device)
{
    HRESULT hr;

    // シェーダーのコンパイルと作成
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    hr = D3DCompileFromFile(L"DebugLine.hlsl", nullptr, nullptr, "VS", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) return false;

    hr = D3DCompileFromFile(L"DebugLine.hlsl", nullptr, nullptr, "PS", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) return false;

    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);

    // 入力レイアウトの作成 (Position と Color)
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

    // 頂点バッファの作成（動的更新可能: D3D11_USAGE_DYNAMIC）
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DYNAMIC;
    vbDesc.ByteWidth = sizeof(DebugVertex) * 10000; // 最大1万頂点（5000本ライン）
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPUから書き込み可能
    device->CreateBuffer(&vbDesc, nullptr, &m_vertexBuffer);

    // 4. 定数バッファの作成
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(DebugCB);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);

    return true;
}

void DebugRenderer::Render(ID3D11DeviceContext* context, const DirectX::XMMATRIX& viewProj)
{
    if (m_vertices.empty()) return;

    // 最大頂点数を超えないように制限
    size_t vertexCount = m_vertices.size();
    if (vertexCount > 10000) vertexCount = 10000;

    // 頂点バッファの内容を更新（Map / Unmap）
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
    {
        memcpy(mappedResource.pData, m_vertices.data(), sizeof(DebugVertex) * vertexCount);
        context->Unmap(m_vertexBuffer.Get(), 0);
    }

    // 定数バッファ（ViewProjection行列）の更新
    DebugCB cb;
    cb.viewProj = DirectX::XMMatrixTranspose(viewProj); // シェーダーに送るために転置
    context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

    // 描画パイプラインのセットアップ
    UINT stride = sizeof(DebugVertex);
    UINT offset = 0;
    context->IASetInputLayout(m_inputLayout.Get());
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

    // ポリゴンではなく線分リストとして描画する！
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // 描画実行
    context->Draw((UINT)vertexCount, 0);

    // 描画が終わったらリストを空にする（毎フレーム作り直すため）
    m_vertices.clear();

    // 他の描画に影響が出ないようにトポロジーを三角形に戻す
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DebugRenderer::Finalize()
{
    m_vertices.clear();
}