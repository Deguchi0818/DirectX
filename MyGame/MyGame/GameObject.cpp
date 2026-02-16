#include "GameObject.h"

AABB GameObject::GetAABB() const
{
    MyVector3 pos = transform.GetPosition();
    MyVector3 scale = transform.GetScale();

    float halfX = 0.5f * scale.x;
    float halfY = 0.5f * scale.y;
    float halfZ = 0.5f * scale.z;

    return 
    {
        .min = { pos.x - halfX, pos.y - halfY, pos.z - halfZ },
        .max = { pos.x + halfX, pos.y + halfY, pos.z + halfZ }
    };
}