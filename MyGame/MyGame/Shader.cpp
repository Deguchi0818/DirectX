#include "Shader.h"
#include <d3dcompiler.h>
#include <vector>
#include "Common.h"

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
     { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
     { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // 12から
     { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // 24から
     { "COLOR",        0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // 32から
     { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // 48から
     { "BLENDWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // 64から
    };
    device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

    D3D11_BUFFER_DESC bbd = {};
    bbd.ByteWidth = sizeof(BoneBuffer);
    bbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bbd.Usage = D3D11_USAGE_DYNAMIC;

    device->CreateBuffer(&bbd, nullptr, &m_boneBuffer);

    D3D11_SAMPLER_DESC sd = {};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 線形補間
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;    // 繰り返しOK
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd.MinLOD = 0;
    sd.MaxLOD = D3D11_FLOAT32_MAX;

    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable = TRUE;               // 深度テストを有効にする
    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsd.DepthFunc = D3D11_COMPARISON_LESS;

    D3D11_BUFFER_DESC mbd = {};
    mbd.ByteWidth = sizeof(MaterialData);
    mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    mbd.Usage = D3D11_USAGE_DYNAMIC;
    mbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device->CreateBuffer(&mbd, nullptr, &m_materialBuffer);

    hr = device->CreateDepthStencilState(&dsd, &m_depthStencilState);
    if (FAILED(hr)) return false;

    // サンプラーを作成
    hr = device->CreateSamplerState(&sd, &m_samplerState);
    if (FAILED(hr)) return false;

    // ピクセルシェーダーのコンパイル
    hr = D3DCompileFromFile(psFile, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) return false;

    hr = device->CreateBuffer(&mbd, nullptr, &m_materialBuffer);
    if (FAILED(hr)) return false;

    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);

    return true;
}

void Shader::Bind(ID3D11DeviceContext* context) {
    context->IASetInputLayout(m_inputLayout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(1, 1, m_boneBuffer.GetAddressOf());
    context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    context->OMSetDepthStencilState(m_depthStencilState.Get(), 1);
}
void Shader::UpdateBones(ID3D11DeviceContext* context, const std::vector<DirectX::XMMATRIX>& matrices) {
    if (matrices.empty()) return;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    // GPUのバッファを書き込みモードで開く
    HRESULT hr = context->Map(m_boneBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr)) {
        BoneBuffer* dataPtr = (BoneBuffer*)mappedResource.pData;

        size_t count = min(matrices.size(), 256);
        for (size_t i = 0; i < count; i++) {
            dataPtr->mBoneMatrices[i] = matrices[i];
        }

        context->Unmap(m_boneBuffer.Get(), 0); // 書き終わったら閉じる
    }
}

void Shader::SetMaterial(ID3D11DeviceContext* context, bool useTex) {
    D3D11_MAPPED_SUBRESOURCE ms;
    if (SUCCEEDED(context->Map(m_materialBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms))) {
        MaterialData* data = (MaterialData*)ms.pData;
        data->useTexture = useTex ? 1 : 0;
        context->Unmap(m_materialBuffer.Get(), 0);
    }
    // ピクセルシェーダーのレジスタ b2 にセット
    context->PSSetConstantBuffers(2, 1, m_materialBuffer.GetAddressOf());
}