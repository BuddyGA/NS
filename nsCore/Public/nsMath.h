#pragma once

#include "nsPlatform.h"
#include <cmath>

#define NS_MATH_SIMD		1

#if NS_MATH_SIMD
#include <immintrin.h>
#endif // NS_MATH_SIMD


#define NS_MATH_TAU				(6.283185307f)
#define NS_MATH_PI				(3.141592653f)
#define NS_MATH_PI_2			(1.570796326f)
#define NS_MATH_PI_4			(0.785398163f)
#define NS_MATH_EPS				(0.000001f)
#define NS_MATH_EPS_LOW_P		(0.0001f)



namespace nsMath
{
	NS_NODISCARD_INLINE bool FloatEquals(float a, float b, float epsilon = NS_MATH_EPS) noexcept
	{
		return fabsf(a - b) <= epsilon;
	}


	NS_NODISCARD_INLINE bool FloatGreaterThanEquals(float a, float b, float epsilon = NS_MATH_EPS) noexcept
	{
		return a > b || FloatEquals(a, b, epsilon);
	}


	NS_NODISCARD_INLINE bool FloatLessThanEquals(float a, float b, float epsilon = NS_MATH_EPS) noexcept
	{
		return a < b || FloatEquals(a, b, epsilon);
	}


	NS_NODISCARD_INLINE bool IsZero(float value) noexcept
	{
		return FloatEquals(value, 0.0f, NS_MATH_EPS);
	}


	NS_NODISCARD_INLINE bool IsNaN(float value) noexcept
	{
		return isnan(value);
	}


	NS_NODISCARD_INLINE bool IsInfinity(float value) noexcept
	{
		return isinf(value);
	}


	NS_NODISCARD_INLINE float Abs(float value) noexcept
	{
		return fabsf(value);
	}


	NS_NODISCARD_INLINE float Sqrt(float value) noexcept
	{
		return sqrtf(value);
	}


	NS_NODISCARD_INLINE float ModF(float a, float b) noexcept
	{
		return fmodf(a, b);
	}


	NS_NODISCARD_INLINE float DegToRad(float degree) noexcept
	{
		return degree * NS_MATH_PI / 180.0f;
	}


	NS_NODISCARD_INLINE float RadToDeg(float radian) noexcept
	{
		return radian * 180.0f / NS_MATH_PI;
	}


	NS_NODISCARD_INLINE float Sin(float radian) noexcept
	{
		return sinf(radian);
	}


	NS_NODISCARD_INLINE float ASin(float v) noexcept
	{
		return asinf(v);
	}


	NS_NODISCARD_INLINE float Cos(float radian) noexcept
	{
		return cosf(radian);
	}


	NS_NODISCARD_INLINE float ACos(float v) noexcept
	{
		return acosf(v);
	}


	NS_NODISCARD_INLINE float Tan(float radian) noexcept
	{
		return tanf(radian);
	}


	NS_NODISCARD_INLINE float ATan(float v) noexcept
	{
		return atanf(v);
	}


	NS_NODISCARD_INLINE float ATan2(float x, float y) noexcept
	{
		return atan2f(y, x);
	}


	NS_NODISCARD_INLINE float NormalizeInRange(float value, float minValue, float maxValue, float minNormalize, float maxNormalize) noexcept
	{
		return (maxNormalize - minNormalize) * (value - minValue) / (maxValue - minValue) + minNormalize;
	}


	NS_NODISCARD_INLINE float NormalizeZeroToOne(float value, float minValue, float maxValue) noexcept
	{
		return NormalizeInRange(value, minValue, maxValue, 0.0f, 1.0f);
	}


	NS_NODISCARD_INLINE float NormalizeMinusOneToOne(float value, float minValue, float maxValue) noexcept
	{
		return NormalizeInRange(value, minValue, maxValue, -1.0f, 1.0f);
	}


	NS_NODISCARD_INLINE int RandomInRange(int minInclusive, int maxInclusive) noexcept
	{
		if (minInclusive == 0 && maxInclusive == 0)
		{
			return 0;
		}

		return minInclusive + (rand() % (maxInclusive - minInclusive + 1));
	}


	NS_NODISCARD_INLINE float RandomInRange(float minInclusive, float maxInclusive) noexcept
	{
		return minInclusive + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (maxInclusive - minInclusive));
	}


	NS_NODISCARD_INLINE float Lerp(float a, float b, float t) noexcept
	{
		return a + (b - a) * t;
	}


	NS_NODISCARD_INLINE float ClampDegree(float degree) noexcept
	{
		degree = ModF(degree + 180.0f, 360.0f);

		if (degree < 0.0f)
		{
			degree += 360.0f;
		}

		return degree - 180.0f;
	}


	template<typename T>
	NS_NODISCARD_INLINE T Clamp(T value, T minValue, T maxValue) noexcept
	{
		if (value < minValue) return minValue;
		if (value > maxValue) return maxValue;

		return value;
	}


	template<typename T>
	NS_NODISCARD_INLINE T Min(T a, T b) noexcept
	{
		return a < b ? a : b;
	}


	template<typename T>
	NS_NODISCARD_INLINE T Max(T a, T b) noexcept
	{
		return a > b ? a : b;
	}


	template<typename T>
	NS_NODISCARD_INLINE int Sign(T value) noexcept
	{
		return (0 < value) - (value < 0);
	}


	template<typename T>
	NS_NODISCARD_INLINE T Floor(T value) noexcept
	{
		return floor(value);
	}


	template<typename T>
	NS_NODISCARD_INLINE T Ceil(T value) noexcept
	{
		return ceil(value);
	}

};




// =============================================================================================================================================================== //
// COLOR - RGBA 8 bits per channel
// =============================================================================================================================================================== //
class nsColor
{
public:
	uint8 R;
	uint8 G;
	uint8 B;
	uint8 A;

	static NS_CORE_API const nsColor BLACK;
	static NS_CORE_API const nsColor BLACK_TRANSPARENT;
	static NS_CORE_API const nsColor BLUE;
	static NS_CORE_API const nsColor CYAN;
	static NS_CORE_API const nsColor GRAY;
	static NS_CORE_API const nsColor GREEN;
	static NS_CORE_API const nsColor RED;
	static NS_CORE_API const nsColor YELLOW;
	static NS_CORE_API const nsColor WHITE;
	static NS_CORE_API const nsColor WHITE_TRANSPARENT;

public:
	nsColor() noexcept
		: R(0)
		, G(0)
		, B(0)
		, A(0)
	{
	}

	nsColor(uint8 r, uint8 g, uint8 b, uint8 a = 255) noexcept
		: R(r)
		, G(g)
		, B(b)
		, A(a)
	{
	}

	nsColor(uint32 rgba) noexcept
	{
		R = (rgba & 0xFF000000) >> 24;
		G = (rgba & 0x00FF0000) >> 16;
		B = (rgba & 0x0000FF00) >> 8;
		A = (rgba & 0x000000FF);
	}

	NS_INLINE nsColor operator+(uint8 value) noexcept
	{
		const uint8 s = nsMath::Clamp<uint8>(value, 0, 255);
		return nsColor(R + s, G + s, B + s, A + s);
	}

	NS_INLINE nsColor operator-(uint8 value) noexcept
	{
		const uint8 s = nsMath::Clamp<uint8>(value, 0, 255);
		return nsColor(R - s, G - s, B - s, A - s);
	}

	NS_INLINE bool operator==(const nsColor& rhs) noexcept
	{
		return (R == rhs.R) && (G == rhs.G) && (B == rhs.B) && (A == rhs.A);
	}

	NS_INLINE bool operator!=(const nsColor& rhs) noexcept
	{
		return !(*this == rhs);
	}

};



// =============================================================================================================================================================== //
// LINEAR COLOR - RGBA 32 bits per channel
// =============================================================================================================================================================== //
class nsLinearColor
{
public:
	float R;
	float G;
	float B;
	float A;

	static NS_CORE_API const nsLinearColor BLACK;
	static NS_CORE_API const nsLinearColor BLACK_TRANSPARENT;
	static NS_CORE_API const nsLinearColor BLUE;
	static NS_CORE_API const nsLinearColor CYAN;
	static NS_CORE_API const nsLinearColor GREEN;
	static NS_CORE_API const nsLinearColor RED;
	static NS_CORE_API const nsLinearColor YELLOW;
	static NS_CORE_API const nsLinearColor WHITE;
	static NS_CORE_API const nsLinearColor WHITE_TRANSPARENT;

public:
	nsLinearColor() noexcept
		: R(0.0f)
		, G(0.0f)
		, B(0.0f)
		, A(0.0f)
	{
	}

	nsLinearColor(float rgba) noexcept
		: R(rgba)
		, G(rgba)
		, B(rgba)
		, A(rgba)
	{
	}

	nsLinearColor(float r, float g, float b, float a = 1.0f) noexcept
		: R(r)
		, G(g)
		, B(b)
		, A(a)
	{
	}

};



// ==================================================================================================================================================== //
// VECTOR2
// ==================================================================================================================================================== //
class nsVector2
{
public:
	float X;
	float Y;

	static NS_CORE_API const nsVector2 ZERO;

public:
	nsVector2() noexcept
		: X(0.0f)
		, Y(0.0f)
	{
	}

	nsVector2(float xy) noexcept
		: X(xy)
		, Y(xy)
	{
	}

	nsVector2(float x, float y) noexcept
		: X(x)
		, Y(y)
	{
	}

	NS_INLINE nsVector2 operator+(float rhs) const noexcept
	{
		return nsVector2(X + rhs, Y + rhs);
	}

	NS_INLINE nsVector2& operator+=(float rhs) noexcept
	{
		X += rhs;
		Y += rhs;
		return *this;
	}

	NS_INLINE nsVector2 operator+(const nsVector2& rhs) const noexcept
	{
		return nsVector2(X + rhs.X, Y + rhs.Y);
	}

	NS_INLINE nsVector2& operator+=(const nsVector2& rhs) noexcept
	{
		X += rhs.X;
		Y += rhs.Y;
		return *this;
	}

	NS_INLINE nsVector2 operator-(float rhs) const noexcept
	{
		return nsVector2(X - rhs, Y - rhs);
	}

	NS_INLINE nsVector2& operator-=(float rhs) noexcept
	{
		X -= rhs;
		Y -= rhs;
		return *this;
	}

	NS_INLINE nsVector2 operator-(const nsVector2& rhs) const noexcept
	{
		return nsVector2(X - rhs.X, Y - rhs.Y);
	}

	NS_INLINE nsVector2& operator-=(const nsVector2& rhs) noexcept
	{
		X -= rhs.X;
		Y -= rhs.Y;
		return *this;
	}

	NS_INLINE nsVector2 operator*(float rhs) const noexcept
	{
		return nsVector2(X * rhs, Y * rhs);
	}

	NS_INLINE nsVector2& operator*=(float rhs) noexcept
	{
		X *= rhs;
		Y *= rhs;
		return *this;
	}

	NS_INLINE nsVector2 operator*(const nsVector2& rhs) const noexcept
	{
		return nsVector2(X * rhs.X, Y * rhs.Y);
	}

	NS_INLINE nsVector2& operator*=(const nsVector2& rhs) noexcept
	{
		X *= rhs.X;
		Y *= rhs.Y;
		return *this;
	}

	NS_INLINE nsVector2 operator/(float rhs) const noexcept
	{
		return nsVector2(X / rhs, Y / rhs);
	}

	NS_INLINE nsVector2& operator/(float rhs) noexcept
	{
		X /= rhs;
		Y /= rhs;
		return *this;
	}

	NS_INLINE nsVector2 operator/(const nsVector2& rhs) const noexcept
	{
		return nsVector2(X / rhs.X, Y / rhs.Y);
	}

	NS_INLINE nsVector2& operator/=(const nsVector2& rhs) noexcept
	{
		X /= rhs.X;
		Y /= rhs.Y;
		return *this;
	}

	NS_INLINE float operator[](int index) const
	{
		NS_ValidateV(index >= 0 && index < 2, "nsVector2 index out or range!");
		return *((float*)this + index);
	}

	NS_NODISCARD_INLINE float GetMagnitudeSqr() const noexcept
	{
		return X * X + Y * Y;
	}

	NS_NODISCARD_INLINE float GetMagnitude() const noexcept
	{
		return sqrtf(X * X + Y * Y);
	}

	NS_INLINE void Normalize() noexcept
	{
		const float mag = GetMagnitude();

		if (mag != 0.0f)
		{
			X /= mag;
			Y /= mag;
		}
	}

	NS_NODISCARD_INLINE nsVector2 GetNormalized() const noexcept
	{
		nsVector2 n = *this;
		n.Normalize();
		return n;
	}

	NS_NODISCARD_INLINE bool IsZero() const noexcept
	{
		return nsMath::FloatEquals(X, 0.0f, NS_MATH_EPS_LOW_P) && nsMath::FloatEquals(Y, 0.0f, NS_MATH_EPS_LOW_P);
	}


	NS_NODISCARD static NS_INLINE float DotProduct(const nsVector2& a, const nsVector2& b) noexcept
	{
		return a.X * b.X + a.Y * b.Y;
	}


	NS_NODISCARD static NS_INLINE nsVector2 Lerp(const nsVector2& a, const nsVector2& b, float t) noexcept
	{
		return a + (b - a) * t;
	}

};



// ==================================================================================================================================================== //
// VECTOR3
// ==================================================================================================================================================== //
class nsVector3
{
public:
	float X;
	float Y;
	float Z;

	static NS_CORE_API const nsVector3 ZERO;
	static NS_CORE_API const nsVector3 RIGHT;
	static NS_CORE_API const nsVector3 UP;
	static NS_CORE_API const nsVector3 FORWARD;


public:
	nsVector3() noexcept
		: X(0.0f)
		, Y(0.0f)
		, Z(0.0f)
	{
	}

	nsVector3(float s) noexcept
		: X(s)
		, Y(s)
		, Z(s)
	{
	}

	nsVector3(float x, float y, float z) noexcept
		: X(x)
		, Y(y)
		, Z(z)
	{
	}

	nsVector3(const nsVector2& vec2, float z = 0.0f) noexcept
		: X(vec2.X)
		, Y(vec2.Y)
		, Z(z)
	{
	}

	NS_INLINE nsVector3 operator+(float rhs) const noexcept
	{
		return nsVector3(X + rhs, Y + rhs, Z + rhs);
	}

	NS_INLINE nsVector3& operator+=(float rhs) noexcept
	{
		X += rhs;
		Y += rhs;
		Z += rhs;
		return *this;
	}

	NS_INLINE nsVector3 operator+(const nsVector3& rhs) const noexcept
	{
		return nsVector3(X + rhs.X, Y + rhs.Y, Z + rhs.Z);
	}

	NS_INLINE nsVector3& operator+=(const nsVector3& rhs) noexcept
	{
		X += rhs.X;
		Y += rhs.Y;
		Z += rhs.Z;
		return *this;
	}

	NS_INLINE nsVector3 operator-(float rhs) const noexcept
	{
		return nsVector3(X - rhs, Y - rhs, Z - rhs);
	}

	NS_INLINE nsVector3& operator-=(float rhs) noexcept
	{
		X -= rhs;
		Y -= rhs;
		Z -= rhs;
		return *this;
	}

	NS_INLINE nsVector3 operator-() const noexcept
	{
		return nsVector3(-X, -Y, -Z);
	}

	NS_INLINE nsVector3 operator-(const nsVector3& rhs) const noexcept
	{
		return nsVector3(X - rhs.X, Y - rhs.Y, Z - rhs.Z);
	}

	NS_INLINE nsVector3& operator-=(const nsVector3& rhs) noexcept
	{
		X -= rhs.X;
		Y -= rhs.Y;
		Z -= rhs.Z;
		return *this;
	}

	NS_INLINE nsVector3 operator*(float rhs) const noexcept
	{
		return nsVector3(X * rhs, Y * rhs, Z * rhs);
	}

	NS_INLINE nsVector3& operator*=(float rhs) noexcept
	{
		X *= rhs;
		Y *= rhs;
		Z *= rhs;
		return *this;
	}

	NS_INLINE nsVector3 operator*(const nsVector3& rhs) const noexcept
	{
		return nsVector3(X * rhs.X, Y * rhs.Y, Z * rhs.Z);
	}

	NS_INLINE nsVector3& operator*=(const nsVector3& rhs) noexcept
	{
		X *= rhs.X;
		Y *= rhs.Y;
		Z *= rhs.Z;
		return *this;
	}

	NS_INLINE nsVector3 operator/(float rhs) const noexcept
	{
		return nsVector3(X / rhs, Y / rhs, Z / rhs);
	}

	NS_INLINE nsVector3& operator/=(float rhs) noexcept
	{
		X /= rhs;
		Y /= rhs;
		Z /= rhs;
		return *this;
	}

	NS_INLINE nsVector3 operator/(const nsVector3& rhs) const noexcept
	{
		return nsVector3(X / rhs.X, Y / rhs.Y, Z / rhs.Z);
	}

	NS_INLINE nsVector3& operator/=(const nsVector3& rhs) noexcept
	{
		X /= rhs.X;
		Y /= rhs.Y;
		Z /= rhs.Z;
		return *this;
	}

	NS_INLINE float& operator[](int index) const
	{
		NS_ValidateV(index >= 0 && index < 3, "nsVector3 index out or range!");
		return *((float*)this + index);
	}

	NS_NODISCARD_INLINE float GetMagnitudeSqr() const noexcept
	{
		return X * X + Y * Y + Z * Z;
	}

	NS_NODISCARD_INLINE float GetMagnitude() const noexcept
	{
		return sqrtf(X * X + Y * Y + Z * Z);
	}

	NS_INLINE void Normalize() noexcept
	{
		const float mag = GetMagnitude();

		if (!nsMath::IsZero(mag))
		{
			X /= mag;
			Y /= mag;
			Z /= mag;
		}
	}

	NS_NODISCARD_INLINE nsVector3 GetNormalized() const noexcept
	{
		nsVector3 n = *this;
		n.Normalize();
		return n;
	}

	NS_NODISCARD_INLINE bool IsZero() const noexcept
	{
		return nsMath::FloatEquals(X, 0.0f, NS_MATH_EPS_LOW_P) && nsMath::FloatEquals(Y, 0.0f, NS_MATH_EPS_LOW_P) && nsMath::FloatEquals(Z, 0.0f, NS_MATH_EPS_LOW_P);
	}

	NS_NODISCARD_INLINE bool IsEquals(const nsVector3& other, float eps = NS_MATH_EPS_LOW_P) const noexcept
	{
		return nsMath::FloatEquals(X, other.X, eps) && nsMath::FloatEquals(Y, other.Y, eps) && nsMath::FloatEquals(Z, other.Z, eps);
	}

	NS_NODISCARD_INLINE bool IsNormalized() const noexcept
	{
		return nsMath::FloatEquals(GetMagnitudeSqr(), 1.0f, NS_MATH_EPS_LOW_P);
	}


	NS_NODISCARD static NS_INLINE float DotProduct(const nsVector3& a, const nsVector3& b) noexcept
	{
		return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
	}

	NS_NODISCARD static NS_INLINE nsVector3 CrossProduct(const nsVector3& a, const nsVector3& b) noexcept
	{
		return nsVector3
		(
			a.Y * b.Z - a.Z * b.Y,
			a.Z * b.X - a.X * b.Z,
			a.X * b.Y - a.Y * b.X
		);
	}

	NS_NODISCARD static NS_INLINE nsVector3 Lerp(const nsVector3& a, const nsVector3& b, float t) noexcept
	{
		return a + (b - a) * t;
	}

	// Returns angle between two vectors in radian
	NS_NODISCARD static NS_INLINE float AngleBetween(const nsVector3& a, const nsVector3& b) noexcept
	{
		NS_Assert(a.IsNormalized());
		NS_Assert(b.IsNormalized());

		const float dot = DotProduct(a, b);

		return nsMath::ACos(nsMath::Clamp(dot, -1.0f, 1.0f));
	}

	NS_NODISCARD static NS_INLINE float DistanceSqr(const nsVector3& a, const nsVector3& b) noexcept
	{
		return (b - a).GetMagnitudeSqr();
	}

	NS_NODISCARD static NS_INLINE float Distance(const nsVector3& a, const nsVector3& b) noexcept
	{
		return (b - a).GetMagnitude();
	}

};



// ==================================================================================================================================================== //
// VECTOR4
// ==================================================================================================================================================== //
class nsVector4
{
public:
#if NS_MATH_SIMD
	union
	{
		struct
		{
			float X, Y, Z, W;
		};

		__m128 Xmm;
	};

#else
	float X;
	float Y;
	float Z;
	float W;

#endif // NS_MATH_SIMD

public:
	nsVector4() noexcept
		: X(0.0f)
		, Y(0.0f)
		, Z(0.0f)
		, W(0.0f)
	{
	}

	nsVector4(float x, float y, float z, float w) noexcept
		: X(x)
		, Y(y)
		, Z(z)
		, W(w)
	{
	}

	nsVector4(nsVector3 vec, float w = 1.0f) noexcept
		: X(vec.X)
		, Y(vec.Y)
		, Z(vec.Z)
		, W(w)
	{
	}

	NS_NODISCARD_INLINE float GetMagnitudeSqr() const noexcept
	{
		return X * X + Y * Y + Z * Z + W * W;
	}

	NS_NODISCARD_INLINE float GetMagnitude() const noexcept
	{
		return sqrtf(X * X + Y * Y + Z * Z + W * W);
	}

	NS_INLINE void Normalize() noexcept
	{
		const float mag = GetMagnitude();

		if (mag != 0.0f)
		{
			X /= mag;
			Y /= mag;
			Z /= mag;
			W /= mag;
		}
	}

	NS_NODISCARD_INLINE nsVector4 GetNormalized() const noexcept
	{
		nsVector4 n = *this;
		n.Normalize();
		return n;
	}

	NS_NODISCARD_INLINE nsVector3 ToVector3() const noexcept
	{
		return nsVector3(X, Y, Z);
	}


	NS_INLINE nsVector4 operator+(float rhs) const noexcept
	{
		return nsVector4(X + rhs, Y + rhs, Z + rhs, W + rhs);
	}

	NS_INLINE nsVector4& operator+=(float rhs) noexcept
	{
		X += rhs;
		Y += rhs;
		Z += rhs;
		W += rhs;
		return *this;
	}

	NS_INLINE nsVector4 operator+(const nsVector4& rhs) const noexcept
	{
		return nsVector4(X + rhs.X, Y + rhs.Y, Z + rhs.Z, W + rhs.W);
	}

	NS_INLINE nsVector4& operator+=(const nsVector4& rhs) noexcept
	{
		X += rhs.X;
		Y += rhs.Y;
		Z += rhs.Z;
		W += rhs.W;
		return *this;
	}

	NS_INLINE nsVector4 operator-(float rhs) const noexcept
	{
		return nsVector4(X - rhs, Y - rhs, Z - rhs, W - rhs);
	}

	NS_INLINE nsVector4& operator-=(float rhs) noexcept
	{
		X -= rhs;
		Y -= rhs;
		Z -= rhs;
		W -= rhs;
		return *this;
	}

	NS_INLINE nsVector4 operator-(const nsVector4& rhs) const noexcept
	{
		return nsVector4(X - rhs.X, Y - rhs.Y, Z - rhs.Z, W - rhs.W);
	}

	NS_INLINE nsVector4& operator-=(const nsVector4& rhs) noexcept
	{
		X -= rhs.X;
		Y -= rhs.Y;
		Z -= rhs.Z;
		W -= rhs.W;
		return *this;
	}

	NS_INLINE nsVector4 operator*(float rhs) const noexcept
	{
		return nsVector4(X * rhs, Y * rhs, Z * rhs, W * rhs);
	}

	NS_INLINE nsVector4& operator*=(float rhs) noexcept
	{
		X *= rhs;
		Y *= rhs;
		Z *= rhs;
		W *= rhs;
		return *this;
	}

	NS_INLINE nsVector4 operator*(const nsVector4& rhs) const noexcept
	{
		return nsVector4(X * rhs.X, Y * rhs.Y, Z * rhs.Z, W * rhs.W);
	}

	NS_INLINE nsVector4& operator*=(const nsVector4& rhs) noexcept
	{
		X *= rhs.X;
		Y *= rhs.Y;
		Z *= rhs.Z;
		W *= rhs.W;
		return *this;
	}

	NS_INLINE nsVector4 operator/(float rhs) const noexcept
	{
		return nsVector4(X / rhs, Y / rhs, Z / rhs, W / rhs);
	}

	NS_INLINE nsVector4& operator/(float rhs) noexcept
	{
		X /= rhs;
		Y /= rhs;
		Z /= rhs;
		W /= rhs;
		return *this;
	}

	NS_INLINE nsVector4 operator/(const nsVector4& rhs) const noexcept
	{
		return nsVector4(X / rhs.X, Y / rhs.Y, Z / rhs.Z, W / rhs.W);
	}

	NS_INLINE nsVector4& operator/=(const nsVector4& rhs) noexcept
	{
		X /= rhs.X;
		Y /= rhs.Y;
		Z /= rhs.Z;
		W /= rhs.W;
		return *this;
	}

	NS_INLINE nsVector4& operator/=(float rhs) noexcept
	{
		X /= rhs;
		Y /= rhs;
		Z /= rhs;
		W /= rhs;
		return *this;
	}

	NS_INLINE float operator[](int index)
	{
		NS_ValidateV(index >= 0 && index < 4, "nsVector4 index out or range!");
		return *((float*)this + index);
	}

	NS_INLINE const float operator[](int index) const
	{
		NS_ValidateV(index >= 0 && index < 4, "nsVector4 index out or range!");
		return *((float*)this + index);
	}

	NS_NODISCARD static NS_INLINE float DotProduct(const nsVector4& a, const nsVector4& b) noexcept
	{
		return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;
	}

	NS_NODISCARD static NS_INLINE nsVector4 Lerp(const nsVector4& a, const nsVector4& b, float t) noexcept
	{
		return a + (b - a) * t;
	}

};



// =============================================================================================================================================================== //
// QUATERNION (X, Y, Z, W)
// =============================================================================================================================================================== //
class nsQuaternion
{
public:
	union
	{
		struct
		{
			float X;
			float Y;
			float Z;
		};

		nsVector3 Vec;
	};

	float W;

	static NS_CORE_API const nsQuaternion IDENTITY;


public:
	nsQuaternion() noexcept
		: X(0.0f)
		, Y(0.0f)
		, Z(0.0f)
		, W(1.0f)
	{
	}

	nsQuaternion(float x, float y, float z, float w = 1.0f) noexcept
		: X(x)
		, Y(y)
		, Z(z)
		, W(w)
	{
	}

	nsQuaternion(const nsVector3& axis, float radian) noexcept
	{
		const nsVector3 n = axis.GetNormalized();
		X = n.X * nsMath::Sin(radian * 0.5f);
		Y = n.Y * nsMath::Sin(radian * 0.5f);
		Z = n.Z * nsMath::Sin(radian * 0.5f);
		W = nsMath::Cos(radian * 0.5f);
	}

	NS_INLINE nsQuaternion operator+(const nsQuaternion& rhs) const noexcept
	{
		nsQuaternion q = nsQuaternion(X + rhs.X, Y + rhs.Y, Z + rhs.Z, W + rhs.W);
		q.Normalize();

		return q;
	}

	NS_INLINE nsQuaternion operator-(const nsQuaternion& rhs) const noexcept
	{
		nsQuaternion q = nsQuaternion(X - rhs.X, Y - rhs.Y, Z - rhs.Z, W - rhs.W);
		q.Normalize();

		return q;
	}

	NS_INLINE nsQuaternion operator*(const nsQuaternion& rhs) const noexcept
	{
		nsQuaternion q;
		q.Vec = rhs.Vec * W + Vec * rhs.W + nsVector3::CrossProduct(Vec, rhs.Vec);
		q.W = W * rhs.W - nsVector3::DotProduct(Vec, rhs.Vec);
		q.Normalize();

		return q;
	}

	NS_INLINE nsQuaternion& operator*=(const nsQuaternion& rhs) noexcept
	{
		*this = *this * rhs;
		return *this;
	}

	NS_INLINE nsQuaternion operator*(float rhs) const noexcept
	{
		nsQuaternion q = nsQuaternion(X * rhs, Y * rhs, Z * rhs, W * rhs);
		q.Normalize();

		return q;
	}

	NS_NODISCARD_INLINE float GetMagnitudeSqr() const noexcept
	{
		return X * X + Y * Y + Z * Z + W * W;
	}

	NS_NODISCARD_INLINE float GetMagnitude() const noexcept
	{
		return sqrtf(X * X + Y * Y + Z * Z + W * W);
	}

	NS_INLINE void Normalize()
	{
		const float mag = GetMagnitude();

		if (mag != 0.0f)
		{
			X /= mag;
			Y /= mag;
			Z /= mag;
			W /= mag;
		}
	}

	NS_NODISCARD_INLINE nsQuaternion GetNormalized() const
	{
		const float mag = GetMagnitude();
		return nsQuaternion(X / mag, Y / mag, Z / mag, W / mag);
	}

	NS_NODISCARD_INLINE nsQuaternion GetConjugate() const noexcept
	{
		return nsQuaternion(-X, -Y, -Z, W);
	}

	NS_NODISCARD_INLINE nsQuaternion GetInverse() const noexcept
	{
		const nsQuaternion conjugate = GetConjugate();
		const float sqrMag = GetMagnitudeSqr();

		return nsQuaternion(conjugate.X / sqrMag, conjugate.Y / sqrMag, conjugate.Z / sqrMag, conjugate.W / sqrMag);
	}

	NS_NODISCARD_INLINE nsVector3 GetPitchYawRoll() const noexcept
	{
		const float pitch = atan2f(2.0f * X * W - 2.0f * Y * Z, 1.0f - 2.0f * X * X - 2.0f * Z * Z);
		const float yaw = atan2f(2.0f * Y * W - 2.0f * X * Z, 1.0f - 2.0f * Y * Y - 2.0f * Z * Z);

		const float sinR = 2.0f * X * Y + 2.0f * Z * W;
		float roll = 0.0f;

		if (nsMath::Abs(sinR) >= 1.0f)
		{
			roll = std::copysignf(NS_MATH_PI_2, sinR);
		}
		else
		{
			roll = asinf(sinR);
		}

		return nsVector3(nsMath::RadToDeg(pitch), nsMath::RadToDeg(yaw), nsMath::RadToDeg(roll));
	}

	NS_NODISCARD static NS_INLINE bool Equals(const nsQuaternion& a, const nsQuaternion& b, float eps = NS_MATH_EPS_LOW_P) noexcept
	{
		return nsMath::FloatEquals(a.X, b.X, eps) && nsMath::FloatEquals(a.Y, b.Y, eps) && nsMath::FloatEquals(a.Z, b.Z, eps) && nsMath::FloatEquals(a.W, b.W, eps);
	}

	NS_NODISCARD static NS_INLINE nsQuaternion FromVectors(const nsVector3& from, const nsVector3& to) noexcept
	{
		NS_Assert(from.IsNormalized());
		NS_Assert(to.IsNormalized());

		const float dot = nsVector3::DotProduct(from, to);
		nsQuaternion quat = nsQuaternion::IDENTITY;

		if (nsMath::FloatGreaterThanEquals(dot, 1.0f, NS_MATH_EPS))
		{
			return quat;
		}
		else if (nsMath::FloatLessThanEquals(dot, -1.0f, NS_MATH_EPS))
		{
			quat.Vec = nsVector3(-from.Z, from.Y, from.X);
			quat.Normalize();

			return quat;
		}

		quat.Vec = nsVector3::CrossProduct(from, to);
		quat.W = from.GetMagnitude() * to.GetMagnitude() + nsVector3::DotProduct(from, to);
		quat.Normalize();

		return quat;
	}

	// Create quaternion from pitch, yaw, roll in degree
	NS_NODISCARD static NS_INLINE nsQuaternion FromRotation(float pitchDegree, float yawDegree, float rollDegree) noexcept
	{
		const float pitchRad = nsMath::DegToRad(pitchDegree);
		const float yawRad = nsMath::DegToRad(yawDegree);
		const float rollRad = nsMath::DegToRad(rollDegree);

		const float cp = cosf(pitchRad * 0.5f);
		const float sp = sinf(pitchRad * 0.5f);
		const float cy = cosf(yawRad * 0.5f);
		const float sy = sinf(yawRad * 0.5f);
		const float cr = cosf(rollRad * 0.5f);
		const float sr = sinf(rollRad * 0.5f);

		nsQuaternion quat;
		quat.X = cr * sp * cy + sr * cp * sy;
		quat.Y = cr * cp * sy - sr * sp * cy;
		quat.Z = sr * cp * cy - cr * sp * sy;
		quat.W = cr * cp * cy + sr * sp * sy;
		quat.Normalize();

		return quat;
	}

	NS_NODISCARD static NS_INLINE nsQuaternion FromRotation(const nsVector3& pitchYawRoll) noexcept
	{
		return FromRotation(pitchYawRoll.X, pitchYawRoll.Y, pitchYawRoll.Z);
	}

	NS_NODISCARD static NS_INLINE nsVector3 RotateVector(const nsQuaternion& quat, const nsVector3& vec) noexcept
	{
		const nsVector3 nvec = vec.GetNormalized();
		//const nsQuaternion nq = quat.GetNormalized();
		const nsQuaternion q0 = quat * nsQuaternion(nvec.X, nvec.Y, nvec.Z, 0.0f);
		const nsQuaternion q1 = q0 * quat.GetConjugate();

		return q1.Vec;
	}

	NS_NODISCARD static NS_INLINE nsQuaternion Lerp(const nsQuaternion& a, const nsQuaternion& b, float t) noexcept
	{
		nsQuaternion quat;
		quat.X = a.X + (b.X - a.X) * t;
		quat.Y = a.Y + (b.Y - a.Y) * t;
		quat.Z = a.Z + (b.Z - a.Z) * t;
		quat.W = a.W + (b.W - a.W) * t;
		quat.Normalize();

		return quat;
	}

	NS_NODISCARD static NS_INLINE nsQuaternion Slerp(const nsQuaternion& quatA, const nsQuaternion& quatB, float t) noexcept
	{
		const float cosHalfTheta = quatA.X * quatB.X + quatA.Y * quatB.Y + quatA.Z * quatB.Z + quatA.W * quatB.W;

		if (nsMath::Abs(cosHalfTheta) >= 1.0f)
		{
			return quatA;
		}

		const float halfTheta = nsMath::ACos(cosHalfTheta);
		const float sinHalfTheta = nsMath::Sqrt(1.0f - cosHalfTheta * cosHalfTheta);

		if (nsMath::Abs(sinHalfTheta) < 0.001f)
		{
			return quatA * 0.5f + quatB * 0.5f;
		}

		const float ratioA = nsMath::Sin((1.0f - t) * halfTheta) / sinHalfTheta;
		const float ratioB = nsMath::Sin(t * halfTheta) / sinHalfTheta;

		return quatA * ratioA + quatB * ratioB;
	}

};



// =============================================================================================================================================================== //
// MATRIX 3x3
// =============================================================================================================================================================== //
class nsMatrix3
{
public:
	union
	{
		float M[3][3];
		nsVector3 Vecs[3];
	};

	static NS_CORE_API const nsMatrix3 IDENTITY;


public:
	nsMatrix3() noexcept
		: M()
	{
	}


	nsMatrix3(const nsVector3& row0, const nsVector3& row1, const nsVector3& row2) noexcept
	{
		Vecs[0] = row0;
		Vecs[1] = row1;
		Vecs[2] = row2;
	}


	nsMatrix3(float m00, float m01, float m02,
			  float m10, float m11, float m12,
			  float m20, float m21, float m22) noexcept
	{
		M[0][0] = m00;	M[0][1] = m01;	M[0][2] = m02;
		M[1][0] = m10;	M[1][1] = m11;	M[1][2] = m12;
		M[2][0] = m20;	M[2][1] = m21;	M[2][2] = m22;
	}


	NS_NODISCARD_INLINE nsMatrix3 GetTranspose() const noexcept
	{
		nsMatrix3 r;

		r.M[0][0] = M[0][0];
		r.M[0][1] = M[1][0];
		r.M[0][2] = M[2][0];

		r.M[1][0] = M[0][1];
		r.M[1][1] = M[1][1];
		r.M[1][2] = M[2][1];

		r.M[2][0] = M[0][2];
		r.M[2][1] = M[1][2];
		r.M[2][2] = M[2][2];

		return r;
	}


	NS_NODISCARD_INLINE float GetDeterminant() const noexcept
	{
		// | a, b, c |
		// | d, e, f |
		// | g, h, i |
		const float a = M[0][0]; const float b = M[0][1]; const float c = M[0][2];
		const float d = M[1][0]; const float e = M[1][1]; const float f = M[1][2];
		const float g = M[2][0]; const float h = M[2][1]; const float i = M[2][2];

		return a*(e*i - f*h) - b*(d*i - f*g) + c* (d*h - e*g);
	}


	NS_NODISCARD_INLINE nsMatrix3 GetInverse() const noexcept
	{
		const float determinant = GetDeterminant();
		if (abs(determinant) <= NS_MATH_EPS) return nsMatrix3();

		// | a, b, c |
		// | d, e, f |
		// | g, h, i |
		const float a = M[0][0]; const float b = M[0][1]; const float c = M[0][2];
		const float d = M[1][0]; const float e = M[1][1]; const float f = M[1][2];
		const float g = M[2][0]; const float h = M[2][1]; const float i = M[2][2];

		// Minor cofactors
		const nsMatrix3 mc((e*i - f*h), -(d*i - f*g),  (d*h - e*g),
						  -(b*i - c*h),  (a*i - c*g), -(a*h - b*g),
						   (b*f - c*e), -(a*f - c*d),  (a*e - b*d));

		return Multiply(mc.GetTranspose(), 1.0f / determinant);
	}


	NS_NODISCARD static NS_INLINE bool Equals(const nsMatrix3& a, const nsMatrix3& b, float eps = NS_MATH_EPS_LOW_P) noexcept
	{
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				if (!nsMath::FloatEquals(a.M[i][j], b.M[i][j], eps)) return false;
			}
		}

		return true;
	}


	NS_NODISCARD static NS_INLINE nsMatrix3 Add(const nsMatrix3& a, const nsMatrix3& b) noexcept
	{
		nsMatrix3 r;
		r.Vecs[0] = a.Vecs[0] + b.Vecs[0];
		r.Vecs[1] = a.Vecs[1] + b.Vecs[1];
		r.Vecs[2] = a.Vecs[2] + b.Vecs[2];

		return r;
	}


	NS_NODISCARD static NS_INLINE nsMatrix3 Subtract(const nsMatrix3& a, const nsMatrix3& b) noexcept
	{
		nsMatrix3 r;
		r.Vecs[0] = a.Vecs[0] - b.Vecs[0];
		r.Vecs[1] = a.Vecs[1] - b.Vecs[1];
		r.Vecs[2] = a.Vecs[2] - b.Vecs[2];

		return r;
	}


	NS_NODISCARD static NS_INLINE nsMatrix3 Multiply(const nsMatrix3& mat, float scalar) noexcept
	{
		nsMatrix3 r;
		r.Vecs[0] = mat.Vecs[0] * scalar;
		r.Vecs[1] = mat.Vecs[1] * scalar;
		r.Vecs[2] = mat.Vecs[2] * scalar;

		return r;
	}


	NS_INLINE float* operator[](int index) noexcept
	{
		NS_AssertV(index >= 0 && index < 3, "Matrix element index out of range!");
		return M[index];
	}


	NS_INLINE const float* operator[](int index) const noexcept
	{
		NS_AssertV(index >= 0 && index < 3, "Matrix element index out of range!");
		return M[index];
	}


	NS_INLINE nsMatrix3 operator+(const nsMatrix3& rhs) const noexcept
	{
		return Add(*this, rhs);
	}


	NS_INLINE nsMatrix3 operator-(const nsMatrix3& rhs) const noexcept
	{
		return Subtract(*this, rhs);
	}


	NS_INLINE friend nsMatrix3 operator*(const nsMatrix3& lhs, float rhs) noexcept
	{
		return nsMatrix3::Multiply(lhs, rhs);
	}

};



// =============================================================================================================================================================== //
// MATRIX 4x4
// =============================================================================================================================================================== //
#if NS_MATH_SIMD
// Credits: https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html

#define MakeShuffleMask(x,y,z,w)           (x | (y<<2) | (z<<4) | (w<<6))

// vec(0, 1, 2, 3) -> (vec[x], vec[y], vec[z], vec[w])
#define VecSwizzleMask(vec, mask)          _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), mask))
#define VecSwizzle(vec, x, y, z, w)        VecSwizzleMask(vec, MakeShuffleMask(x,y,z,w))
#define VecSwizzle1(vec, x)                VecSwizzleMask(vec, MakeShuffleMask(x,x,x,x))
// special swizzle
#define VecSwizzle_0022(vec)               _mm_moveldup_ps(vec)
#define VecSwizzle_1133(vec)               _mm_movehdup_ps(vec)

// return (vec1[x], vec1[y], vec2[z], vec2[w])
#define VecShuffle(vec1, vec2, x,y,z,w)    _mm_shuffle_ps(vec1, vec2, MakeShuffleMask(x,y,z,w))
// special shuffle
#define VecShuffle_0101(vec1, vec2)        _mm_movelh_ps(vec1, vec2)
#define VecShuffle_2323(vec1, vec2)        _mm_movehl_ps(vec2, vec1)

#endif // NS_MATH_SIMD


class nsMatrix4
{
public:
#if NS_MATH_SIMD
	union
	{
		float M[4][4];
		nsVector4 Vecs[4];
		__m128 Xmms[4];
	};

#else
	union
	{
		float M[4][4];
		nsVector4 Vecs[4];
	};

#endif // NS_MATH_SIMD


	static NS_CORE_API const nsMatrix4 IDENTITY;


public:
	nsMatrix4() noexcept
		: M()
	{
	}


	nsMatrix4(const nsVector4& row0, const nsVector4& row1, const nsVector4& row2, const nsVector4& row3)
	{
	#if NS_MATH_SIMD
		Xmms[0] = _mm_load_ps((const float*)&row0);
		Xmms[1] = _mm_load_ps((const float*)&row1);
		Xmms[2] = _mm_load_ps((const float*)&row2);
		Xmms[3] = _mm_load_ps((const float*)&row3);

	#else
		Vecs[0] = row0;
		Vecs[1] = row1;
		Vecs[2] = row2;
		Vecs[3] = row3;

	#endif // NS_MATH_SIMD
	}


	nsMatrix4(float m00, float m01, float m02, float m03,
			  float m10, float m11, float m12, float m13,
			  float m20, float m21, float m22, float m23,
			  float m30, float m31, float m32, float m33)
	{
		M[0][0] = m00;	M[0][1] = m01;	M[0][2] = m02;	M[0][3] = m03;
		M[1][0] = m10;	M[1][1] = m11;	M[1][2] = m12;	M[1][3] = m13;
		M[2][0] = m20;	M[2][1] = m21;	M[2][2] = m22;	M[2][3] = m23;
		M[3][0] = m30;	M[3][1] = m31;	M[3][2] = m32;	M[3][3] = m33;
	}


	NS_NODISCARD static NS_INLINE bool Equals(const nsMatrix4& a, const nsMatrix4& b, float epsilon = NS_MATH_EPS) noexcept
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				if (!nsMath::FloatEquals(a.M[i][j], b.M[i][j], epsilon)) return false;
			}
		}

		return true;
	}


	NS_NODISCARD static NS_INLINE nsMatrix4 Add(const nsMatrix4& a, const nsMatrix4& b) noexcept
	{
		nsMatrix4 r;

	#if NS_MATH_SIMD
		r.Xmms[0] = _mm_add_ps(a.Xmms[0], b.Xmms[0]);
		r.Xmms[1] = _mm_add_ps(a.Xmms[1], b.Xmms[1]);
		r.Xmms[2] = _mm_add_ps(a.Xmms[2], b.Xmms[2]);
		r.Xmms[3] = _mm_add_ps(a.Xmms[3], b.Xmms[3]);

	#else
		r.Vecs[0] = a.Vecs[0] + b.Vecs[0];
		r.Vecs[1] = a.Vecs[1] + b.Vecs[1];
		r.Vecs[2] = a.Vecs[2] + b.Vecs[2];
		r.Vecs[3] = a.Vecs[3] + b.Vecs[3];

	#endif // NS_MATH_SIMD

		return r;
	}


	NS_NODISCARD static NS_INLINE nsMatrix4 Subtract(const nsMatrix4& a, const nsMatrix4& b) noexcept
	{
		nsMatrix4 r;

	#if NS_MATH_SIMD
		r.Xmms[0] = _mm_sub_ps(a.Xmms[0], b.Xmms[0]);
		r.Xmms[1] = _mm_sub_ps(a.Xmms[1], b.Xmms[1]);
		r.Xmms[2] = _mm_sub_ps(a.Xmms[2], b.Xmms[2]);
		r.Xmms[3] = _mm_sub_ps(a.Xmms[3], b.Xmms[3]);

	#else
		r.Vecs[0] = a.Vecs[0] - b.Vecs[0];
		r.Vecs[1] = a.Vecs[1] - b.Vecs[1];
		r.Vecs[2] = a.Vecs[2] - b.Vecs[2];
		r.Vecs[3] = a.Vecs[3] - b.Vecs[3];

	#endif // NS_MATH_SIMD

		return r;
	}


	NS_NODISCARD static NS_INLINE nsMatrix4 Multiply(const nsMatrix4& a, const nsMatrix4& b) noexcept
	{
		nsMatrix4 r;

	#if NS_MATH_SIMD
		// (100,000 matrices. Debug: 11.5 ms, Release: 0.15 ms)

		for (int i = 0; i < 4; ++i)
		{
			r.Xmms[i] = _mm_mul_ps(_mm_shuffle_ps(a.Xmms[i], a.Xmms[i], 0x00), b.Xmms[0]);
			r.Xmms[i] = _mm_add_ps(r.Xmms[i], _mm_mul_ps(_mm_shuffle_ps(a.Xmms[i], a.Xmms[i], 0x55), b.Xmms[1]));
			r.Xmms[i] = _mm_add_ps(r.Xmms[i], _mm_mul_ps(_mm_shuffle_ps(a.Xmms[i], a.Xmms[i], 0xAA), b.Xmms[2]));
			r.Xmms[i] = _mm_add_ps(r.Xmms[i], _mm_mul_ps(_mm_shuffle_ps(a.Xmms[i], a.Xmms[i], 0xFF), b.Xmms[3]));
		}

	#else
		// (100,000 matrices. Debug: 14.7 ms, Release: 0.8 ms)
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				r.M[i][j] = a.M[i][0] * b.M[0][j] + a.M[i][1] * b.M[1][j] + a.M[i][2] * b.M[2][j] + a.M[i][3] * b.M[3][j];
			}
		}

	#endif // NS_MATH_SIMD

		return r;
	}


	NS_NODISCARD static NS_INLINE nsMatrix4 Multiply(const nsMatrix4& mat, float scalar) noexcept
	{
		nsMatrix4 r;

	#if NS_MATH_SIMD
		__m128 temp = _mm_set_ps(scalar, scalar, scalar, scalar);
		r.Xmms[0] = _mm_mul_ps(mat.Xmms[0], temp);
		r.Xmms[1] = _mm_mul_ps(mat.Xmms[1], temp);
		r.Xmms[2] = _mm_mul_ps(mat.Xmms[2], temp);
		r.Xmms[3] = _mm_mul_ps(mat.Xmms[3], temp);

	#else
		r.Vecs[0] = mat.Vecs[0] * scalar;
		r.Vecs[1] = mat.Vecs[1] * scalar;
		r.Vecs[2] = mat.Vecs[2] * scalar;
		r.Vecs[3] = mat.Vecs[3] * scalar;

	#endif // NS_MATH_SIMD

		return r;
	}


	NS_NODISCARD static NS_INLINE nsVector4 Multiply(const nsVector4& vec, const nsMatrix4& mat) noexcept
	{
		nsVector4 r;

	#if NS_MATH_SIMD
		r.Xmm = _mm_mul_ps(_mm_shuffle_ps(vec.Xmm, vec.Xmm, 0x00), mat.Xmms[0]);
		r.Xmm = _mm_add_ps(r.Xmm, _mm_mul_ps(_mm_shuffle_ps(vec.Xmm, vec.Xmm, 0x55), mat.Xmms[1]));
		r.Xmm = _mm_add_ps(r.Xmm, _mm_mul_ps(_mm_shuffle_ps(vec.Xmm, vec.Xmm, 0xAA), mat.Xmms[2]));
		r.Xmm = _mm_add_ps(r.Xmm, _mm_mul_ps(_mm_shuffle_ps(vec.Xmm, vec.Xmm, 0xFF), mat.Xmms[3]));
			
	#else
		r.X = vec.X * mat.M[0][0] + vec.Y * mat.M[1][0] + vec.Z * mat.M[2][0] + vec.W * mat.M[3][0];
		r.Y = vec.X * mat.M[0][1] + vec.Y * mat.M[1][1] + vec.Z * mat.M[2][1] + vec.W * mat.M[3][1];
		r.Z = vec.X * mat.M[0][2] + vec.Y * mat.M[1][2] + vec.Z * mat.M[2][2] + vec.W * mat.M[3][2];
		r.W = vec.X * mat.M[0][3] + vec.Y * mat.M[1][3] + vec.Z * mat.M[2][3] + vec.W * mat.M[3][3];

	#endif // NS_MATH_SIMD

		return r;
	}


	NS_NODISCARD static NS_INLINE nsMatrix4 Translation(const nsVector3& position) noexcept
	{
		nsMatrix4 m = IDENTITY;
		m[3][0] = position.X;
		m[3][1] = position.Y;
		m[3][2] = position.Z;

		return m;
	}


	NS_NODISCARD static NS_INLINE nsMatrix4 RotationX(float radian) noexcept
	{
		nsMatrix4 m = IDENTITY;
		m[1][1] = cosf(radian);
		m[1][2] = sinf(radian);
		m[2][1] = -sinf(radian);
		m[2][2] = cosf(radian);

		return m;
	}


	NS_NODISCARD static NS_INLINE nsMatrix4 RotationY(float radian) noexcept
	{
		nsMatrix4 m = IDENTITY;
		m[0][0] = cosf(radian);
		m[0][2] = -sinf(radian);
		m[2][0] = sinf(radian);
		m[2][2] = cosf(radian);

		return m;
	}


	NS_NODISCARD static NS_INLINE nsMatrix4 RotationZ(float radian) noexcept
	{
		nsMatrix4 m = IDENTITY;
		m[0][0] = cosf(radian);
		m[0][1] = sinf(radian);
		m[1][0] = -sinf(radian);
		m[1][1] = cosf(radian);

		return m;
	}


	NS_NODISCARD static NS_INLINE nsMatrix4 RotationQuat(const nsQuaternion& quat) noexcept
	{
		const nsQuaternion nq = quat.GetNormalized();
		const float qxx = nq.X * nq.X;
		const float qxy = nq.X * nq.Y;
		const float qxz = nq.X * nq.Z;
		const float qxw = nq.X * nq.W;
		const float qyy = nq.Y * nq.Y;
		const float qyz = nq.Y * nq.Z;
		const float qyw = nq.Y * nq.W;
		const float qzz = nq.Z * nq.Z;
		const float qzw = nq.Z * nq.W;

		nsMatrix4 m = IDENTITY;
		m[0][0] = 1.0f - 2.0f * (qyy + qzz);	m[0][1] = 2.0f * (qxy + qzw);			m[0][2] = 2.0f * (qxz - qyw);
		m[1][0] = 2.0f * (qxy - qzw);			m[1][1] = 1.0f - 2.0f * (qxx + qzz);	m[1][2] = 2.0f * (qyz + qxw);
		m[2][0] = 2.0f * (qxz + qyw);			m[2][1] = 2.0f * (qyz - qxw);			m[2][2] = 1.0f - 2.0f * (qxx + qyy);

		return m;
	}


	NS_NODISCARD static NS_INLINE nsMatrix4 Scale(float scale) noexcept
	{
		nsMatrix4 m = IDENTITY;
		m[0][0] = scale;
		m[1][1] = scale;
		m[2][2] = scale;

		return m;
	}


	NS_NODISCARD static NS_INLINE nsMatrix4 Scale(const nsVector3& scale) noexcept
	{
		nsMatrix4 m = IDENTITY;
		m[0][0] = scale.X;
		m[1][1] = scale.Y;
		m[2][2] = scale.Z;

		return m;
	}


	NS_NODISCARD static NS_INLINE nsMatrix4 LookAt(const nsVector3& eye, const nsVector3& target, const nsVector3& up) noexcept
	{
		const nsVector3 f = (target - eye).GetNormalized();
		const nsVector3 r = nsVector3::CrossProduct(up, f).GetNormalized();
		const nsVector3 u = nsVector3::CrossProduct(f, r);

		nsMatrix4 m = IDENTITY;

	#if 0
		m.Xmms[0] = _mm_set_ps(r.X, r.Y, r.Z, 0.0f);
		m.Xmms[1] = _mm_set_ps(u.X, u.Y, u.Z, 0.0f);
		m.Xmms[2] = _mm_set_ps(f.X, f.Y, f.Z, 0.0f);
		m.Xmms[3] = _mm_set_ps(eye.X, eye.Y, eye.Z, 1.0f);

	#else
		m[0][0] = r.X;
		m[1][0] = r.Y;
		m[2][0] = r.Z;
		m[0][1] = u.X;
		m[1][1] = u.Y;
		m[2][1] = u.Z;
		m[0][2] = f.X;
		m[1][2] = f.Y;
		m[2][2] = f.Z;
		m[3][0] = -nsVector3::DotProduct(r, eye);
		m[3][1] = -nsVector3::DotProduct(u, eye);
		m[3][2] = -nsVector3::DotProduct(f, eye);

	#endif // SK_MATH_SSE
		
		return m;
	}


	// Create orthographic projection matrix
	// @param left - Projection left plane
	// @param right - Projection right plane
	// @param top - Projection top plane
	// @param bottom - Projection bottom plane
	// @param nearZ - near clip range
	// @param farZ - far clip range
	NS_NODISCARD static NS_INLINE nsMatrix4 Orthographic(float left, float right, float top, float bottom, float nearZ, float farZ)
	{
		nsMatrix4 m = IDENTITY;
		m[0][0] = 2.0f / (right - left);
		m[1][1] = 2.0f / (top - bottom);
		m[2][2] = 1.0f / (farZ - nearZ);
		m[3][0] = -(right + left) / (right - left);
		m[3][1] = -(top + bottom) / (top - bottom);
		m[3][2] = farZ / (farZ - nearZ);
		m[3][3] = 1.0f;

		return m;
	}


	// Create perspective projection matrix
	// @param fov - Field of view in degree
	// @param width - Projection width
	// @param height - Projection height
	// @param nearZ - near clip range
	// @param farZ - far clip range
	NS_NODISCARD static NS_INLINE nsMatrix4 Perspective(float fovDegree, float aspect, float nearZ, float farZ)
	{
		const float fovY = tanf(nsMath::DegToRad(fovDegree) * 0.5f);

		nsMatrix4 m = IDENTITY;
		m[0][0] = 1.0f / (aspect * fovY);
		m[1][1] = 1.0f / fovY;
		m[2][2] = (farZ) / (farZ - nearZ);
		m[2][3] = 1.0f;
		m[3][2] = -(farZ * nearZ) / (farZ - nearZ);
		m[3][3] = 0.0f;

		return m;
	}

	
	NS_INLINE void Transpose() noexcept
	{
		nsMatrix4 temp = *this;

		M[0][1] = temp[1][0];
		M[0][2] = temp[2][0];
		M[0][3] = temp[3][0];

		M[1][0] = temp[0][1];
		M[1][2] = temp[2][1];
		M[1][3] = temp[3][1];

		M[2][0] = temp[0][2];
		M[2][1] = temp[1][2];
		M[2][3] = temp[3][2];

		M[3][0] = temp[0][3];
		M[3][1] = temp[1][3];
		M[3][2] = temp[2][3];
	}
	

	NS_NODISCARD_INLINE nsMatrix4 GetTransposed() const noexcept
	{
		nsMatrix4 m;

		m[0][0] = M[0][0];
		m[0][1] = M[1][0];
		m[0][2] = M[2][0];
		m[0][3] = M[3][0];

		m[1][0] = M[0][1];
		m[1][1] = M[1][1];
		m[1][2] = M[2][1];
		m[1][3] = M[3][1];

		m[2][0] = M[0][2];
		m[2][1] = M[1][2];
		m[2][2] = M[2][2];
		m[2][3] = M[3][2];

		m[3][0] = M[0][3];
		m[3][1] = M[1][3];
		m[3][2] = M[2][3];
		m[3][3] = M[3][3];

		return m;
	}


	NS_NODISCARD_INLINE float GetDeterminant() const noexcept
	{
		// | a, b, c, d |
		// | e, f, g, h |
		// | i, j, k, l |
		// | m, n, o, p |
		const float a = M[0][0]; const float b = M[0][1]; const float c = M[0][2]; const float d = M[0][3];
		const float e = M[1][0]; const float f = M[1][1]; const float g = M[1][2]; const float h = M[1][3];
		const float i = M[2][0]; const float j = M[2][1]; const float k = M[2][2]; const float l = M[2][3];
		const float m = M[3][0]; const float n = M[3][1]; const float o = M[3][2]; const float p = M[3][3];

		const float detA = a * nsMatrix3(f, g, h, j, k, l, n, o, p).GetDeterminant();
		const float detB = b * nsMatrix3(e, g, h, i, k, l, m, o, p).GetDeterminant();
		const float detC = c * nsMatrix3(e, f, h, i, j, l, m, n, p).GetDeterminant();
		const float detD = d * nsMatrix3(e, f, g, i, j, k, m, n, o).GetDeterminant();

		return (detA - detB + detC - detD);
	}


	NS_NODISCARD_INLINE nsMatrix4 GetInverse() const noexcept
	{
	#if NS_MATH_SIMD
		// Credits: https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html

		auto Mat2Mul = [](__m128 vec1, __m128 vec2)
		{
			return _mm_add_ps(_mm_mul_ps(vec1, VecSwizzle(vec2, 0, 3, 0, 3)), _mm_mul_ps(VecSwizzle(vec1, 1, 0, 3, 2), VecSwizzle(vec2, 2, 1, 2, 1)));
		};

		// 2x2 row major Matrix adjugate multiply (A#)*B
		auto Mat2AdjMul = [](__m128 vec1, __m128 vec2)
		{
			return _mm_sub_ps(_mm_mul_ps(VecSwizzle(vec1, 3, 3, 0, 0), vec2), _mm_mul_ps(VecSwizzle(vec1, 1, 1, 2, 2), VecSwizzle(vec2, 2, 3, 0, 1)));

		};

		// 2x2 row major Matrix multiply adjugate A*(B#)
		auto Mat2MulAdj = [](__m128 vec1, __m128 vec2)
		{
			return _mm_sub_ps(_mm_mul_ps(vec1, VecSwizzle(vec2, 3, 0, 3, 0)), _mm_mul_ps(VecSwizzle(vec1, 1, 0, 3, 2), VecSwizzle(vec2, 2, 1, 2, 1)));
		};


		// use block matrix method
		// A is a matrix, then i(A) or iA means inverse of A, A# (or A_ in code) means adjugate of A, |A| (or detA in code) is determinant, tr(A) is trace

		// sub matrices
		__m128 A = VecShuffle_0101(Xmms[0], Xmms[1]);
		__m128 B = VecShuffle_2323(Xmms[0], Xmms[1]);
		__m128 C = VecShuffle_0101(Xmms[2], Xmms[3]);
		__m128 D = VecShuffle_2323(Xmms[2], Xmms[3]);

		// determinant as (|A| |B| |C| |D|)
		__m128 detSub = _mm_sub_ps(
			_mm_mul_ps(VecShuffle(Xmms[0], Xmms[2], 0, 2, 0, 2), VecShuffle(Xmms[1], Xmms[3], 1, 3, 1, 3)),
			_mm_mul_ps(VecShuffle(Xmms[0], Xmms[2], 1, 3, 1, 3), VecShuffle(Xmms[1], Xmms[3], 0, 2, 0, 2))
		);
		__m128 detA = VecSwizzle1(detSub, 0);
		__m128 detB = VecSwizzle1(detSub, 1);
		__m128 detC = VecSwizzle1(detSub, 2);
		__m128 detD = VecSwizzle1(detSub, 3);

		// let iM = 1/|M| * | X  Y |
		//                  | Z  W |

		// D#C
		__m128 D_C = Mat2AdjMul(D, C);
		// A#B
		__m128 A_B = Mat2AdjMul(A, B);
		// X# = |D|A - B(D#C)
		__m128 X_ = _mm_sub_ps(_mm_mul_ps(detD, A), Mat2Mul(B, D_C));
		// W# = |A|D - C(A#B)
		__m128 W_ = _mm_sub_ps(_mm_mul_ps(detA, D), Mat2Mul(C, A_B));

		// |M| = |A|*|D| + ... (continue later)
		__m128 detM = _mm_mul_ps(detA, detD);

		// Y# = |B|C - D(A#B)#
		__m128 Y_ = _mm_sub_ps(_mm_mul_ps(detB, C), Mat2MulAdj(D, A_B));
		// Z# = |C|B - A(D#C)#
		__m128 Z_ = _mm_sub_ps(_mm_mul_ps(detC, B), Mat2MulAdj(A, D_C));

		// |M| = |A|*|D| + |B|*|C| ... (continue later)
		detM = _mm_add_ps(detM, _mm_mul_ps(detB, detC));

		// tr((A#B)(D#C))
		__m128 tr = _mm_mul_ps(A_B, VecSwizzle(D_C, 0, 2, 1, 3));
		tr = _mm_hadd_ps(tr, tr);
		tr = _mm_hadd_ps(tr, tr);
		// |M| = |A|*|D| + |B|*|C| - tr((A#B)(D#C)
		detM = _mm_sub_ps(detM, tr);

		const __m128 adjSignMask = _mm_setr_ps(1.f, -1.f, -1.f, 1.f);
		// (1/|M|, -1/|M|, -1/|M|, 1/|M|)
		__m128 rDetM = _mm_div_ps(adjSignMask, detM);

		X_ = _mm_mul_ps(X_, rDetM);
		Y_ = _mm_mul_ps(Y_, rDetM);
		Z_ = _mm_mul_ps(Z_, rDetM);
		W_ = _mm_mul_ps(W_, rDetM);

		nsMatrix4 r;

		// apply adjugate and store, here we combine adjugate shuffle and store shuffle
		r.Xmms[0] = VecShuffle(X_, Y_, 3, 1, 3, 1);
		r.Xmms[1] = VecShuffle(X_, Y_, 2, 0, 2, 0);
		r.Xmms[2] = VecShuffle(Z_, W_, 3, 1, 3, 1);
		r.Xmms[3] = VecShuffle(Z_, W_, 2, 0, 2, 0);

		return r;

	#else
		const float determinant = GetDeterminant();
		if (abs(determinant) <= NS_MATH_SMALL_FLOAT) return nsMatrix4();

		// | a, b, c, d |
		// | e, f, g, h |
		// | i, j, k, l |
		// | m, n, o, p |
		const float a = M[0][0]; const float b = M[0][1]; const float c = M[0][2]; const float d = M[0][3];
		const float e = M[1][0]; const float f = M[1][1]; const float g = M[1][2]; const float h = M[1][3];
		const float i = M[2][0]; const float j = M[2][1]; const float k = M[2][2]; const float l = M[2][3];
		const float m = M[3][0]; const float n = M[3][1]; const float o = M[3][2]; const float p = M[3][3];

		// Minor cofactors
		nsMatrix4 mc;
		mc[0][0] =  nsMatrix3(f, g, h, j, k, l, n, o, p).GetDeterminant();
		mc[0][1] = -nsMatrix3(e, g, h, i, k, l, m, o, p).GetDeterminant();
		mc[0][2] =  nsMatrix3(e, f, h, i, j, l, m, n, p).GetDeterminant();
		mc[0][3] = -nsMatrix3(e, f, g, i, j, k, m, n, o).GetDeterminant();

		mc[1][0] = -nsMatrix3(b, c, d, j, k, l, n, o, p).GetDeterminant();
		mc[1][1] =  nsMatrix3(a, c, d, i, k, l, m, o, p).GetDeterminant();
		mc[1][2] = -nsMatrix3(a, b, d, i, j, l, m, n, p).GetDeterminant();
		mc[1][3] =  nsMatrix3(a, b, c, i, j, k, m, n, o).GetDeterminant();

		mc[2][0] =  nsMatrix3(b, c, d, f, g, h, n, o, p).GetDeterminant();
		mc[2][1] = -nsMatrix3(a, c, d, e, g, h, m, o, p).GetDeterminant();
		mc[2][2] =  nsMatrix3(a, b, d, e, f, h, m, n, p).GetDeterminant();
		mc[2][3] = -nsMatrix3(a, b, c, e, f, g, m, n, o).GetDeterminant();

		mc[3][0] = -nsMatrix3(b, c, d, f, g, h, j, k, l).GetDeterminant();
		mc[3][1] =  nsMatrix3(a, c, d, e, g, h, i, k, l).GetDeterminant();
		mc[3][2] = -nsMatrix3(a, b, d, e, f, h, i, j, l).GetDeterminant();
		mc[3][3] =  nsMatrix3(a, b, c, e, f, g, i, j, k).GetDeterminant();

		return Multiply(mc.GetTransposed(), 1.0f / determinant);

	#endif // NS_MATH_SIMD
	}


	NS_NODISCARD_INLINE nsMatrix4 GetInverseTransform() const noexcept
	{
	#if NS_MATH_SIMD
		// Credits: https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html

		nsMatrix4 r;

		// transpose 3x3, we know m03 = m13 = m23 = 0
		__m128 t0 = VecShuffle_0101(Xmms[0], Xmms[1]); // 00, 01, 10, 11
		__m128 t1 = VecShuffle_2323(Xmms[0], Xmms[1]); // 02, 03, 12, 13
		r.Xmms[0] = VecShuffle(t0, Xmms[2], 0, 2, 0, 3); // 00, 10, 20, 23(=0)
		r.Xmms[1] = VecShuffle(t0, Xmms[2], 1, 3, 1, 3); // 01, 11, 21, 23(=0)
		r.Xmms[2] = VecShuffle(t1, Xmms[2], 0, 2, 2, 3); // 02, 12, 22, 23(=0)

		// (SizeSqr(mVec[0]), SizeSqr(mVec[1]), SizeSqr(mVec[2]), 0)
		__m128 sizeSqr;
		sizeSqr = _mm_mul_ps(r.Xmms[0], r.Xmms[0]);
		sizeSqr = _mm_add_ps(sizeSqr, _mm_mul_ps(r.Xmms[1], r.Xmms[1]));
		sizeSqr = _mm_add_ps(sizeSqr, _mm_mul_ps(r.Xmms[2], r.Xmms[2]));

		// optional test to avoid divide by 0
		__m128 one = _mm_set1_ps(1.f);
		// for each component, if(sizeSqr < SMALL_NUMBER) sizeSqr = 1;
		__m128 rSizeSqr = _mm_blendv_ps(_mm_div_ps(one, sizeSqr), one, _mm_cmplt_ps(sizeSqr, _mm_set1_ps(NS_MATH_EPS)));

		r.Xmms[0] = _mm_mul_ps(r.Xmms[0], rSizeSqr);
		r.Xmms[1] = _mm_mul_ps(r.Xmms[1], rSizeSqr);
		r.Xmms[2] = _mm_mul_ps(r.Xmms[2], rSizeSqr);

		// last line
		r.Xmms[3] = _mm_mul_ps(r.Xmms[0], VecSwizzle1(Xmms[3], 0));
		r.Xmms[3] = _mm_add_ps(r.Xmms[3], _mm_mul_ps(r.Xmms[1], VecSwizzle1(Xmms[3], 1)));
		r.Xmms[3] = _mm_add_ps(r.Xmms[3], _mm_mul_ps(r.Xmms[2], VecSwizzle1(Xmms[3], 2)));
		r.Xmms[3] = _mm_sub_ps(_mm_setr_ps(0.f, 0.f, 0.f, 1.f), r.Xmms[3]);

		return r;

	#else
		return GetInverse();

	#endif // NS_MATH_SIMD
	}


	NS_NODISCARD_INLINE nsMatrix4 GetInverseTransformNoScale() const noexcept
	{
	#if NS_MATH_SIMD
		// Credits: https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html

		nsMatrix4 r;

		// transpose 3x3, we know m03 = m13 = m23 = 0
		__m128 t0 = VecShuffle_0101(Xmms[0], Xmms[1]); // 00, 01, 10, 11
		__m128 t1 = VecShuffle_2323(Xmms[0], Xmms[1]); // 02, 03, 12, 13
		r.Xmms[0] = VecShuffle(t0, Xmms[2], 0, 2, 0, 3); // 00, 10, 20, 23(=0)
		r.Xmms[1] = VecShuffle(t0, Xmms[2], 1, 3, 1, 3); // 01, 11, 21, 23(=0)
		r.Xmms[2] = VecShuffle(t1, Xmms[2], 0, 2, 2, 3); // 02, 12, 22, 23(=0)

		// last line
		r.Xmms[3] = _mm_mul_ps(r.Xmms[0], VecSwizzle1(Xmms[3], 0));
		r.Xmms[3] = _mm_add_ps(r.Xmms[3], _mm_mul_ps(r.Xmms[1], VecSwizzle1(Xmms[3], 1)));
		r.Xmms[3] = _mm_add_ps(r.Xmms[3], _mm_mul_ps(r.Xmms[2], VecSwizzle1(Xmms[3], 2)));
		r.Xmms[3] = _mm_sub_ps(_mm_setr_ps(0.f, 0.f, 0.f, 1.f), r.Xmms[3]);

		return r;

	#else
		return GetInverse();

	#endif // NS_MATH_SIMD
	}


	NS_NODISCARD_INLINE nsVector3 GetAxisRight() const noexcept
	{
		nsVector3 x(M[0][0], M[0][1], M[0][2]);
		x.Normalize();

		return x;
	}


	NS_NODISCARD_INLINE nsVector3 GetAxisUp() const noexcept
	{
		nsVector3 y(M[1][0], M[1][1], M[1][2]);
		y.Normalize();

		return y;
	}


	NS_NODISCARD_INLINE nsVector3 GetAxisForward() const noexcept
	{
		nsVector3 z(M[2][0], M[2][1], M[2][2]);
		z.Normalize();

		return z;
	}


	NS_NODISCARD_INLINE nsVector3 GetPosition() const noexcept
	{
		return nsVector3(M[3][0], M[3][1], M[3][2]);
	}


	NS_NODISCARD_INLINE nsQuaternion GetRotation() const
	{
		const nsVector3 scale = GetScale();

		nsMatrix3 m = nsMatrix3::IDENTITY;
		m.Vecs[0] = nsVector3(M[0][0] / scale.X, M[0][1] / scale.X, M[0][2] / scale.X);
		m.Vecs[0].Normalize();

		m.Vecs[1] = nsVector3(M[1][0] / scale.Y, M[1][1] / scale.Y, M[1][2] / scale.Y);
		m.Vecs[1].Normalize();

		m.Vecs[2] = nsVector3(M[2][0] / scale.Z, M[2][1] / scale.Z, M[2][2] / scale.Z);
		m.Vecs[2].Normalize();

		nsQuaternion q;
		const float tr = 1.0f + m[0][0] + m[1][1] + m[2][2];

		if (tr > 0.0f)
		{
			const float s = 0.5f / sqrtf(tr);
			q.X = (m[1][2] - m[2][1]) * s;
			q.Y = (m[2][0] - m[0][2]) * s;
			q.Z = (m[0][1] - m[1][0]) * s;
			q.W = 0.25f / s;
		}
		else if ((m[0][0] > m[1][1]) && (m[0][0] > m[2][2])) 
		{
			const float s = sqrtf(1.0f + m[0][0] - m[1][1] - m[2][2]) * 2.0f; // S=4*qx 
			q.X = 0.25f * s;
			q.Y = (m[1][0] + m[0][1]) / s;
			q.Z = (m[2][0] + m[0][2]) / s;
			q.W = (m[1][2] - m[2][1]) / s;
		}
		else if (m[1][1] > m[2][2])
		{
			const float s = sqrtf(1.0f + m[1][1] - m[0][0] - m[2][2]) * 2.0f; // S=4*qy
			q.X = (m[1][0] + m[0][1]) / s;
			q.Y = 0.25f * s;
			q.Z = (m[2][1] + m[1][2]) / s;
			q.W = (m[2][0] - m[0][2]) / s;
		}
		else 
		{
			const float s = sqrtf(1.0f + m[2][2] - m[0][0] - m[1][1]) * 2.0f; // S=4*qz
			q.X = (m[2][0] + m[0][2]) / s;
			q.Y = (m[2][1] + m[1][2]) / s;
			q.Z = 0.25f * s;
			q.W = (m[0][1] - m[1][0]) / s;
		}

		q.Normalize();

		return q;
	}


	NS_NODISCARD_INLINE nsVector3 GetScale() const noexcept
	{
		nsVector3 scale;
		scale.X = nsVector3(M[0][0], M[0][1], M[0][2]).GetMagnitude();
		scale.Y = nsVector3(M[1][0], M[1][1], M[1][2]).GetMagnitude();
		scale.Z = nsVector3(M[2][0], M[2][1], M[2][2]).GetMagnitude();

		return scale;
	}


	NS_INLINE float* operator[](int index)
	{
		NS_ValidateV(index >= 0 && index < 4, "Matrix element index out of range!");
		return M[index];
	}


	NS_INLINE const float* operator[](int index) const
	{
		NS_ValidateV(index >= 0 && index < 4, "Matrix element index out of range!");
		return M[index];
	}


	NS_INLINE nsMatrix4 operator+(const nsMatrix4& rhs) const noexcept
	{
		return Add(*this, rhs);
	}


	NS_INLINE nsMatrix4 operator-(const nsMatrix4& rhs) const noexcept
	{
		return Subtract(*this, rhs);
	}


	NS_INLINE nsMatrix4 operator*(const nsMatrix4& rhs) const noexcept
	{
		return Multiply(*this, rhs);
	}


	NS_INLINE nsMatrix4& operator*=(const nsMatrix4& rhs) noexcept
	{
		*this = Multiply(rhs, *this);
		return *this;
	}


	NS_INLINE friend nsVector4 operator*(const nsVector4& lhs, const nsMatrix4& rhs) noexcept
	{
		return nsMatrix4::Multiply(lhs, rhs);
	}


	NS_INLINE friend nsMatrix4 operator*(const nsMatrix4& lhs, float rhs) noexcept
	{
		return nsMatrix4::Multiply(lhs, rhs);
	}

};



// =============================================================================================================================================================== //
// TRANSFORM
// =============================================================================================================================================================== //
class nsTransform
{
public:
	nsQuaternion Rotation;
	nsVector3 Position;
	nsVector3 Scale;


public:
	nsTransform() noexcept
		: Rotation(nsQuaternion::IDENTITY)
		, Position()
		, Scale(1.0f)
	{
	}


	nsTransform(const nsVector3& position, const nsQuaternion& rotation = nsQuaternion::IDENTITY, const nsVector3& scale = 1.0f)
		: Position(position)
		, Rotation(rotation)
		, Scale(scale)
	{
	}


	NS_NODISCARD_INLINE nsMatrix4 ToMatrix() const noexcept
	{
		return nsMatrix4::Scale(Scale) * nsMatrix4::RotationQuat(Rotation) * nsMatrix4::Translation(Position);
	}


	NS_NODISCARD_INLINE nsMatrix4 ToMatrixNoScale() const noexcept
	{
		return nsMatrix4::RotationQuat(Rotation) * nsMatrix4::Translation(Position);
	}


	NS_NODISCARD_INLINE nsVector3 TransformPosition(const nsVector3& p) const noexcept
	{
		nsVector4 pos = nsVector4(p) * ToMatrixNoScale();

		return nsVector3(pos.X, pos.Y, pos.Z);
	}


	NS_NODISCARD_INLINE nsVector3 InverseTransformPosition(const nsVector3& p) const noexcept
	{
		nsVector4 pos = nsVector4(p) * ToMatrixNoScale().GetInverseTransformNoScale();

		return nsVector3(pos.X, pos.Y, pos.Z);
	}


	NS_NODISCARD_INLINE nsVector3 GetAxisRight() const noexcept
	{
		return nsQuaternion::RotateVector(Rotation, nsVector3::RIGHT);
	}


	NS_NODISCARD_INLINE nsVector3 GetAxisUp() const noexcept
	{
		return nsQuaternion::RotateVector(Rotation, nsVector3::UP);
	}


	NS_NODISCARD_INLINE nsVector3 GetAxisForward() const noexcept
	{
		return nsQuaternion::RotateVector(Rotation, nsVector3::FORWARD);
	}

};



// =============================================================================================================================================================== //
// LINE
// =============================================================================================================================================================== //
class nsLine
{
public:
	nsVector3 A;
	nsVector3 B;


public:
	nsLine() noexcept
	{
	}


	nsLine(const nsVector3& a, const nsVector3& b) noexcept
		: A(a)
		, B(b)
	{
	}


	NS_NODISCARD_INLINE nsVector3 GetDirection() const noexcept
	{
		nsVector3 direction = (B - A);
		direction.Normalize();

		return direction;
	}


	NS_NODISCARD_INLINE float GetLength() const noexcept
	{
		const nsVector3 AB = (B - A);

		return AB.GetMagnitude();
	}


	NS_NODISCARD_INLINE nsVector3 ProjectPoint(const nsVector3& point) const noexcept
	{
		const nsVector3 AB = (B - A);
		const nsVector3 AP = (point - A);
		const float t = nsVector3::DotProduct(AP, AB) / AB.GetMagnitudeSqr();

		return A + (AB * t);
	}


	NS_NODISCARD_INLINE nsVector3 ClosestPoint(const nsVector3& point) const noexcept
	{
		const nsVector3 AB = (B - A);
		const nsVector3 AP = (point - A);
		const float t = nsVector3::DotProduct(AP, AB) / AB.GetMagnitudeSqr();

		if (t <= 0.0f)
		{
			return A;
		}
		else if (t >= 1.0f)
		{
			return B;
		}

		return A + AB * t;
	}


	NS_NODISCARD_INLINE float DistanceSqrPoint(const nsVector3& point) const noexcept
	{
		const nsVector3 AB = (B - A);
		const nsVector3 AP = (point - A);
		const nsVector3 BP = (point - B);
		const float a = nsVector3::DotProduct(AP, AB);

		if (a <= 0.0f)
		{
			return AP.GetMagnitudeSqr();
		}

		const float b = AB.GetMagnitudeSqr();

		if (a >= b)
		{
			return BP.GetMagnitudeSqr();
		}

		return AP.GetMagnitudeSqr() - a * a / b;
	}


	NS_NODISCARD_INLINE float GetShortestDistance(const nsVector3& point) const noexcept
	{
		const nsVector3 normal = (B - A).GetNormalized();
		return nsVector3::DotProduct(normal, point) + nsVector3::Distance(A, B);
	}


	NS_NODISCARD_INLINE float GetShortestDistance(const nsLine& line) const noexcept
	{
		const nsVector3 vecA = GetDirection();
		const nsVector3 vecB = line.GetDirection();
		const nsVector3 AxB = nsVector3::CrossProduct(vecA, vecB);
		const float mag = AxB.GetMagnitude();
		const nsVector3 AB = line.A - A;
		float distance = 0.0f;

		if (mag > 0.0f)
		{
			const float dot = nsVector3::DotProduct(AxB, AB);
			distance = nsMath::Abs(dot / mag);
		}
		else
		{
			const nsVector3 cross = nsVector3::CrossProduct(vecA, AB);
			distance = cross.GetMagnitude() / vecA.GetMagnitude();
		}

		return distance;
	}


	// Computes closest points <p1> and <p2> between lines <lineA> and <lineB>. Returns squared distance between lines
	NS_NODISCARD static NS_INLINE float ClosestPointLines(nsVector3& p1, nsVector3& p2, const nsLine& lineA, const nsLine& lineB) noexcept
	{
		const nsVector3 L1 = (lineA.B - lineA.A);
		const nsVector3 L2 = (lineB.B - lineB.A);
		const nsVector3 R = (lineA.A - lineB.A);
		const float dSqrL1 = L1.GetMagnitudeSqr();
		const float dSqrL2 = L2.GetMagnitudeSqr();
		const float f = nsVector3::DotProduct(L2, R);
		float t1 = 0.0f;
		float t2 = 0.0f;

		if (dSqrL1 <= NS_MATH_EPS && dSqrL2 <= NS_MATH_EPS)
		{
			p1 = lineA.A;
			p2 = lineB.A;

			return (p1 - p2).GetMagnitudeSqr();
		}

		// L1 segment degenerates into a point
		if (dSqrL1 <= NS_MATH_EPS)
		{
			t1 = 0.0f;
			t2 = nsMath::Clamp(f / dSqrL2, 0.0f, 1.0f);
		}
		else
		{
			const float c = nsVector3::DotProduct(L1, R);

			// L2 segment degenerates into a point
			if (dSqrL2 <= NS_MATH_EPS)
			{
				t1 = nsMath::Clamp(-c / dSqrL1, 0.0f, 1.0f);
				t2 = 0.0f;
			}
			else
			{
				const float b = nsVector3::DotProduct(L1, L2);
				const float denom = dSqrL1 * dSqrL2 - b * b;

				if (denom != 0.0f)
				{
					t1 = nsMath::Clamp((b * f - c * dSqrL2) / denom, 0.0f, 1.0f);
				}
				else
				{
					t1 = 0.0f;
				}

				t2 = (b * t1 + f) / dSqrL2;

				if (t2 < 0.0f)
				{
					t1 = nsMath::Clamp(-c / dSqrL1, 0.0f, 1.0f);
					t2 = 0.0f;
				}
				else if (t2 > 1.0f)
				{
					t1 = nsMath::Clamp((b - c) / dSqrL1, 0.0f, 1.0f);
					t2 = 1.0f;
				}
			}
		}

		p1 = lineA.A + L1 * t1;
		p2 = lineB.A + L2 * t2;

		return (p1 - p2).GetMagnitudeSqr();
	}

};



// =============================================================================================================================================================== //
// PLANE
// =============================================================================================================================================================== //
class nsPlane
{
public:
	nsVector3 Normal;
	float Distance;


public:
	nsPlane() noexcept
		: Normal(0.0f)
		, Distance(0.0f)
	{
	}


	nsPlane(const nsVector3& normal, float distance) noexcept
		: Normal(normal.GetNormalized())
		, Distance(distance)
	{
	}


	nsPlane(const nsVector3& normal, const nsVector3& point) noexcept
	{
		Normal = normal.GetNormalized();
		Distance = -nsVector3::DotProduct(Normal, point);
	}


	NS_NODISCARD_INLINE float GetSignedDistancePoint(const nsVector3& point) const noexcept
	{
		return nsVector3::DotProduct(Normal, point) - Distance;
	}


	NS_NODISCARD_INLINE nsVector3 ClosestPoint(const nsVector3& point) const noexcept
	{
		return point - Normal * (nsVector3::DotProduct(Normal, point) + Distance);
	}


	NS_INLINE bool TestIntersectionRay(nsVector3& outIntersectionPoint, const nsVector3& rayOrigin, const nsVector3& rayDirection, float* outT = nullptr) const noexcept
	{
		const float denom = nsVector3::DotProduct(Normal, rayDirection);

		if (nsMath::FloatEquals(denom, 0.0f, NS_MATH_EPS))
		{
			return false;
		}

		const nsVector3 point = -Normal * Distance;
		const float t = -(nsVector3::DotProduct(Normal, rayOrigin) + Distance) / denom;
		outIntersectionPoint = rayOrigin + rayDirection * t;

		if (outT)
		{
			*outT = t;
		}

		return true;
	}

};



// =============================================================================================================================================================== //
// AABB
// =============================================================================================================================================================== //
class nsAABB
{
public:
	nsVector3 Min;
	nsVector3 Max;

public:
	nsAABB() noexcept
	{
	}


	nsAABB(const nsVector3& min, const nsVector3& max) noexcept
		: Min(min)
		, Max(max)
	{
	}


	NS_NODISCARD_INLINE nsVector3 ClosestPoint(const nsVector3& point) const noexcept
	{
		nsVector3 q = point;
		q.X = nsMath::Max(q.X, Min.X);
		q.X = nsMath::Min(q.X, Max.X);
		q.Y = nsMath::Max(q.Y, Min.Y);
		q.Y = nsMath::Min(q.Y, Max.Y);
		q.Z = nsMath::Max(q.Z, Min.Z);
		q.Z = nsMath::Min(q.Z, Max.Z);

		return q;
	}


	NS_NODISCARD nsVector3 ClosestPointLine(const nsLine& line) const noexcept
	{
		float distSqrs[6];
		distSqrs[0] = line.DistanceSqrPoint(Min.X);
		distSqrs[1] = line.DistanceSqrPoint(Min.Y);
		distSqrs[2] = line.DistanceSqrPoint(Min.Z);
		distSqrs[3] = line.DistanceSqrPoint(Max.X);
		distSqrs[4] = line.DistanceSqrPoint(Max.Y);
		distSqrs[5] = line.DistanceSqrPoint(Max.Z);

		float t = 999999999.0f;

		for (int i = 0; i < 6; ++i)
		{
			if (t > distSqrs[i])
			{
				t = distSqrs[i];
			}
		}

		const nsVector3 AB = line.B - line.A;
		t /= AB.GetMagnitudeSqr();

		return line.A + AB * t;
	}


	NS_NODISCARD_INLINE float DistanceSqrPoint(const nsVector3& point) const noexcept
	{
		float d = 0.0f;

		if (point.X < Min.X) d += (Min.X - point.X) * (Min.X - point.X);
		if (point.X > Max.X) d += (point.X - Max.X) * (point.X - Max.X);
		if (point.Y < Min.Y) d += (Min.Y - point.Y) * (Min.Y - point.Y);
		if (point.Y > Max.Y) d += (point.Y - Max.Y) * (point.Y - Max.Y);
		if (point.Z < Min.Z) d += (Min.Z - point.Z) * (Min.Z - point.Z);
		if (point.Z > Max.Z) d += (point.Z - Max.Z) * (point.Z - Max.Z);

		return d;
	}

};



namespace nsMath
{
	NS_NODISCARD static NS_INLINE nsVector3 MinVector3(const nsVector3& a, const nsVector3& b) noexcept
	{
		return nsVector3(
			nsMath::Min(a.X, b.X),
			nsMath::Min(a.Y, b.Y),
			nsMath::Min(a.Z, b.Z)
		);
	}


	NS_NODISCARD static NS_INLINE nsVector3 MaxVector3(const nsVector3& a, const nsVector3& b) noexcept
	{
		return nsVector3(
			nsMath::Max(a.X, b.X),
			nsMath::Max(a.Y, b.Y),
			nsMath::Max(a.Z, b.Z)
		);
	}


	NS_NODISCARD static NS_INLINE nsVector3 ClampVector3(const nsVector3& value, const nsVector3& minValue, const nsVector3& maxValue) noexcept
	{
		return nsVector3(
			nsMath::Clamp(value.X, minValue.X, maxValue.X), 
			nsMath::Clamp(value.Y, minValue.Y, maxValue.Y), 
			nsMath::Clamp(value.Z, minValue.Z, maxValue.Z)
		);
	}


	NS_NODISCARD static NS_INLINE nsVector3 ClampVector3Degrees(const nsVector3& pitchYawRoll) noexcept
	{
		return nsVector3(
			nsMath::ClampDegree(pitchYawRoll.X),
			nsMath::ClampDegree(pitchYawRoll.Y),
			nsMath::ClampDegree(pitchYawRoll.Z)
		);
	}


	NS_NODISCARD static NS_INLINE nsVector3 FloorVector3(const nsVector3& value) noexcept
	{
		return nsVector3(
			nsMath::Floor(value.X),
			nsMath::Floor(value.Y),
			nsMath::Floor(value.Z)
		);
	}

};
