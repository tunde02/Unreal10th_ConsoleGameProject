#include "Player.h"
#include "GameEngine.h"
#include "Bullet.h"
#include <Windows.h>

Player::Player()
{
    Transform_.Position = Vector2{ 5, 5 };
    Transform_.Width = 3;
    Transform_.Height = 3;
    Collider_ = Collider(Transform_, CollisionLayer::Player);
    Hp = InitialHp;
    Damage = 1;
    Faction_ = Faction::Player;
    ShotDelay = PlayerShotDelay;

    RenderString_.push_back(L" ▲ ");
    RenderString_.push_back(L"║▇║");
    RenderString_.push_back(L"◢▇◣");
}

void Player::Update()
{
    Transform_.Delta.X = 0;
    Transform_.Delta.Y = 0;
    ShotDelay -= GameEngine::Instance().GetFixedDeltaTime();

    if (GetAsyncKeyState(VK_UP)) // ↑
    {
        Transform_.Delta.Y = -1;
    }
    else if (GetAsyncKeyState(VK_DOWN)) // ↓
    {
        Transform_.Delta.Y = 1;
    }

    if (GetAsyncKeyState(VK_LEFT)) // ←
    {
        Transform_.Delta.X = -1;
    }
    else if (GetAsyncKeyState(VK_RIGHT)) // →
    {
        Transform_.Delta.X = 1;
    }

    if (GetAsyncKeyState(VK_SPACE))
    {
        if (ShotDelay <= 0.0f)
        {
            ShotDelay = PlayerShotDelay;
            SpawnBullet();
        }
        /*
        Transform BulletTransform = Transform_;
        BulletTransform.Position.Y -= 2;
        BulletTransform.Width = 1;
        BulletTransform.Height = 2;
        //Bullet NewBullet = new Bullet()
        GameEngine::Instance().Instantiate(new Bullet(), BulletTransform, Vector2{ 0, -1 });
        */
    }

    NextPosition_ = Transform_.Position + Transform_.Delta;
}

void Player::OnCollisionEnter(GameObject* Other)
{
}

void Player::OnCollisionExit(GameObject* Other)
{
}

void Player::SpawnBullet() const
{
    // TODO:
    // 플레이어의 위치, 크기와 총알의 프리셋에 따른 크기에 따라 생성 위치 조절
    Transform BulletTransform = Transform_;
    BulletTransform.Position.X += 1;
    BulletTransform.Position.Y -= 2;
    BulletTransform.Width = 1;
    BulletTransform.Height = 2;

    Vector2 BulletDelta{ 0, -1 };

    GameEngine::Instance().Instantiate(new Bullet(Faction_, CurrentBulletType), BulletTransform, BulletDelta);
}
