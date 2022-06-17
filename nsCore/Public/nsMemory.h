#pragma once

#include "nsString.h"



// nsMemory
// Default memory allocator using first-fit algorithm
class NS_CORE_API nsMemory
{
	NS_DECLARE_NOCOPY_NOMOVE(nsMemory)

protected:
	nsName Name;
	int TotalSize;
	int AllocatedSize;
	int AlignmentSize;


	struct Block
	{
	#ifdef _DEBUG
		nsName Name;
	#endif // _DEBUG

		int IsFree;
		int Size;
		Block* Next;
	};

	Block* Blocks;


public:
	nsMemory() noexcept;
	nsMemory(nsName name, int totalSize, int alignmentSize = 16) noexcept;
	virtual ~nsMemory() noexcept;
	virtual void Initialize(nsName name, int totalSize, int alignmentSize = 16) noexcept;


private:
	NS_NODISCARD Block* FindFreeBlock(int requestedSize) const noexcept;
	NS_NODISCARD bool IsValidPtr(void* data) const noexcept;


public:
	NS_NODISCARD virtual void* Allocate(int size, nsName debugName = "") noexcept;
	virtual void Deallocate(void* data) noexcept;
	virtual void Defragment() noexcept;
	virtual void Clear(bool bFreeMemory) noexcept;


	template<typename T, typename...TConstructorArgs>
	NS_INLINE T* AllocateConstruct(TConstructorArgs&&... args) noexcept
	{
		T* obj = static_cast<T*>(Allocate(sizeof(T)));
		new (obj)T(std::forward<TConstructorArgs>(args)...);

		return obj;
	}


	template<typename T, typename...TConstructorArgs>
	NS_INLINE T* AllocateConstructDebug(nsName debugName, TConstructorArgs&&... args) noexcept
	{
		T* obj = static_cast<T*>(Allocate(sizeof(T), debugName));
		new (obj)T(std::forward<TConstructorArgs>(args)...);

		return obj;
	}


	template<typename T>
	NS_INLINE void DeallocateDestruct(T*& obj) noexcept
	{
		if (obj)
		{
			obj->~T();
			Deallocate(obj);
			obj = nullptr;
		}
	}


	NS_INLINE int GetTotalSize() const noexcept
	{
		return TotalSize;
	}


	NS_INLINE int GetAllocatedSize() const noexcept
	{
		return AllocatedSize;
	}


	NS_INLINE int GetAlignmentSize() const noexcept
	{
		return AlignmentSize;
	}

};
