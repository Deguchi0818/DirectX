#include "Mesh.h"

void Mesh::Create(ID3D11Device* device, Vertex* vArray, int vCount, unsigned int* iArray, int iCount) {
    m_vertexCount = vCount;
    m_indexCount = iCount;

    // 頂点バッファの作成
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = sizeof(Vertex) * vCount;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vData = {};
    vData.pSysMem = vArray;

    device->CreateBuffer(&vbDesc, &vData, &m_vertexBuffer);

    // インデックスバッファの作成
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = sizeof(unsigned int) * iCount;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA iData = { iArray };
    device->CreateBuffer(&ibDesc, &iData, &m_indexBuffer);
}

void Mesh::Draw(ID3D11DeviceContext* context) {
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    // 自分のバッファをセットして描画
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(m_indexCount, 0, 0);
}

void Mesh::BindBuffers(ID3D11DeviceContext* context) {
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    // 頂点バッファをセット
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

    // インデックスバッファをセット（32bit: DXGI_FORMAT_R32_UINT）
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // トポロジ（三角形リスト）をセット
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}