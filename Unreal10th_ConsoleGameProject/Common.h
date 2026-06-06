#pragma once

struct Vector2
{
    int X = 0;
    int Y = 0;

    Vector2() = default;
    Vector2(int InX, int InY) : X(InX), Y(InY) {}

    Vector2 operator+(const Vector2& other) const;
    Vector2 operator-(const Vector2& other) const;
    Vector2& operator=(const Vector2& other);
};

struct Transform
{
    Vector2 Position{};
    Vector2 Delta{};
    size_t Width = 0;
    size_t Height = 0;

    Transform() = default;
};
