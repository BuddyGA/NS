#pragma once

#ifdef __NS_CORE_BUILD__
#define NS_CORE_API __declspec(dllexport)
#else
#define NS_CORE_API __declspec(dllimport)
#endif // __NS_CORE_BUILD__


#define NS_NODISCARD			[[nodiscard]]
#define NS_INLINE				inline
#define NS_NODISCARD_INLINE		NS_NODISCARD NS_INLINE


#define NS_MEMORY_SIZE_KiB(n)	(n * 1024)
#define NS_MEMORY_SIZE_MiB(n)	(NS_MEMORY_SIZE_KiB(n) * 1024)
#define NS_MEMORY_SIZE_GiB(n)	(NS_MEMORY_SIZE_MiB(n) * 1024)


#define NS_DECLARE_NOCOPY(type) \
private: \
	type(const type&) = delete; \
	type& operator=(const type&) = delete;


#define NS_DECLARE_NOCOPY_NOMOVE(type) \
NS_DECLARE_NOCOPY(type) \
type(type&&) = delete; \
type& operator=(type&&) = delete;


#define NS_DECLARE_SINGLETON(type) \
NS_DECLARE_NOCOPY_NOMOVE(type) \
type() noexcept; \
public: \
static NS_INLINE type& Get() noexcept { static type _singleton; return _singleton; }



#include <stdarg.h>
#include <stdio.h>
#include <utility>
#include <new>
#include <float.h>


typedef unsigned char uint8;
static_assert(sizeof(uint8) == 1, "Size of uint8 is not 1 byte!");

typedef unsigned short uint16;
static_assert(sizeof(uint16) == 2, "Size of uint16 is not 2 bytes!");

typedef unsigned int uint32;
static_assert(sizeof(uint32) == 4, "Size of uint32 is not 4 bytes!");

typedef unsigned long long uint64;
static_assert(sizeof(uint64) == 8, "Size of uint64 is not 8 bytes!");

typedef signed char int8;
static_assert(sizeof(int8) == 1, "Size of int8 is not 1 byte!");

typedef signed short int16;
static_assert(sizeof(int16) == 2, "Size of int16 is not 2 bytes!");

typedef signed int int32;
static_assert(sizeof(int32) == 4, "Size of int32 is not 4 bytes!");

typedef signed long long int64;
static_assert(sizeof(int64) == 8, "Size of int64 is not 8 bytes!");


template<typename T>
class nsTPoint
{
	static_assert(std::is_arithmetic<T>::value, "nsTPoint type of <T> must be arithmetic!");

public:
	T X;
	T Y;

public:
	nsTPoint() noexcept
		: X(0)
		, Y(0)
	{
	}

	nsTPoint(T v) noexcept
		: X(v)
		, Y(v)
	{
	}

	nsTPoint(T x, T y) noexcept
		: X(x)
		, Y(y)
	{
	}

	
	template<typename U>
	nsTPoint(nsTPoint<U> other)
	{
		X = static_cast<T>(other.X);
		Y = static_cast<T>(other.Y);
	}

	nsTPoint(const nsTPoint&) = default;
	nsTPoint& operator=(const nsTPoint&) = default;
	nsTPoint(nsTPoint&&) = default;
	nsTPoint& operator=(nsTPoint&&) = default;


public:
	NS_INLINE bool operator==(const nsTPoint& rhs) const noexcept
	{
		return X == rhs.X && Y == rhs.Y;
	}


	NS_INLINE bool operator!=(const nsTPoint& rhs) const noexcept
	{
		return !(*this == rhs);
	}

	NS_INLINE nsTPoint operator+(const nsTPoint& rhs) const noexcept
	{
		return nsTPoint(X + rhs.X, Y + rhs.Y);
	}

	NS_INLINE nsTPoint& operator+=(const nsTPoint& rhs) noexcept
	{
		X += rhs.X;
		Y += rhs.Y;
		return *this;
	}

	NS_INLINE nsTPoint operator+(T rhs) const noexcept
	{
		return nsTPoint(X + rhs, Y + rhs);
	}

	NS_INLINE nsTPoint& operator+=(T rhs) noexcept
	{
		X += rhs;
		Y += rhs;
		return *this;
	}

	NS_INLINE nsTPoint operator-(const nsTPoint& rhs) const noexcept
	{
		return nsTPoint(X - rhs.X, Y - rhs.Y);
	}

	NS_INLINE nsTPoint& operator-=(const nsTPoint& rhs) noexcept
	{
		X -= rhs.X;
		Y -= rhs.Y;
		return *this;
	}

	NS_INLINE nsTPoint operator-(T rhs) const noexcept
	{
		return nsTPoint(X - rhs, Y - rhs);
	}

	NS_INLINE nsTPoint& operator-=(T rhs) noexcept
	{
		X -= rhs;
		Y -= rhs;
		return *this;
	}

	NS_INLINE nsTPoint operator*(const nsTPoint& rhs) const noexcept
	{
		return nsTPoint(X * rhs.X, Y * rhs.Y);
	}

	NS_INLINE nsTPoint& operator*=(const nsTPoint& rhs) noexcept
	{
		X *= rhs.X;
		Y *= rhs.Y;
		return *this;
	}

	NS_INLINE nsTPoint operator*(T rhs) const noexcept
	{
		return nsTPoint(X * rhs, Y * rhs);
	}

	NS_INLINE nsTPoint& operator*=(T rhs) noexcept
	{
		X *= rhs;
		Y *= rhs;
		return *this;
	}

};

typedef nsTPoint<int> nsPointInt;
typedef nsTPoint<float> nsPointFloat;



template<typename TOffset, typename TSize>
class nsTRect
{
	static_assert(std::is_arithmetic<TOffset>::value && std::is_arithmetic<TSize>::value, "nsTRect type of <TOffset> and <TSize> must be arithmetic!");

public:
	TOffset X;
	TOffset Y;
	TSize Width;
	TSize Height;

public:
	nsTRect() noexcept
		: X(0)
		, Y(0)
		, Width(0)
		, Height(0)
	{
	}

	nsTRect(TOffset x, TOffset y, TSize w, TSize h) noexcept
		: X(x)
		, Y(y)
		, Width(w)
		, Height(h)
	{
	}

	nsTRect(const nsTRect&) = default;
	nsTRect& operator=(const nsTRect&) = default;
	nsTRect(nsTRect&&) = default;
	nsTRect& operator=(nsTRect&&) = default;

};

typedef nsTRect<int, int> nsRectInt;
typedef nsTRect<float, float> nsRectFloat;



NS_NODISCARD_INLINE uint64 ns_GetHash(int value) noexcept
{
	return static_cast<uint64>(value);
}


template<typename T>
NS_NODISCARD_INLINE uint64 ns_GetHash(const T* value) noexcept
{
	return reinterpret_cast<uint64>(value);
}
