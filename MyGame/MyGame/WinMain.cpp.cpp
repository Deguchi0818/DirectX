#include <windows.h>
#include "GameInstance.h"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) 
{
    const wchar_t* CLASS_NAME = L"DX11_Window";
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WindowProc, 0, 0, hInst, NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, CLASS_NAME, NULL };
    RegisterClassEx(&wc);

    HWND hWnd = CreateWindow(CLASS_NAME, L"DirectX 11 Test", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, hInst, NULL);
    ShowWindow(hWnd, nCmdShow);

    GameInstance game;
    if (!game.Initialize(hWnd, 1280, 720)) return -1;

    // ÉÅÉCÉìÉãÅ[Év
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else 
        {
            game.Render(); 
        }
    }

    return (int)msg.wParam;
}