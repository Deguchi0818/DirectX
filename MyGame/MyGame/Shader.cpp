#include "Shader.h"
#include <d3dcompiler.h>
#include <vector>

#pragma comment(lib, "d3dcompiler.lib")

bool Shader::Load(ID3D11Device* device, const wchar_t* vsFile, const wchar_t* psFile) {
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

    // 頂点シェーダーのコンパイル
    HRESULT hr = D3DCompileFromFile(vsFile, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        return false;
    }
    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);

    // 入力レイアウトの作成
    D3D11_INPUT_ELEMENT_DESC layout[] = {
     { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
     { "COLOR",        0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
     { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
     { "BLENDWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);


    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;              // 毎フレーム更新するので DYNAMIC
    bd.ByteWidth = sizeof(DirectX::XMMATRIX) * 128; // 行列128個分
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // ピクセルシェーダーのコンパイル
    hr = D3DCompileFromFile(psFile, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) return false;

    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);

    return true;
}

void Shader::Bind(ID3D11DeviceContext* context) {
    context->IASetInputLayout(m_inputLayout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(1, 1, m_boneBuffer.GetAddressOf());
}

void Shader::UpdateBones(ID3D11DeviceContext* context, const std::vector<DirectX::XMMATRIX>& matrices) {
    if (matrices.empty()) return;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    // GPUのバッファを書き込みモードで開く
    HRESULT hr = context->Map(m_boneBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr)) {
        BoneBuffer* dataPtr = (BoneBuffer*)mappedResource.pData;

        // 最大128個までコピーする
        size_t count = min(matrices.size(), 128);
        for (size_t i = 0; i < count; i++) {
            // シェーダー側が行列を正しく計算できるように「転置(Transpose)」して送るのが一般的です
            dataPtr->mBoneMatrices[i] = DirectX::XMMatrixTranspose(matrices[i]);
        }

        context->Unmap(m_boneBuffer.Get(), 0); // 書き終わったら閉じる
    }
}