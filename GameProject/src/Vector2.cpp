#include "Vector2.h"

Vector2::Vector2() {
	this->x = 0;
	this->y = 0;
}

Vector2::Vector2(float x, float y) {
	this->x = x;
	this->y = y;
}

Vector2& Vector2::operator+(const Vector2 v) {
	Vector2 temp;
	temp.x = this->x + v.x;
	temp.y = this->y + v.y;
	return temp;
}

Vector2& Vector2::operator-(const Vector2 v) {
	Vector2 temp;
	temp.x = this->x - v.x;
	temp.y = this->y - v.y;
	return temp;
}

Vector2& Vector2::operator*(const Vector2 v) {
	Vector2 temp;
	temp.x = this->x * v.x;
	temp.y = this->y * v.y;
	return temp;
}

Vector2& Vector2::operator/(const Vector2 v) {
	Vector2 temp;
	temp.x = this->x / v.x;
	temp.y = this->y / v.y;
	return temp;
}

Vector2& Vector2::operator+=(const Vector2& v) {
	this->x += v.x;
	this->y += v.y;
	return *this;
}

Vector2& Vector2::operator-=(const Vector2& v) {
	this->x -= v.x;
	this->y -= v.y;
	return *this;
}

Vector2& Vector2::operator*=(const Vector2& v) {
	this->x *= v.x;
	this->y *= v.y;
	return *this;
}

Vector2& Vector2::operator/=(const Vector2& v) {
	this->x /= v.x;
	this->y /= v.y;
	return *this;
}

std::ostream& operator<<(std::ostream& out, const Vector2& curr) {
	out << "Vector2(" << curr.x << ", " << curr.y << ")";
	return out;
}