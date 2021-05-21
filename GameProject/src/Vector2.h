#pragma once
#include <iostream>

struct Vector2 {
	float x;
	float y;

	Vector2();
	Vector2(float x, float y);

	Vector2& operator+(const Vector2 v);
	Vector2& operator-(const Vector2 v);
	Vector2& operator*(const Vector2 v);
	Vector2& operator/(const Vector2 v);

	Vector2& operator+=(const Vector2& v);
	Vector2& operator-=(const Vector2& v);
	Vector2& operator*=(const Vector2& v);
	Vector2& operator/=(const Vector2& v);

	friend std::ostream& operator<<(std::ostream& out, const Vector2& curr);
};