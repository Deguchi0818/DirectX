#include <windows.h>
#include "GameInstance.h"
#include "imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// ウィンドウメッセージを処理する関数
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    // ImGui にメッセージを渡す
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    // ウィンドウクラスの登録
    const wchar_t* CLASS_NAME = L"DX11_Window";
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX),
        CS_HREDRAW | CS_VREDRAW,
        WindowProc,
        0, 0, hInst, NULL,
        LoadCursor(NULL, IDC_ARROW),
        NULL, NULL, CLASS_NAME, NULL
    };
    RegisterClassEx(&wc);

    // ウィンドウの作成
    HWND hWnd = CreateWindow(
        CLASS_NAME,
        L"DirectX 11 Game Engine with ImGui",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
        NULL, NULL, hInst, NULL
    );

    if (!hWnd) return -1;

    ShowWindow(hWnd, nCmdShow);

    // ゲームエンジンの初期化
    GameInstance game;
    if (!game.Initialize(hWnd, 1280, 720)) {
        return -1;
    }

    // メインループ（ゲームループ）
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            game.Update();
            game.Render();
        }
    }
    game.Finalize();

    return (int)msg.wParam;
}