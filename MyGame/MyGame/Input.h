#pragma once
#include <windows.h>
#include <vector>
class Input
{
public:
public:
    // 初期化
    static void Initialize();

    // 毎フレームの更新
    static void Update();

    // 押しっぱなし判定
    static bool GetKey(int vKey);

    // 押した瞬間判定
    static bool GetKeyDown(int vKey);

    // 離した瞬間判定
    static bool GetKeyUp(int vKey);
private:
    static BYTE m_currentStates[256];  // 今のフレームの全キー状態
    static BYTE m_previousStates[256]; // 前のフレームの全キー状態
};
