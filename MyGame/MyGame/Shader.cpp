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
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                             D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

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
}