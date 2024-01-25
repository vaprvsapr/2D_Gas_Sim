#pragma once
#include <math.h>

struct Vec2D
{
	float x, y;

	float Squared() const
	{
		return x * x + y * y;
	}

	float Abs() const
	{
		return sqrt(x * x + y * y);
	}

	Vec2D Projection(const Vec2D& other) const
	{
		return other / other.Squared() * (x * other.x + y * other.y);
	}

	float operator*(const Vec2D& other) const
	{
		return x * other.x + y * other.y;
	}

	Vec2D operator*(const float alpha) const
	{
		return { alpha * x, alpha * y };
	}

	void operator*=(const float alpha)
	{
		x *= alpha;
		y *= alpha;
		return;
	}

	Vec2D operator/(const float alpha) const
	{
		return {x / alpha, y / alpha };
	}

	void operator/=(const float alpha)
	{
		x /= alpha;
		y /= alpha;
		return;
	}

	Vec2D operator+(const Vec2D& other) const
	{
		return { x + other.x, y + other.y };
	}

	void operator+=(const Vec2D& other)
	{
		x += other.x;
		y += other.y;
		return;
	}

	Vec2D operator-(const Vec2D& other) const
	{
		return { x - other.x, y - other.y };
	}

	void operator-=(const Vec2D& other)
	{
		x -= other.x;
		y -= other.y;
		return;
	}
};