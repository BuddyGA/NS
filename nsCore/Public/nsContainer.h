#pragma once

#include "nsPlatform.h"
#include <initializer_list>



#define NS_ARRAY_INDEX_INVALID		(-1)
#define NS_ARRAY_INDEX_FIRST		(0)
#define NS_ARRAY_INDEX_LAST			(INT32_MAX - 1)
#define NS_ARRAY_MAX_COUNT			(INT32_MAX)
#define NS_ARRAY_ValidateIndex(i)	NS_ValidateV(i >= 0 && i < Count, TEXT("Array index [%i] is out of bound!"), i)



template<typename T>
class nsTArray
{
private:
	int Capacity;
	int Count;
	T* Data;


public:
	nsTArray(int count = 0) noexcept
		: Capacity(0)
		, Count(0)
		, Data(nullptr)
	{
		if (count > 0)
		{
			Resize(count);
		}
	}


	template<typename...TConstructorArgs>
	nsTArray(int count = 0, TConstructorArgs&&... args) noexcept
		: Capacity(0)
		, Count(0)
		, Data(nullptr)
	{
		if (count > 0)
		{
			ResizeConstructs(count, std::forward<TConstructorArgs>(args)...);
		}
	}


	nsTArray(const nsTArray& other) noexcept
		: Capacity(0)
		, Count(0)
		, Data(nullptr)
	{
		//nsPlatform::Output("nsTArray copy constructor\n");

		Resize(other.Count);
		CopyElements(0, other.Data, other.Count);
	}


	nsTArray(nsTArray&& other) noexcept
		: Capacity(other.Capacity)
		, Count(other.Count)
		, Data(other.Data)
	{
		//nsPlatform::Output("nsTArray move constructor\n");

		other.Data = nullptr;
		other.Capacity = 0;
		other.Count = 0;
	}


	nsTArray(const std::initializer_list<T>& initializerList) noexcept
		: Capacity(0)
		, Count(0)
		, Data(nullptr)
	{
		//nsPlatform::Output("nsTArray initializeList constructor\n");

		const int count = static_cast<int>(initializerList.size());
		Resize(count);
		CopyElements(0, initializerList.begin(), count);
	}


	~nsTArray() noexcept
	{
		//nsPlatform::Output("nsTArray destructor\n");

		Clear(true);
	}


private:
	NS_INLINE void CopyElements(int dstIndex, const T* source, int count) noexcept
	{
		if (source == nullptr || count <= 0)
		{
			return;
		}

		NS_ARRAY_ValidateIndex(dstIndex);

		const int endIndex = dstIndex + count;
		NS_ARRAY_ValidateIndex(endIndex - 1);

		if constexpr (std::is_arithmetic<T>::value)
		{
			nsPlatform::Memory_Copy(Data + dstIndex, source, sizeof(T) * count);
		}
		else
		{
			for (int i = 0; i < count; ++i)
			{
				Data[dstIndex + i] = source[i];
			}
		}
	}


	NS_INLINE void ShiftElements(int dstIndex, int srcIndex, int count) noexcept
	{
		NS_ARRAY_ValidateIndex(dstIndex);
		NS_ARRAY_ValidateIndex(srcIndex);
		NS_ARRAY_ValidateIndex(dstIndex + count - 1);
		NS_ARRAY_ValidateIndex(srcIndex + count - 1);

		nsPlatform::Memory_Move(Data + dstIndex, Data + srcIndex, sizeof(T) * count);
	}


	template<typename...TConstructorArgs>
	NS_INLINE void Constructs(int startIndex, int count, TConstructorArgs&&... args) noexcept
	{
		const int endIndex = startIndex + count;

		for (int i = startIndex; i < endIndex; ++i)
		{
			NS_ARRAY_ValidateIndex(i);
			new (Data + i)T(std::forward<TConstructorArgs>(args)...);
		}
	}


	NS_INLINE void Destructs(int startIndex, int count) noexcept
	{
		const int endIndex = startIndex + count;

		for (int i = startIndex; i < endIndex; ++i)
		{
			NS_ARRAY_ValidateIndex(i);
			(Data + i)->~T();
		}
	}


public:
	NS_INLINE void Reserve(int newCapacity) noexcept
	{
		if (Capacity >= newCapacity)
		{
			return;
		}

		Data = static_cast<T*>(nsPlatform::Memory_Realloc(Data, sizeof(T) * newCapacity));
		NS_ValidateV(Data, TEXT("nsTArray memory realloc failed!"));
		Capacity = newCapacity;
	}


	NS_INLINE void Resize(int newCount) noexcept
	{
		NS_Assert(newCount >= 0 && newCount < INT32_MAX);

		if (Count == newCount)
		{
			return;
		}

		const int lastIndex = Count;

		if (newCount > Count)
		{
			Reserve(newCount);
			Count = newCount;
			nsPlatform::Memory_Set(Data + lastIndex, 0, sizeof(T) * (newCount - lastIndex));
		}
		else
		{
			if constexpr (!std::is_arithmetic<T>::value)
			{
				Destructs(newCount, lastIndex - newCount);
			}

			Count = newCount;
		}
	}


	template<typename...TConstructorArgs>
	NS_INLINE void ResizeConstructs(int newCount, TConstructorArgs&&... args) noexcept
	{
		NS_Assert(newCount >= 0 && newCount < INT32_MAX);

		if (Count == newCount)
		{
			return;
		}

		NS_Assert(newCount > Count);

		Reserve(newCount);
		const int lastIndex = Count;
		Count = newCount;
		Constructs(lastIndex, newCount - lastIndex, std::forward<TConstructorArgs>(args)...);
	}


	NS_NODISCARD_INLINE int Find(const T& value) const noexcept
	{
		for (int i = 0; i < Count; ++i)
		{
			if (Data[i] == value)
			{
				return i;
			}
		}

		return NS_ARRAY_INDEX_INVALID;
	}


	template<typename U>
	NS_NODISCARD_INLINE int Find(const U& value) const noexcept
	{
		for (int i = 0; i < Count; ++i)
		{
			if (Data[i] == value)
			{
				return i;
			}
		}

		return NS_ARRAY_INDEX_INVALID;
	}


	template<typename U, typename TComparePredicate>
	NS_NODISCARD_INLINE int Find(const U& compareValue, TComparePredicate compare) const noexcept
	{
		for (int i = 0; i < Count; ++i)
		{
			if (compare(Data[i], compareValue))
			{
				return i;
			}
		}

		return NS_ARRAY_INDEX_INVALID;
	}


	NS_NODISCARD_INLINE int FindLast(const T& value) const noexcept
	{
		for (int i = Count - 1; i >= 0; --i)
		{
			if (Data[i] == value)
			{
				return i;
			}
		}

		return NS_ARRAY_INDEX_INVALID;
	}


	template<typename U, typename TComparePredicate>
	NS_NODISCARD_INLINE int FindLast(const U& compareValue, TComparePredicate compare) const noexcept
	{
		for (int i = Count - 1; i >= 0; --i)
		{
			if (compare(Data[i], compareValue))
			{
				return i;
			}
		}

		return NS_ARRAY_INDEX_INVALID;
	}


	template<typename...TConstructorArgs>
	NS_INLINE T& Add(TConstructorArgs&&... args) noexcept
	{
		ResizeConstructs(Count + 1, std::forward<TConstructorArgs>(args)...);
		return Data[Count - 1];
	}


	NS_INLINE bool AddUnique(const T& value, int* outIndex = nullptr) noexcept
	{
		int index = Find(value);
		bool bAdded = false;

		if (index == NS_ARRAY_INDEX_INVALID)
		{
			index = Count;
			Add(value);
			bAdded = true;
		}

		if (outIndex)
		{
			*outIndex = index;
		}

		return bAdded;
	}


	NS_INLINE void InsertAt(const T* source, int count, int dstIndex = NS_ARRAY_INDEX_LAST) noexcept
	{
		if (source == nullptr || count <= 0)
		{
			return;
		}

		if ( (Count == 0) || (dstIndex == Count - 1) || (dstIndex == NS_ARRAY_INDEX_LAST) )
		{
			const int lastIndex = Count;
			Resize(Count + count);
			CopyElements(lastIndex, source, count);
		}
		else
		{
			const int PrevCount = Count;
			NS_Assert(PrevCount > dstIndex);

			Resize(Count + count);
			ShiftElements(dstIndex + count, dstIndex, PrevCount - dstIndex);
			CopyElements(dstIndex, source, count);
		}
	}


	NS_INLINE void InsertAt(const nsTArray& other, int dstIndex = NS_ARRAY_INDEX_LAST) noexcept
	{
		InsertAt(other.Data, other.Count, dstIndex);
	}


	NS_INLINE void InsertAt(const std::initializer_list<T>& initializerList, int dstIndex = NS_ARRAY_INDEX_LAST) noexcept
	{
		InsertAt(initializerList.begin(), static_cast<int>(initializerList.size()), dstIndex);
	}


	NS_INLINE void InsertAt(const T& value, int index = NS_ARRAY_INDEX_LAST) noexcept
	{
		InsertAt(&value, 1, index);
	}


	NS_INLINE bool RemoveAt(int index, bool bKeepOrder = true) noexcept
	{
		if (Count == 0)
		{
			return false;
		}

		if (index == NS_ARRAY_INDEX_LAST)
		{
			index = Count - 1;
		}

		NS_ARRAY_ValidateIndex(index);

		if constexpr (!std::is_arithmetic<T>::value)
		{
			Destructs(index, 1);
		}

		if (Count == 1 || index == (Count - 1) || index == NS_ARRAY_INDEX_LAST)
		{
			Count--;
			return true;
		}

		if (bKeepOrder)
		{
			const int shiftSrcIndex = index + 1;
			ShiftElements(index, shiftSrcIndex, Count - shiftSrcIndex);
		}
		else
		{
			Data[index] = Data[Count - 1];
		}

		Count--;

		return true;
	}


	NS_INLINE bool RemoveAtRange(int index, int count = NS_ARRAY_MAX_COUNT) noexcept
	{
		if (Count == 0)
		{
			return false;
		}

		if (Count == 1)
		{
			return RemoveAt(index);
		}

		if (count == NS_ARRAY_MAX_COUNT)
		{
			count = Count - index;
		}

		NS_ARRAY_ValidateIndex(index);
		NS_Assert(index + count <= Count);

		if constexpr (!std::is_arithmetic<T>::value)
		{
			Destructs(index, count);
		}

		if (Count > (index + count))
		{
			const int shiftSrcIndex = index + count;
			ShiftElements(index, shiftSrcIndex, Count - shiftSrcIndex);
		}

		Count -= count;

		return true;
	}


	NS_INLINE bool Remove(const T& value, bool bKeepOrder = true) noexcept
	{
		const int index = Find(value);

		if (index != NS_ARRAY_INDEX_INVALID)
		{
			RemoveAt(index, bKeepOrder);
			return true;
		}

		return false;
	}


	template<typename U, typename TComparePredicate>
	NS_INLINE bool Remove(const U& compareValue, TComparePredicate compare, bool bKeepOrder = true) noexcept
	{
		const int index = Find(compareValue, compare);

		if (index != NS_ARRAY_INDEX_INVALID)
		{
			RemoveAt(index, bKeepOrder);
			return true;
		}

		return false;
	}


	NS_INLINE void Clear(bool bFree = false) noexcept
	{
		Resize(0);

		if (Data && bFree)
		{
			nsPlatform::Memory_Free(Data);
			Data = nullptr;
			Capacity = 0;
		}
	}


	NS_NODISCARD_INLINE T* GetData() noexcept
	{
		return Data;
	}


	NS_NODISCARD_INLINE const T* GetData() const noexcept
	{
		return Data;
	}


	NS_NODISCARD_INLINE int GetCount() const noexcept
	{
		return Count;
	}


	NS_NODISCARD_INLINE bool IsEmpty() const noexcept
	{
		return Count == 0;
	}


	NS_NODISCARD_INLINE int GetTotalSizeBytes() const noexcept
	{
		return sizeof(T) * Capacity;
	}


	NS_NODISCARD_INLINE int GetAllocatedSizeBytes() const noexcept
	{
		return sizeof(T) * Count;
	}


public:
	NS_INLINE nsTArray& operator=(const nsTArray& rhs) noexcept
	{
		if (this != &rhs)
		{
			//nsPlatform::Output("nsTArray copy assignment\n");

			Resize(rhs.Count);
			CopyElements(0, rhs.Data, rhs.Count);
		}

		return *this;
	}


	NS_INLINE nsTArray& operator=(nsTArray&& rhs) noexcept
	{
		if (this != &rhs)
		{
			//nsPlatform::Output("nsTArray move assignment\n");

			Clear(true);
			Data = rhs.Data;
			Capacity = rhs.Capacity;
			Count = rhs.Count;
			rhs.Data = nullptr;
			rhs.Capacity = 0;
			rhs.Count = 0;
		}

		return *this;
	}


	NS_INLINE nsTArray& operator=(const std::initializer_list<T>& rhs) noexcept
	{
		const int count = static_cast<int>(rhs.size());
		Resize(count);
		CopyElements(0, rhs.begin(), count);

		return *this;
	}


	NS_INLINE T& operator[](int index) noexcept
	{
		NS_ARRAY_ValidateIndex(index);
		return Data[index];
	}


	NS_INLINE const T& operator[](int index) const noexcept
	{
		NS_ARRAY_ValidateIndex(index);
		return Data[index];
	}

};




template<typename T, int CAPACITY>
class nsTArrayInline
{
	static_assert(CAPACITY > 1, "nsTArrayInline <CAPACITY> must be greater than 1!");

private:
	int Count;
	T Data[CAPACITY];


public:
	nsTArrayInline() noexcept
		: Count(0)
		, Data()
	{
	}


	nsTArrayInline(const nsTArrayInline& other) noexcept
		: Count(0)
		, Data()
	{
		Resize(other.Count);
		CopyElements(0, other.Data, other.Count);
	}


	nsTArrayInline(nsTArrayInline&&) noexcept = default;
	nsTArrayInline& operator=(nsTArrayInline&&) noexcept = default;
	~nsTArrayInline() noexcept = default;


private:
	NS_INLINE void CopyElements(int dstIndex, const T* source, int count) noexcept
	{
		if (source == nullptr || count == 0)
		{
			return;
		}

		NS_ARRAY_ValidateIndex(dstIndex);

		const int endIndex = dstIndex + count;
		NS_ARRAY_ValidateIndex(endIndex - 1);

		if constexpr (std::is_arithmetic<T>::value)
		{
			nsPlatform::Memory_Copy(Data + dstIndex, source, sizeof(T) * count);
		}
		else
		{
			for (int i = 0; i < count; ++i)
			{
				Data[dstIndex + i] = source[i];
			}
		}
	}


	NS_INLINE void ShiftElements(int dstIndex, int srcIndex, int count) noexcept
	{
		NS_ARRAY_ValidateIndex(dstIndex);
		NS_ARRAY_ValidateIndex(srcIndex);
		NS_ARRAY_ValidateIndex(dstIndex + count - 1);
		NS_ARRAY_ValidateIndex(srcIndex + count - 1);

		nsPlatform::Memory_Move(Data + dstIndex, Data + srcIndex, sizeof(T) * count);
	}


public:
	NS_INLINE void Resize(int newCount) noexcept
	{
		if (newCount == Count)
		{
			return;
		}

		NS_ValidateV(newCount <= CAPACITY, TEXT("nsTArrayInline exceeds capacity!"));
		const int lastIndex = Count;

		if (newCount > Count)
		{
			Count = newCount;
			nsPlatform::Memory_Set(Data + lastIndex, 0, sizeof(T) * (newCount - lastIndex));
		}
		else
		{
			nsPlatform::Memory_Zero(Data + newCount, sizeof(T) * (lastIndex - newCount));
			Count = newCount;
		}
	}


	NS_NODISCARD_INLINE int Find(const T& value) const noexcept
	{
		for (int i = 0; i < Count; ++i)
		{
			if (Data[i] == value)
			{
				return i;
			}
		}

		return NS_ARRAY_INDEX_INVALID;
	}


	template<typename U>
	NS_NODISCARD_INLINE int Find(const U& compareValue) const noexcept
	{
		for (int i = 0; i < Count; ++i)
		{
			if (Data[i] == compareValue)
			{
				return i;
			}
		}

		return NS_ARRAY_INDEX_INVALID;
	}


	template<typename...TConstructorArgs>
	NS_INLINE T& Add(TConstructorArgs&&... args) noexcept
	{
		const int index = Count;
		Resize(Count + 1);
		Data[index] = T(std::forward<TConstructorArgs>(args)...);

		return Data[index];
	}


	NS_INLINE void Add(const T& value) noexcept
	{
		Resize(Count + 1);
		Data[Count - 1] = value;
	}


	NS_INLINE bool AddUnique(const T& value, int* outIndex = nullptr) noexcept
	{
		int index = Find(value);
		bool bAdded = false;

		if (index == NS_ARRAY_INDEX_INVALID)
		{
			index = Count;
			Add(value);
			bAdded = true;
		}

		if (outIndex)
		{
			*outIndex = index;
		}

		return bAdded;
	}


	NS_INLINE void RemoveAt(int index, bool bKeepOrder = true) noexcept
	{
		if (Count == 0)
		{
			return;
		}

		if (index == NS_ARRAY_INDEX_LAST)
		{
			index = Count - 1;
		}

		NS_ARRAY_ValidateIndex(index);

		if (Count == 1 || index == (Count - 1) || index == NS_ARRAY_INDEX_LAST)
		{
			Count--;
			return;
		}

		if (bKeepOrder)
		{
			const int shiftSrcIndex = index + 1;
			ShiftElements(index, shiftSrcIndex, Count - shiftSrcIndex);
		}
		else
		{
			Data[index] = Data[Count - 1];
		}

		Count--;
	}


	NS_INLINE bool Remove(const T& value, bool bKeepOrder = true) noexcept
	{
		const int index = Find(value);

		if (index != NS_ARRAY_INDEX_INVALID)
		{
			RemoveAt(index, bKeepOrder);
			return true;
		}

		return false;
	}


	NS_INLINE void Clear() noexcept
	{
		Count = 0;
	}


	NS_NODISCARD_INLINE T* GetData() noexcept
	{
		return Data;
	}


	NS_NODISCARD_INLINE const T* GetData() const noexcept
	{
		return Data;
	}


	NS_NODISCARD_INLINE int GetCount() const noexcept
	{
		return Count;
	}


	NS_NODISCARD_INLINE bool IsEmpty() const noexcept
	{
		return Count == 0;
	}


	NS_NODISCARD_INLINE nsTArray<T> ToArray() const noexcept
	{
		nsTArray<T> arr;

		if (Count > 0)
		{
			arr.InsertAt(Data, Count);
		}

		return arr;
	}


public:
	NS_INLINE nsTArrayInline& operator=(const nsTArrayInline& rhs) noexcept
	{
		if (this != &rhs)
		{
			Resize(rhs.Count);
			CopyElements(0, rhs.Data, rhs.Count);
		}

		return *this;
	}


	NS_INLINE T& operator[](int index) noexcept
	{
		NS_ARRAY_ValidateIndex(index);
		return Data[index];
	}


	NS_INLINE const T& operator[](int index) const noexcept
	{
		NS_ARRAY_ValidateIndex(index);
		return Data[index];
	}

};




template<typename TKey, typename TValue>
class nsTArrayPair
{
private:
	nsTArray<TKey> Keys;
	nsTArray<TValue> Values;


public:
	nsTArrayPair() noexcept = default;
	~nsTArrayPair() noexcept = default;


	nsTArrayPair(const nsTArrayPair& other) noexcept
		: Keys(other.Keys)
		, Values(other.Values)
	{
	}


	nsTArrayPair(nsTArrayPair&& other) noexcept
		: Keys(std::move(other.Keys))
		, Values(std::move(other.Values))
	{
	}


	NS_INLINE void Reserve(int newCapacity) noexcept
	{
		Keys.Reserve(newCapacity);
		Values.Reserve(newCapacity);
	}


	NS_INLINE void Resize(int newCount) noexcept
	{
		Keys.Resize(newCount);
		Values.Resize(newCount);
	}


	NS_NODISCARD_INLINE int Find(const TKey& key) const noexcept
	{
		return Keys.Find(key);
	}


	NS_INLINE TValue& Add(const TKey& key, int* outIndex = nullptr) noexcept
	{
		Keys.Add(key);
		return Values.Add();
	}


	template<typename...TConstructorArgs>
	NS_INLINE TValue& Add(const TKey& key, TConstructorArgs&&... args) noexcept
	{
		Keys.Add(key);
		return Values.Add(std::forward<TConstructorArgs>(args)...);
	}


	NS_INLINE bool Remove(const TKey& key, bool bKeepOrder = true) noexcept
	{
		const int index = Keys.Find(key);

		if (index != NS_ARRAY_INDEX_INVALID)
		{
			Keys.RemoveAt(index, bKeepOrder);
			Values.RemoveAt(index, bKeepOrder);

			return true;
		}

		return false;
	}


	NS_INLINE int RemoveAll(const TKey& key, bool bKeepOrder = true) noexcept
	{
		int removeCount = 0;

		for (int i = 0; i < Keys.GetCount();)
		{
			if (Keys[i] == key)
			{
				Keys.RemoveAt(i, bKeepOrder);
				Values.RemoveAt(i, bKeepOrder);
			}
			else
			{
				++i;
			}
		}

		return removeCount;
	}


	NS_INLINE void Clear(bool bFree = false) noexcept
	{
		Keys.Clear(bFree);
		Values.Clear(bFree);
	}


	NS_NODISCARD_INLINE const nsTArray<TKey>& GetKeys() const noexcept
	{
		return Keys;
	}


	NS_NODISCARD_INLINE TKey& GetKeyByIndex(int index) noexcept
	{
		return Keys[index];
	}


	NS_NODISCARD_INLINE const TKey& GetKeyByIndex(int index) const noexcept
	{
		return Keys[index];
	}


	NS_NODISCARD_INLINE const nsTArray<TValue>& GetValues() const noexcept
	{
		return Values;
	}


	NS_NODISCARD_INLINE TValue& GetValueByIndex(int index) noexcept
	{
		return Values[index];
	}


	NS_NODISCARD_INLINE const TValue& GetValueByIndex(int index) const noexcept
	{
		return Values[index];
	}


	NS_NODISCARD_INLINE TValue* GetValueByKey(const TKey& key) noexcept
	{
		const int index = Find(key);

		if (index == NS_ARRAY_INDEX_INVALID)
		{
			return nullptr;
		}

		return &Values[index];
	}


	NS_NODISCARD_INLINE const TValue* GetValueByKey(const TKey& key) const noexcept
	{
		const int index = Find(key);

		if (index == NS_ARRAY_INDEX_INVALID)
		{
			return nullptr;
		}

		return &Values[index];
	}


	NS_NODISCARD_INLINE int GetCount() const noexcept
	{
		return Keys.GetCount();
	}


	NS_NODISCARD_INLINE bool IsEmpty() const noexcept
	{
		return Keys.IsEmpty();
	}


public:
	NS_INLINE nsTArrayPair& operator=(const nsTArrayPair& rhs) noexcept
	{
		if (this != &rhs)
		{
			Keys = rhs.Keys;
			Values = rhs.Values;
		}

		return *this;
	}


	NS_INLINE nsTArrayPair& operator=(nsTArrayPair&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Keys = std::move(rhs.Keys);
			Values = std::move(rhs.Values);
		}

		return *this;
	}

};




template<typename T>
class nsTArrayFreeList
{
	static_assert(sizeof(T) >= sizeof(int), "nsTArrayFreeList sizeof(T) must be >= than sizeof(int)!");

private:
	int Count;
	int NextFreeIndex;
	nsTArray<T> Array;


public:
	nsTArrayFreeList() noexcept
		: Count(0)
		, NextFreeIndex(NS_ARRAY_INDEX_INVALID)
	{
	}


	nsTArrayFreeList(const nsTArrayFreeList& other) noexcept
		: Count(other.Count)
		, NextFreeIndex(other.NextFreeIndex)
		, Array(other.Array)
	{
	}


	nsTArrayFreeList(nsTArrayFreeList&& other) noexcept
		: Count(other.Count)
		, NextFreeIndex(other.NextFreeIndex)
		, Array(std::move(other.Array))
	{
	}


	nsTArrayFreeList(const std::initializer_list<T>& initializerList) noexcept
		: Count(0)
		, NextFreeIndex(NS_ARRAY_INDEX_INVALID)
	{
		const int count = static_cast<int>(initializerList.size());
		Array.Reserve(count);

		for (int i = 0; i < count; ++i)
		{
			Add(*(initializerList.begin() + i));
		}
	}


	~nsTArrayFreeList() noexcept = default;


public:
	NS_NODISCARD_INLINE bool IsValid(int index) const noexcept
	{
		if (index < 0 || index >= Array.GetCount() || index == NextFreeIndex)
		{
			return false;
		}

		int check = NextFreeIndex;

		while (check != NS_ARRAY_INDEX_INVALID)
		{
			if (check == index)
			{
				return false;
			}

			check = *(const int*)&Array[check];
		}

		return true;
	}


	NS_INLINE void Reserve(int newCapacity) noexcept
	{
		Array.Reserve(newCapacity);
	}


	template<typename... TConstructorArgs>
	NS_INLINE int Add(TConstructorArgs&&... args) noexcept
	{
		int index = NS_ARRAY_INDEX_INVALID;

		if (NextFreeIndex == NS_ARRAY_INDEX_INVALID)
		{
			index = Array.GetCount();
			Array.Add(std::forward<TConstructorArgs>(args)...);
		}
		else
		{
			index = NextFreeIndex;
			NextFreeIndex = *(const int*)&Array[index];
			new (&Array[index])T(std::forward<TConstructorArgs>(args)...);
		}

		Count++;

		return index;
	}


	NS_INLINE void RemoveAt(int index)
	{
		NS_ValidateV(IsValid(index), TEXT("nsTArrayFreeList element at index %i is not valid!"), index);

		T& data = Array[index];
		data.~T();
		nsPlatform::Memory_Set(&Array[index], 0xCC, sizeof(int));
		*(int*)&Array[index] = NextFreeIndex;
		NextFreeIndex = index;
		Count--;
	}


	NS_INLINE void Clear(bool bFree = false) noexcept
	{
		Array.Clear(bFree);
		Count = 0;
		NextFreeIndex = NS_ARRAY_INDEX_INVALID;
	}


	NS_NODISCARD_INLINE const nsTArray<T>& GetArray() const noexcept
	{
		return Array;
	}


	NS_NODISCARD_INLINE int GetCount() const noexcept
	{
		return Count;
	}


	NS_NODISCARD_INLINE bool IsEmpty() const noexcept
	{
		return Count == 0;
	}


public:
	NS_INLINE nsTArrayFreeList& operator=(const nsTArrayFreeList& rhs) noexcept
	{
		if (this != &rhs)
		{
			Array = rhs.Array;
			Count = rhs.Count;
			NextFreeIndex = rhs.NextFreeIndex;
		}

		return *this;
	}


	NS_INLINE nsTArrayFreeList& operator=(nsTArrayFreeList&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Array = std::move(rhs.Array);
			Count = rhs.Count;
			NextFreeIndex = rhs.NextFreeIndex;
		}

		return *this;
	}


	NS_INLINE nsTArrayFreeList& operator=(const std::initializer_list<T>& initializerList) noexcept
	{
		const int count = static_cast<int>(initializerList.size());
		Array.Reserve(count);

		for (int i = 0; i < count; ++i)
		{
			Add(*(initializerList.begin() + i));
		}
	}


	inline T& operator[](int index)
	{
		NS_ValidateV(IsValid(index), TEXT("nsTArrayFreeList element at index %i is not valid!"), index);
		return Array[index];
	}


	inline const T& operator[](int index) const
	{
		NS_ValidateV(IsValid(index), TEXT("nsTArrayFreeList element at index %i is not valid!"), index);
		return Array[index];
	}


public:
	class Iterator
	{
		nsTArrayFreeList* Data;
		int Index;

	public:
		Iterator() noexcept
			: Data(nullptr)
			, Index(-1)
		{
		}

		Iterator(nsTArrayFreeList* data) noexcept
			: Data(data)
			, Index(-1)
		{
			IterateNextValidIndex();
		}

	private:
		inline void IterateNextValidIndex() noexcept
		{
			const int lastIndex = Data->GetArray().GetCount() - 1;
			int i = Index + 1;

			while (i <= lastIndex)
			{
				if (Data->IsValid(i)) break;
				i++;
			}

			Index = i;
		}

	public:
		NS_NODISCARD_INLINE int GetIndex() const noexcept
		{
			return Index;
		}

		NS_NODISCARD_INLINE T& GetValue() const noexcept
		{
			return (*Data)[Index];
		}

		NS_INLINE Iterator& operator++() noexcept
		{
			IterateNextValidIndex();
			return *this;
		}

		NS_INLINE T& operator*() noexcept
		{
			return (*Data)[Index];
		}

		NS_INLINE T* operator->() noexcept
		{
			return &(*Data)[Index];
		}

		NS_INLINE bool operator==(const Iterator& rhs) const noexcept
		{
			return Data == rhs.Data && Index == rhs.Index;
		}

		NS_INLINE bool operator!=(const Iterator& rhs) const noexcept
		{
			return !(*this == rhs);
		}

		NS_INLINE operator bool() const noexcept
		{
			if (Data == nullptr || Index == -1)
			{
				return false;
			}

			const int lastIndex = Data->GetArray().GetCount() - 1;

			if (Index >= lastIndex)
			{
				return Data->IsValid(Index);
			}

			return Index >= 0 && Index <= lastIndex;
		}

	};


	class ConstIterator
	{
		const nsTArrayFreeList* Data;
		int Index;

	public:
		ConstIterator() noexcept
			: Data(nullptr)
			, Index(-1)
		{
		}

		ConstIterator(const nsTArrayFreeList* data) noexcept
			: Data(data)
			, Index(-1)
		{
			IterateNextValidIndex();
		}

	private:
		void IterateNextValidIndex() noexcept
		{
			const int lastIndex = Data->GetArray().GetCount() - 1;
			int i = Index + 1;

			while (i <= lastIndex)
			{
				if (Data->IsValid(i)) break;
				i++;
			}

			Index = i;
		}

	public:
		NS_NODISCARD_INLINE int GetIndex() const noexcept
		{
			return Index;
		}

		NS_NODISCARD_INLINE const T& GetValue() const noexcept
		{
			return (*Data)[Index];
		}

		NS_INLINE ConstIterator& operator++() noexcept
		{
			IterateNextValidIndex();
			return *this;
		}

		NS_INLINE const T& operator*() const noexcept
		{
			return (*Data)[Index];
		}

		NS_INLINE const T* operator->() const noexcept
		{
			return &(*Data)[Index];
		}

		NS_INLINE bool operator==(const ConstIterator& rhs) const noexcept
		{
			return Data == rhs.Data && Index == rhs.Index;
		}

		NS_INLINE bool operator!=(const ConstIterator& rhs) const noexcept
		{
			return !(*this == rhs);
		}

		NS_INLINE operator bool() const noexcept
		{
			const int lastIndex = Data->GetArray().GetCount() - 1;

			if (Data == nullptr || Index == -1 || Index > lastIndex)
			{
				return false;
			}

			if (Index == lastIndex)
			{
				return Data->IsValid(Index);
			}

			return Index >= 0 && Index <= lastIndex;
		}

	};


public:
	NS_NODISCARD_INLINE Iterator CreateIterator() noexcept { return Iterator(this); }
	NS_NODISCARD_INLINE ConstIterator CreateConstIterator() const noexcept { return ConstIterator(this); }

};




#define NS_MAP_CHECK_COLLISION	1

template<typename TUniqueKey, typename TValue>
class nsTMap
{
private:
	nsTArray<uint64> Hashes;
	nsTArray<TUniqueKey> Keys;
	nsTArray<TValue> Values;


public:
	nsTMap() noexcept
	{
		Hashes.Reserve(8);
		Keys.Reserve(8);
		Values.Reserve(8);
	}


	nsTMap(const nsTMap& other) noexcept
		: Hashes(other.Hashes)
		, Keys(other.Keys)
		, Values(other.Values)
	{
	}


	nsTMap(nsTMap&& other) noexcept
		: Hashes(std::move(other.Hashes))
		, Keys(std::move(other.Keys))
		, Values(std::move(other.Values))
	{
	}


	~nsTMap() noexcept = default;


private:
	NS_NODISCARD_INLINE int FindIndex(const uint64 hashValue) const noexcept
	{
		for (int i = 0; i < Hashes.GetCount(); ++i)
		{
			if (Hashes[i] == hashValue)
			{
				return i;
			}
		}

		return NS_ARRAY_INDEX_INVALID;
	}


public:
	NS_INLINE void Reserve(int newCapacity) noexcept
	{
		Hashes.Reserve(newCapacity);
		Keys.Reserve(newCapacity);
		Values.Reserve(newCapacity);
	}


	NS_INLINE TValue& Add(const TUniqueKey& key) noexcept
	{
		const uint64 hash = ns_GetHash(key);
		int index = FindIndex(hash);

		if (index == NS_ARRAY_INDEX_INVALID)
		{
			index = Hashes.GetCount();
			Hashes.Add(hash);
			Keys.Add(key);
			Values.Add();
		}
		else
		{
		#if NS_MAP_CHECK_COLLISION
			const int keyIndex = Keys.Find(key);
			NS_ValidateV(keyIndex != NS_ARRAY_INDEX_INVALID, TEXT("nsTMap collision!"));
		#endif // NS_MAP_CHECK_COLLISION
		}

		return Values[index];
	}


	template<typename...TConstructorArgs>
	NS_INLINE void Add(const TUniqueKey& key, TConstructorArgs&&... args) noexcept
	{
		const uint64 hash = ns_GetHash(key);
		int index = FindIndex(hash);

		if (index == NS_ARRAY_INDEX_INVALID)
		{
			index = Hashes.GetCount();
			Hashes.Add(hash);
			Keys.Add(key);
			Values.Add(std::forward<TConstructorArgs>(args)...);
		}
		else
		{
		#if NS_MAP_CHECK_COLLISION
			const int keyIndex = Keys.Find(key);
			NS_ValidateV(keyIndex != NS_ARRAY_INDEX_INVALID, TEXT("nsTMap collision!"));
		#endif // NS_MAP_CHECK_COLLISION

			Values[index] = TValue(std::forward<TConstructorArgs>(args)...);
		}
	}


	NS_INLINE void Remove(const TUniqueKey& key, bool bKeepOrdered = false) noexcept
	{
		const int index = FindIndex(ns_GetHash(key));

		if (index != NS_ARRAY_INDEX_INVALID)
		{
			Hashes.RemoveAt(index, bKeepOrdered);
			Keys.RemoveAt(index, bKeepOrdered);
			Values.RemoveAt(index, bKeepOrdered);
		}
	}


	NS_NODISCARD_INLINE bool Exists(const TUniqueKey& key) const noexcept
	{
		return FindIndex(ns_GetHash(key)) != NS_ARRAY_INDEX_INVALID;
	}


	NS_NODISCARD_INLINE TUniqueKey& GetKeyByIndex(int index) noexcept
	{
		return Keys[index];
	}


	NS_NODISCARD_INLINE const TUniqueKey& GetKeyByIndex(int index) const noexcept
	{
		return Keys[index];
	}


	NS_NODISCARD_INLINE const nsTArray<TUniqueKey>& GetKeys() const noexcept
	{
		return Keys;
	}


	NS_NODISCARD_INLINE TValue* GetValueByKey(const TUniqueKey& key) noexcept
	{
		const int index = FindIndex(ns_GetHash(key));

		if (index == NS_ARRAY_INDEX_INVALID)
		{
			return nullptr;
		}

		return &Values[index];
	}


	NS_NODISCARD_INLINE const TValue* GetValueByKey(const TUniqueKey& key) const noexcept
	{
		const int index = FindIndex(ns_GetHash(key));

		if (index == NS_ARRAY_INDEX_INVALID)
		{
			return nullptr;
		}

		return &Values[index];
	}


	NS_NODISCARD_INLINE TValue& GetValueByIndex(int index) noexcept
	{
		return Values[index];
	}


	NS_NODISCARD_INLINE const TValue& GetValueByIndex(int index) const noexcept
	{
		return Values[index];
	}


	NS_NODISCARD_INLINE const nsTArray<TValue>& GetValues() const noexcept
	{
		return Values;
	}


	NS_NODISCARD_INLINE void Clear(bool bFree = false) noexcept
	{
		Hashes.Clear(bFree);
		Keys.Clear(bFree);
		Values.Clear(bFree);
	}


	NS_NODISCARD_INLINE int GetCount() const noexcept
	{
		return Hashes.GetCount();
	}


public:
	NS_INLINE nsTMap& operator=(const nsTMap& rhs) noexcept
	{
		if (this != &rhs)
		{
			Hashes = rhs.Hashes;
			Keys = rhs.Keys;
			Values = rhs.Values;
		}

		return *this;
	}


	NS_INLINE nsTMap& operator=(nsTMap&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Hashes = std::move(rhs.Hashes);
			Keys = std::move(rhs.Keys);
			Values = std::move(rhs.Values);
		}

		return *this;
	}


	NS_INLINE TValue& operator[](const TUniqueKey& key) noexcept
	{
		return Add(key);
	}


	NS_INLINE const TValue& operator[](const TUniqueKey& key) const
	{
		const int index = FindIndex(ns_GetHash(key));
		NS_ValidateV(index != NS_ARRAY_INDEX_INVALID, TEXT("nsTMap key not found!"));

		return Values[index];
	}

};
