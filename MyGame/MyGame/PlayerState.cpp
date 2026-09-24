#include "PlayerState.h"
#include "Player.h"

void IdleState::Update(Player& player, float dt)
{
    if (player.IsQuickBoostInput())
    {
        player.ChangeToQuickBoost();
    }

    else if (player.IsAttackInput())
    {
        player.ChangeToAttack();
    }

    else if (player.HasMoveInput())
    {
        player.ChangeToMove();
    }


}

void MoveState::Update(Player& player, float dt)
{
    if (player.IsQuickBoostInput())
    {
        player.ChangeToQuickBoost();
    }

    else if (player.IsAttackInput())
    {
        player.ChangeToAttack();
    }

    else if (!player.HasMoveInput())
    {
        player.ChangeToIdle();
    }

}

void JumpState::Update(Player& player, float dt)
{
    if (player.IsQuickBoostInput())
    {
        player.ChangeToQuickBoost();
    }

    else if (player.IsAttackInput())
    {
        player.ChangeToAttack();
    }

    else if (player.IsGrounded())
    {
        if (player.HasMoveInput())
        {
            player.ChangeToMove();
        }

        else
        {
            player.ChangeToIdle();
        }
    }
}

void AttackState::OnEnter(Player& player)
{
    m_timer = 0.0f;
}

void AttackState::Update(Player& player, float dt)
{
    m_timer += dt;


    if (m_timer >= player.GetAttackDuration())
    {
        player.ChangeToGroundOrAir();
    }
}

void QuickBoostState::OnEnter(Player& player) 
{
    m_timer = 0.0f;
    MyVector3 vel = player.GetVelocity();
    vel.x = player.GetMoveDirX() * player.GetBoostSpeed();
    vel.z = player.GetMoveDirZ() * player.GetBoostSpeed();
    player.SetSpeedLimit(player.GetBoostSpeed());
    player.SetVelocity(vel);
}
void QuickBoostState::Update(Player& player, float dt) 
{
    m_timer += dt;

    if (m_timer > player.GetDuration()) 
    {
        player.ChangeToGroundOrAir();
    }
}
void QuickBoostState::OnExit(Player& player) 
{
    player.ResetSpeedLimit();
}