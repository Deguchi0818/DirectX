#include "GameInstance.h"
#include "MyMatrix4x4.h"
#include "Transform.h"
#include "Common.h"
#include "Mesh.h"
#include "GeometryGenerator.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")

using Microsoft::WRL::ComPtr;

GameInstance::GameInstance() {};
GameInstance::~GameInstance() { Finalize(); }

bool GameInstance::Initialize(HWND hWnd, int width, int height) 
{
    if (!m_graphics.Initialize(hWnd, width, height)) return false;

    auto device = m_graphics.GetDevice();

    if (!m_baseShader.Load(device, L"VertexShader.hlsl", L"PixelShader.hlsl")) return false;

    m_cubeModel.CreateCube(device, 1.0f, { 0.0f, 0.5f, 1.0f, 1.0f });
    m_planeModel.CreatePlane(device, 10.0f, 10.0f, { 0.0f, 0.5f, 0.0f, 1.0f });

    // 定数バッファ作成
    D3D11_BUFFER_DESC cbDesc{ .ByteWidth = sizeof(ConstantBufferData), .BindFlags = D3D11_BIND_CONSTANT_BUFFER };
    device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);

    // オブジェクトの配置
    m_gameObjects.clear();

    GameObject ground{ .pModel = &m_planeModel };
    m_gameObjects.push_back(ground);

    for (int j = 0; j < 5; j++) {
        GameObject cube{ .pModel = &m_cubeModel };
        cube.transform.SetPosition(j * 1.5f - 2.5f, 0.5f, -3);
        m_gameObjects.push_back(cube);
    }

    return true;
}

void GameInstance::Update() {
    // カメラの移動計算
    m_camera.Update();

    // オブジェクトの回転などの計算
    static float angle = 0;
    angle += 0.01f;
    for (auto& obj : m_gameObjects)
    {
        if (obj.pModel == &m_cubeModel) {
            obj.transform.SetRotation(0, angle, 0);
        }
    }
}

void GameInstance::Render()
{
    // 描画開始
    m_graphics.BeginScene(0.1f, 0.2f, 0.4f, 1.0f);

    auto context = m_graphics.GetContext();

    m_baseShader.Bind(context);

    // 行列の準備
    auto view = m_camera.GetViewMatrix();
    float aspect = 1280.0f / 720.0f;
    auto proj = m_camera.GetProjectionMatrix(aspect);

    // 全てのオブジェクトを描画するループ
    for (auto& obj : m_gameObjects)
    {
        obj.Draw(context, m_constantBuffer.Get(), view, proj);
    }

    //描画終了
    m_graphics.EndScene();
}

void GameInstance::Finalize() 
{

}