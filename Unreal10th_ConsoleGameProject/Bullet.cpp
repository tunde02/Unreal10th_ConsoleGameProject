#include "Bullet.h"
#include "GameEngine.h"

Bullet::Bullet()
{
}

Bullet::Bullet(int InX, int InY, int InDeltaX, int InDeltaY)
{
    Transform_.Position = Vector2{ InX, InY };
    Transform_.Delta = Vector2{ InDeltaX, InDeltaY };
    Transform_.Width = 1;
    Transform_.Height = 2;
    NextPosition_ = Transform_.Position;

    Collider_.Initialize(Transform_);
    CollisionLayer_ = CollisionLayer::Bullet;

    UpdatePeriod_ = 0.016f;

    RenderString_.reserve(Transform_.Width * Transform_.Height);
    //RenderString_.push_back(L"░");
    RenderString_.push_back(L"▒");
    RenderString_.push_back(L"█");
}

void Bullet::Update()
{
    UpdateTimer_ += GameEngine::Instance().GetFixedDeltaTime();
    if (UpdateTimer_ >= UpdatePeriod_)
    {
        UpdateTimer_ -= UpdatePeriod_;


        Transform_.Delta.Y = 1;
        NextPosition_ = Transform_.Position + Transform_.Delta;
    }
}

void Bullet::Update(int Gravity)
{
}

void Bullet::OnCollisionEnter(GameObject* Other)
{
}
