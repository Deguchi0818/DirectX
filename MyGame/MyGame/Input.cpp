#include "Input.h"

// 静的メンバの実体定義
BYTE Input::m_currentStates[256] = { 0 };
BYTE Input::m_previousStates[256] = { 0 };

void Input::Initialize() {
    memset(m_currentStates, 0, sizeof(m_currentStates));
    memset(m_previousStates, 0, sizeof(m_previousStates));
}

void Input::Update() {
    // 前の状態を保存
    memcpy(m_previousStates, m_currentStates, sizeof(m_previousStates));

    // 今の状態を取得
    GetKeyboardState(m_currentStates);
}

bool Input::GetKey(int vKey) {
    // 最上位ビットが1なら押されている
    return (m_currentStates[vKey] & 0x80) != 0;
}

bool Input::GetKeyDown(int vKey) {
    // 今押されていて、前は押されていなかったら「押した瞬間」
    return ((m_currentStates[vKey] & 0x80) != 0) && ((m_previousStates[vKey] & 0x80) == 0);
}

bool Input::GetKeyUp(int vKey) {
    // 今押されていなくて、前は押されていたら「離した瞬間」
    return ((m_currentStates[vKey] & 0x80) == 0) && ((m_previousStates[vKey] & 0x80) != 0);
}