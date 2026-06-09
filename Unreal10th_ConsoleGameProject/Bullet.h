#pragma once
#include "GameObject.h"

class Bullet : public GameObject
{
public:
    Bullet();
    Bullet(int InX, int InY, int InDeltaX, int InDeltaY);

    virtual void Update() override;
    virtual void Update(int Gravity) override;
    virtual void OnCollisionEnter(GameObject* Other) override;

private:
    Direction Direction_ = Direction::None;
};
