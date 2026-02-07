#include "Mesh.h"

void Mesh::Create(ID3D11Device* device, Vertex* vertices, int vCount, unsigned short* indices, int iCount) {
    m_indexCount = iCount;

    // 頂点バッファの作成
    D3D11_BUFFER_DESC vbd = {};
    vbd.ByteWidth = sizeof(Vertex) * vCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vData = { vertices };
    device->CreateBuffer(&vbd, &vData, &m_vertexBuffer);

    // インデックスバッファの作成
    D3D11_BUFFER_DESC ibd = {};
    ibd.ByteWidth = sizeof(unsigned short) * iCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA iData = { indices };
    device->CreateBuffer(&ibd, &iData, &m_indexBuffer);
}

void Mesh::Draw(ID3D11DeviceContext* context) {
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    // 自分のバッファをセットして描画
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    context->DrawIndexed(m_indexCount, 0, 0);
}