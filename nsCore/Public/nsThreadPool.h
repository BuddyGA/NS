#pragma once

#include "nsString.h"



class NS_CORE_API nsIThreadTask
{
	NS_DECLARE_NOCOPY(nsIThreadTask)

public:
	nsIThreadTask() noexcept {}
	virtual ~nsIThreadTask() noexcept {}
	virtual void Reset() noexcept = 0;
	virtual void Execute() noexcept = 0;
	virtual bool IsIdle() const noexcept = 0;
	virtual bool IsRunning() const noexcept = 0;
	virtual bool IsDone() const noexcept = 0;

#ifdef _DEBUG
	virtual nsString GetDebugName() const noexcept { return ""; }
#endif // _DEBUG

};



template<typename TThreadTask, int N>
class nsThreadTaskPool
{
private:
	nsTArrayInline<TThreadTask, N> Tasks;

public:
	nsThreadTaskPool() noexcept = default;


	NS_NODISCARD_INLINE TThreadTask* CreateTask() noexcept
	{
		TThreadTask* task = nullptr;

		for (int i = 0; i < Tasks.GetCount(); ++i)
		{
			if (Tasks[i]->IsIdle())
			{
				task = &Tasks[i];
				break;
			}
		}

		if (task == nullptr)
		{
			task = &Tasks.Add();
		}

		task->Reset();

		return task;
	}


	NS_NODISCARD_INLINE const nsTArrayInline<TThreadTask, N>& GetTasks() const noexcept
	{
		return Tasks;
	}

};


typedef uint32 nsThreadId;


namespace nsEThreadAffinity
{
	enum Mask
	{
		None				= (0),

		Thread_Main			= (1 << 0),
		Thread_Worker_00	= (1 << 1),
		Thread_Worker_01	= (1 << 2),
		Thread_Worker_02	= (1 << 3),
		Thread_Worker_03	= (1 << 4),
		Thread_Worker_04	= (1 << 5),
		Thread_Worker_05	= (1 << 6),
		Thread_Worker_06	= (1 << 7),
		Thread_Worker_07	= (1 << 8),
		Thread_Worker_08	= (1 << 9),
		Thread_Worker_09	= (1 << 10),
		Thread_Worker_10	= (1 << 11),
		Thread_Worker_11	= (1 << 12),
		Thread_Worker_12	= (1 << 13),
		Thread_Worker_13	= (1 << 14),
		Thread_Worker_14	= (1 << 15),
		Thread_Worker_15	= (1 << 16),
		Thread_Worker_16	= (1 << 16),
		Thread_Worker_17	= (1 << 17),
		Thread_Worker_18	= (1 << 18),
		Thread_Worker_19	= (1 << 19),
		Thread_Worker_20	= (1 << 20),
		Thread_Worker_21	= (1 << 21),
		Thread_Worker_22	= (1 << 22),
		Thread_Worker_23	= (1 << 23),
		Thread_Worker_24	= (1 << 24),
		Thread_Worker_25	= (1 << 25),
		Thread_Worker_26	= (1 << 26),
		Thread_Worker_27	= (1 << 27),
		Thread_Worker_28	= (1 << 28),
		Thread_Worker_29	= (1 << 29),
		Thread_Worker_30	= (1 << 30),
		Thread_Worker_31	= (1 << 31),

		Thread_ExcludeMain	= (UINT32_MAX - 1),
		Thread_ALL			= (UINT32_MAX)
	};
};

typedef uint32 nsThreadAffinityMasks;



namespace nsThreadPool
{
	// Initialize threadpool with num worker threads equals to (num cores - 1)
	extern NS_CORE_API void Initialize() noexcept;

	// Shutdown threadpool. Will wait for all worker threads
	extern NS_CORE_API void Shutdown() noexcept;

	// [Main thread only] Submit tasks to specific thread mask.
	// Bit [0] = main thread
	// Bit [1..31] = worker threads
	extern NS_CORE_API void SubmitTasks(nsIThreadTask** tasks, int taskCount, nsThreadAffinityMasks threadAffinityMasks) noexcept;

	// Get worker threads (includes main thread at index 0)
	NS_NODISCARD nsTArrayInline<nsThreadId, 32> GetWorkerThreads() noexcept;

	// Check if current thread is main thread
	NS_NODISCARD extern NS_CORE_API bool IsMainThread() noexcept;

};



#define NS_Validate_IsMainThread() NS_Validate(nsThreadPool::IsMainThread())
