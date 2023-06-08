/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Math.hpp
Open source lines: 208/208 (100.00%)
*****************************************************/

#pragma once
#include "types.h"
#include <cmath>

inline float sead_sqrtf(float a) {
    float b;
    __asm__("VSQRT.F32 %0, %1"
             :"=t"(b)        
             :"t"(a)
             );
    return b;
}

struct Vector3 {
    float x;
    float y;
    float z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    inline void operator=(const Vector3& right)
    {
        this->x = right.x;
        this->y = right.y;
        this->z = right.z;
    }

    inline Vector3 operator+(const Vector3& right) const
    {
        Vector3 ret;
        ret.x = this->x + right.x;
        ret.y = this->y + right.y;
        ret.z = this->z + right.z;
        return ret;
    }

    inline void operator+=(const Vector3& right)
    {
        this->x += right.x;
        this->y += right.y;
        this->z += right.z;
    }

    inline Vector3 operator-(const Vector3& right) const
    {
        Vector3 ret;
        ret.x = this->x - right.x;
        ret.y = this->y - right.y;
        ret.z = this->z - right.z;
        return ret;
    }

    inline void operator-=(const Vector3& right)
    {
        this->x -= right.x;
        this->y -= right.y;
        this->z -= right.z;
    }

    inline Vector3 operator*(const float& amount) const 
    {
        Vector3 ret;
        ret.x = this->x * amount;
        ret.y = this->y * amount;
        ret.z = this->z * amount;
        return ret;
    }

    inline void operator*=(const float& amount)
    {
        this->x *= amount;
        this->y *= amount;
        this->z *= amount;
    }

    inline Vector3 operator/(const float& amount) const 
    {
        Vector3 ret;
        ret.x = this->x / amount;
        ret.y = this->y / amount;
        ret.z = this->z / amount;
        return ret;
    }

    inline void operator/=(const float& amount)
    {
        this->x /= amount;
        this->y /= amount;
        this->z /= amount;
    }

    inline float Dot(const Vector3& other) const {
        return this->x * other.x + this->y * other.y + this->z * other.z;
    }

    inline Vector3 Cross(const Vector3& other) {
        return Vector3(
            this->y * other.z - this->z * other.y,
            this->z * other.x - this->x * other.z,
            this->x * other.y - this->y * other.x
        );
    }

    inline void Lerp(const Vector3& other, float amount) {
        *this += (other - *this) * amount;
    }

    inline void Cerp(const Vector3& other, float amount) {
        Lerp(other, 1.f - (amount - 1.f) * (amount - 1.f));
    }

    inline void InvCerp(const Vector3& other, float amount) {
        Lerp(other, amount * amount);
    }

    inline float Magnitude() const {
        return sead_sqrtf(Dot(*this));
    }

    inline float MagNoSqrt() const {
        return Dot(*this);
    }

    inline bool Normalize() {
        float mag = Magnitude();
        if (mag != 0.f)
            *this /= mag;
        return mag != 0.f;
    }
};

struct Vector2 {
    float x;
    float y;

    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) {
        this->x = x;
        this->y = y;
    }
};

inline float DistanceNoSqrt(const Vector3& pos1, const Vector3& pos2) {
    float difX = pos2.x - pos1.x;
    float difY = pos2.y - pos1.y;
    float difZ = pos2.z - pos1.z;
    return difX*difX + difY*difY + difZ*difZ;
}

extern "C" {
    void sead_crossProduct(Vector3* out, Vector3* v0, Vector3* v1);
    void sead_normalize(Vector3* v0);
}

class Linear
{
private:
    float slope;
    float intercept;
public:
    inline Linear() : slope(0), intercept(0) {}
    inline Linear(const Vector2& point1, const Vector2& point2) {
        float dx = point2.x - point1.x;
        slope = (point2.y - point1.y) / dx;
        intercept = point1.y - (slope * point1.x);
    }
    inline float operator()(float x) {
        return slope * x + intercept;
    }
    inline float Slope() {
        return (slope > 0.f) - (slope < 0.f);
    }
};

struct Color4 {
    float R;
    float G;
    float B;
    float A;

    Color4(float r, float g, float b, float a) {
        R = r;
        G = g;
        B = b;
        A = a;
    }

    Color4(float r, float g, float b) {
        R = r;
        G = g;
        B = b;
        A = 1.f;
    }
};
