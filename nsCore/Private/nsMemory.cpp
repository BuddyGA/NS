#include "nsMemory.h"
#include "nsPlatform.h"



nsMemory::nsMemory() noexcept
	: Name("")
	, TotalSize(0)
	, AllocatedSize(0)
	, DefaultAlignmentSize(0)
	, Blocks(nullptr)
{
}


nsMemory::nsMemory(nsName name, int totalSize, int defaultAlignment) noexcept
	: nsMemory()
{
	Initialize(name, totalSize, defaultAlignment);
}


nsMemory::~nsMemory() noexcept
{
	Clear(true);
}


void nsMemory::Initialize(nsName name, int totalSize, int defaultAlignment) noexcept
{
	NS_Assert(totalSize >= 0);
	NS_Assert(defaultAlignment >= 4);
	NS_AssertV(Blocks == nullptr, TEXT("Memory allocator already initialized!"));

	Name = name;

	const int blockAlignment = sizeof(Block);
	TotalSize = totalSize + (blockAlignment - (totalSize % blockAlignment)) % blockAlignment;
	AllocatedSize = 0;
	DefaultAlignmentSize = defaultAlignment;

	Blocks = static_cast<Block*>(nsPlatform::Memory_Alloc(TotalSize));
	Blocks->IsFree = 1;
	Blocks->Size = TotalSize;
	Blocks->Next = nullptr;
}


nsMemory::Block* nsMemory::FindFreeBlock(int requestedSize) const noexcept
{
	Block* block = Blocks;

	while (block)
	{
		if (block->Size >= requestedSize && block->IsFree)
		{
			break;
		}

		block = block->Next;
	}

	return block;
}


void* nsMemory::AllocateFromBlock(int size, nsName debugName) noexcept
{
	const int newBlockSize = sizeof(Block) + size;
	NS_ValidateV(AllocatedSize + newBlockSize <= TotalSize, TEXT("Memory allocation failed. Not enough memory! [Memory:%s] [RequestedBlockSize:%u, AllocatedSize: %u, TotalSize: %u]"), *Name.ToString(), newBlockSize, AllocatedSize, TotalSize);

	Block* block = FindFreeBlock(newBlockSize);

	if (block == nullptr)
	{
		Defragment();
		block = FindFreeBlock(newBlockSize);
	}

	NS_ValidateV(block, TEXT("Not enough memory. No fit block found! [%s]"), *Name.ToString());

	const uint32 remainingSize = block->Size - newBlockSize;

	if (remainingSize >= sizeof(Block))
	{
		Block* nextFreeBlock = (Block*)((uint8*)(block + 1) + size);
		nextFreeBlock->Size = remainingSize;
		nextFreeBlock->IsFree = 1;
		nextFreeBlock->Next = (block->Next) ? block->Next : nullptr;

		block->Next = nextFreeBlock;
		block->Size = newBlockSize;
	}
	else
	{
		block->Size = newBlockSize + remainingSize;
	}

	block->IsFree = 0;

#ifdef _DEBUG
	block->Name = debugName;
#endif // _DEBUG

	AllocatedSize += newBlockSize;

	return static_cast<void*>(block + 1);
}


bool nsMemory::IsValidPtr(void* data) const noexcept
{
	NS_AssertV(Blocks, TEXT("Must call Initialize()"));

	const uint8* memoryPtr = (uint8*)Blocks;
	const uint8* dataPtr = (uint8*)data;

	return dataPtr >= memoryPtr && dataPtr < (memoryPtr + TotalSize);
}


void* nsMemory::Allocate(int size, nsName debugName) noexcept
{
	NS_AssertV(Blocks, TEXT("Must call Initialize()!"));
	NS_AssertV(size > 0, TEXT("size must be greater than 0!"));

	const int alignedSize = size + (DefaultAlignmentSize - (size % DefaultAlignmentSize)) % DefaultAlignmentSize;

	return AllocateFromBlock(alignedSize, debugName);
}


void* nsMemory::AllocateAligned(int size, int alignment, nsName debugName) noexcept
{
	NS_AssertV(Blocks, TEXT("Must call Initialize()!"));
	NS_AssertV(size > 0, TEXT("size must be greater than 0!"));

	const int alignedSize = size + (alignment - (size % alignment)) % alignment;

	return AllocateFromBlock(alignedSize, debugName);
}


void nsMemory::Deallocate(void* data) noexcept
{
	NS_Assert(data);
	NS_ValidateV(IsValidPtr(data), TEXT("Invalid pointer memory!"));

	Block* block = (Block*)data - 1;
	NS_ValidateV(block->IsFree == 0, TEXT("Cannot deallocate an already freed block!"));

	const uint32 dataSize = block->Size - sizeof(Block);
	nsPlatform::Memory_Set(data, -11, dataSize);

	block->IsFree = 1;
	AllocatedSize -= block->Size;
}


void nsMemory::Defragment() noexcept
{
	NS_AssertV(Blocks, TEXT("Must call Initialize()"));

	Block* block = Blocks;

	while (block)
	{
		if (block->IsFree)
		{
			const Block* nextBlock = block->Next;

			if (nextBlock && nextBlock->IsFree)
			{
				block->Size += nextBlock->Size;
				block->Next = nextBlock->Next;
			}
			else
			{
				block = block->Next;
			}
		}
		else
		{
			block = block->Next;
		}
	}
}


void nsMemory::Clear(bool bFreeMemory) noexcept
{
	if (Blocks)
	{
		nsPlatform::Memory_Set(Blocks, -11, TotalSize);
		Blocks->Size = TotalSize;
		Blocks->IsFree = 1;
		Blocks->Next = nullptr;

	#ifdef _DEBUG
		Blocks->Name = "";
	#endif // _DEBUG

		if (bFreeMemory)
		{
			nsPlatform::Memory_Free(Blocks);
			TotalSize = 0;
			AllocatedSize = 0;
			DefaultAlignmentSize = 0;
			Blocks = nullptr;
		}
	}
}
