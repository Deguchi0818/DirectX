#pragma once
#include "GameObject.h"
class Coin : public GameObject
{
	bool isCollected = false;

	virtual void OnTriggerEnter(GameObject* other) override 
	{
		isCollected = true;
	}
};

