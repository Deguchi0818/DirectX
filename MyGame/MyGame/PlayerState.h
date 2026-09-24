#pragma once
#include <string>

class Player;

class PlayerStateBase
{
public:
    virtual ~PlayerStateBase() = default;

    // その状態に入った瞬間に1回だけ呼ばれる
    virtual void OnEnter(Player& player) {}

    // 毎フレーム呼ばれる。ここで遷移を判断する
    virtual void Update(Player& player, float dt) {}

    // その状態から抜ける瞬間に1回だけ呼ばれる
    virtual void OnExit(Player& player) {}

    // この状態で再生するアニメーション名
    virtual std::string GetAnimName() const = 0;
};

// 待機
class IdleState : public PlayerStateBase
{
public:
    void Update(Player& player, float dt) override;
    std::string GetAnimName() const override { return "Idle"; }
};

// 移動
class MoveState : public PlayerStateBase
{
public:
    void Update(Player& player, float dt) override;
    std::string GetAnimName() const override { return "Running"; }
};

// 空中
class JumpState : public PlayerStateBase
{
public:
    void Update(Player& player, float dt) override;
    std::string GetAnimName() const override { return "Jump"; }
};

// 近接攻撃
class AttackState : public PlayerStateBase
{
public:
    void OnEnter(Player& player) override;
    void Update(Player& player, float dt) override;
    std::string GetAnimName() const override { return "Attack"; }

private:
    float m_timer = 0.0f;
};

// クイックブースト
class QuickBoostState : public PlayerStateBase
{
public:
    void OnEnter(Player& player) override;
    void Update(Player& player, float dt) override;
    void OnExit(Player& player) override;
    std::string GetAnimName() const override { return "Idle"; }  // 専用モーションがないので仮

private:
    float m_timer = 0.0f;
};