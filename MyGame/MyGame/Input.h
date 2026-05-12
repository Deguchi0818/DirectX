#pragma once
#include <windows.h>
#include <vector>
#include <Xinput.h>

#pragma comment(lib, "xinput.lib")

class Input
{
public:
public:
    // 初期化
    static void Initialize();
    // 毎フレームの更新
    static void Update();

    static bool GetKey(int vKey);           // 押しっぱなし判定
    static bool GetKeyDown(int vKey);       // 押した瞬間判定
    static bool GetKeyUp(int vKey);         // 離した瞬間判定

    static bool GetButton(WORD button);     // ボタンの押しっぱなし
    static bool GetButtonDown(WORD button); // ボタンを押した瞬間
    static bool GetButtonUp(WORD button);   // ボタンを離した瞬間

    static float GetAxisX();    // 左スティックの水平移動量（-1.0 ～ 1.0）
    static float GetAxisZ();    // 左スティックの垂直移動量（-1.0 ～ 1.0）

	static float GetRightAxisX();    // 右スティックの水平移動量（-1.0 ～ 1.0）
	static float GetRightAxisZ();    // 右スティックの垂直移動量（-1.0 ～ 1.0）

    static bool IsControllerConnected() { return m_isConnected; }

private:
    static BYTE m_currentStates[256];  // 今のフレームの全キー状態
    static BYTE m_previousStates[256]; // 前のフレームの全キー状態

    static XINPUT_STATE m_currentControllerState;   // 現フレームのコントローラー状態
    static XINPUT_STATE m_previousControllerState;  // 前フレームのコントローラー状態
    static bool m_isConnected;  // コントローラーが接続されているか

};
