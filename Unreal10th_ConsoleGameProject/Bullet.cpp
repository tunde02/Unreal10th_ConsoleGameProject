#include "Bullet.h"
#include "GameEngine.h"

Bullet::Bullet()
{
    NextPosition_ = Transform_.Position;
    Collider_ = Collider(Transform_, CollisionLayer::Bullet);
    Hp = 1;
    Damage = 1;

    UpdatePeriod_ = 0.04f;

    //RenderString_.reserve(Transform_.Width * Transform_.Height);
    //for (int i = 0; i < Transform_.Height; i++)
    //{
    //    std::wstring Str{};
    //    for (int j = 0; j < Transform_.Width; j++)
    //    {
    //        Str += L"█";
    //    }
    //    RenderString_.push_back(Str);
    //}
    //RenderString_.push_back(L"▒");
    //RenderString_.push_back(L"█");
}

Bullet::Bullet(int InX, int InY, int InDeltaX, int InDeltaY)
{
    Transform_.Position = Vector2{ InX, InY };
    //Transform_.Delta = Vector2{ InDeltaX, InDeltaY };
    Delta_ = Vector2{ InDeltaX, InDeltaY };
    Transform_.Delta = Delta_;
    Transform_.Width = 2;
    Transform_.Height = 2;
    NextPosition_ = Transform_.Position;
    Collider_ = Collider(Transform_, CollisionLayer::Bullet);

    UpdatePeriod_ = 0.04f;

    RenderString_.reserve(Transform_.Width * Transform_.Height);
    //RenderString_.push_back(L"░");
    RenderString_.push_back(L"▒ ");
    RenderString_.push_back(L" █");
}

Bullet::Bullet(const Transform& InTransform, const Vector2 InDelta, const Faction InFaction, int InHp)
{
    Transform_ = InTransform;
    Delta_ = InDelta;
    NextPosition_ = Transform_.Position;
    Collider_ = Collider(Transform_, CollisionLayer::Bullet);
    Hp = InHp;
    Damage = 2;

    UpdatePeriod_ = 0.04f;

    RenderString_.reserve(Transform_.Width * Transform_.Height);
    //RenderString_.push_back(L"░");
    RenderString_.push_back(L"▒");
    RenderString_.push_back(L"█");

    Faction_ = InFaction;
}

void Bullet::Initialize(const Transform InTransform, const Vector2 InDelta)
{
    GameObject::Initialize(InTransform, InDelta);
    RenderString_.reserve(Transform_.Width * Transform_.Height);
    for (int i = 0; i < Transform_.Height; i++)
    {
        std::wstring Str{};
        for (int j = 0; j < Transform_.Width; j++)
        {
            Str += L"█";
        }
        RenderString_.push_back(Str);
    }
}

void Bullet::Update()
{
    UpdateTimer_ += GameEngine::Instance().GetFixedDeltaTime();
    if (UpdateTimer_ >= UpdatePeriod_)
    {
        UpdateTimer_ -= UpdatePeriod_;


        Transform_.Delta = Delta_;
        NextPosition_ = Transform_.Position + Transform_.Delta;
    }
}

void Bullet::Update(int Gravity)
{
}

void Bullet::OnCollisionEnter(GameObject* Other)
{
    if (Other == nullptr || (Other->GetCollisionLayer() == CollisionLayer::Bullet))
    {
        return;
    }
    else if (Other->GetCollisionLayer() == CollisionLayer::Wall)
    {
        Destroy();
    }
    else if (Other->GetFaction() != Faction_)
    {
        Other->TakeDamage(Damage);
        TakeDamage(1);
    }
}
