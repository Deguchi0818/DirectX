#pragma once
#include <windows.h>
#include <d3d11.h>

#include "DebugRenderer.h"
#include "MyMatrix4x4.h"

class Scene;
class Camera;

class DebugUI
{
public:
    // ImGui と DebugRenderer の初期化
    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hWnd);
    void Finalize();

    // フレーム開始。以降 ImGui のウィンドウを積める
    void BeginFrame();

    // デバッグウィンドウの中身を組み立てる
    void BuildWindows(Scene& scene, Camera& camera, float deltaTime);

    // 積んだ ImGui の描画を実行する(オブジェクト描画の後に呼ぶ)
    void RenderImGui();

    // コライダーの線を描く(デバッグモード中のみ)
    void DrawColliders(ID3D11DeviceContext* context, Scene& scene,
        const MyMatrix4x4& view, const MyMatrix4x4& proj);

    void ToggleDebugMode() { m_isDebugMode = !m_isDebugMode; }
    bool IsDebugMode() const { return m_isDebugMode; }

private:
    DebugRenderer m_debugRenderer;
    bool m_isDebugMode = false;
};