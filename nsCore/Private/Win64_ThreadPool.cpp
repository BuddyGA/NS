#include "nsThreadPool.h"
#include "nsLogger.h"



struct nsThreadWorker
{
	HANDLE Handle;
	DWORD Id;
	uint32 Affinity;
	nsName Name;
	nsTArray<nsIThreadTask*> Tasks;
	nsCriticalSection CS;
	HANDLE WakeSemaphore;
};


static nsLogCategory ThreadPoolLog("nsThreadPoolLog", nsELogVerbosity::LV_DEBUG);
static DWORD MainThreadId;
static nsThreadWorker ThreadWorkers[32];
static int NumThreads;
static nsAtomic bShuttingDown;
static bool bInitialized;


static DWORD WINAPI ns_ThreadProc(_In_ LPVOID lpParameter) noexcept
{
	nsThreadWorker* worker = reinterpret_cast<nsThreadWorker*>(lpParameter);
	nsTArray<nsIThreadTask*> cachedTasks;

	while (bShuttingDown.Get() == 0)
	{
		WaitForSingleObject(worker->WakeSemaphore, INFINITE);

		if (bShuttingDown.Get() == 1)
		{
			break;
		}

		worker->CS.Enter();
		{
			cachedTasks = worker->Tasks;
			worker->Tasks.Clear();
		}
		worker->CS.Leave();

		for (int i = 0; i < cachedTasks.GetCount(); ++i)
		{
			nsIThreadTask* task = cachedTasks[i];

		#ifdef _DEBUG
			NS_LogDebug(ThreadPoolLog, "[Thread-%u] execute task [%s]", worker->Id, *task->GetDebugName());
		#endif // _DEBUG

			task->Execute();
		}

		cachedTasks.Clear();
		NS_LogDebug(ThreadPoolLog, "[Thread-%u] All tasks finished", worker->Id);
	}

	return 0;
}


static void ns_ExecuteMainThreadTasks() noexcept
{
	nsThreadWorker& main = ThreadWorkers[0];
	nsTArray<nsIThreadTask*>& mainThreadTasks = main.Tasks;

	for (int i = 0; i < mainThreadTasks.GetCount(); ++i)
	{
		nsIThreadTask* task = mainThreadTasks[i];

	#ifdef _DEBUG
		NS_LogDebug(ThreadPoolLog, "[Thread-main] execute task [%s]", *task->GetDebugName());
	#endif // _DEBUG

		task->Execute();
	}

	NS_LogDebug(ThreadPoolLog, "[Thread-main] All tasks finished");
}


static nsTArrayInline<nsThreadWorker*, 32> ns_GetThreadWorkers(nsThreadAffinityMasks masks) noexcept
{
	nsTArrayInline<nsThreadWorker*, 32> threadWorkers;

	for (int i = 0; i < NumThreads; ++i)
	{
		if (masks & ThreadWorkers[i].Affinity)
		{
			threadWorkers.Add(&ThreadWorkers[i]);
		}
	}

	return threadWorkers;
}



void nsThreadPool::Initialize() noexcept
{
	if (bInitialized)
	{
		return;
	}

	MainThreadId = GetCurrentThreadId();

	SYSTEM_INFO sysInfo{};
	GetSystemInfo(&sysInfo);

	NumThreads = static_cast<int>(sysInfo.dwNumberOfProcessors);

	nsThreadWorker& mainThread = ThreadWorkers[0];
	mainThread.Handle = NULL;
	mainThread.Id = MainThreadId;
	mainThread.Affinity = 1;

	for (int i = 1; i < NumThreads; ++i)
	{
		nsThreadWorker& worker = ThreadWorkers[i];
		worker.Handle = CreateThread(NULL, 0, ns_ThreadProc, &ThreadWorkers[i], CREATE_SUSPENDED, &worker.Id);
		worker.WakeSemaphore = CreateSemaphore(NULL, 0, INT32_MAX, NULL);
		worker.Affinity = (1 << i);
	}

	bShuttingDown.Set(0);

	for (int i = 1; i < NumThreads; ++i)
	{
		ResumeThread(ThreadWorkers[i].Handle);
	}

	bInitialized = true;

	NS_LogInfo(ThreadPoolLog, "Initialize thread pool [NumWorkerThreads: %i]", NumThreads - 1);
}


void nsThreadPool::Shutdown() noexcept
{
	if (bInitialized)
	{
		NS_LogInfo(ThreadPoolLog, "Shutdown threadpool");

		bShuttingDown.Set(1);
		nsTArrayInline<HANDLE, 32> workerThreadHandles;

		for (int i = 1; i < NumThreads; ++i)
		{
			workerThreadHandles.Add(ThreadWorkers[i].Handle);
			ReleaseSemaphore(ThreadWorkers[i].WakeSemaphore, 1, NULL);
		}

		WaitForMultipleObjects(static_cast<DWORD>(workerThreadHandles.GetCount()), workerThreadHandles.GetData(), TRUE, INFINITE);

		for (int i = 1; i < NumThreads; ++i)
		{
			CloseHandle(ThreadWorkers[i].Handle);
		}

		bInitialized = false;
	}
}


void nsThreadPool::SubmitTasks(nsIThreadTask** tasks, int taskCount, nsThreadAffinityMasks threadAffinityMasks) noexcept
{
	NS_Assert(bInitialized);
	NS_Validate_IsMainThread();

	nsTArrayInline<nsThreadWorker*, 32> workers = ns_GetThreadWorkers(threadAffinityMasks);
	const int workerCount = workers.GetCount();
	NS_Assert(workerCount > 0);

	// Number of work tasks per worker
	const int workCountPerThread = taskCount / workerCount;

	// Used to distribute remaining task for all workers or if (taskCount < workerCount)
	int remainingWorkCount = taskCount % workerCount;
	int submitCount = 0;

	for (int i = 0; i < workerCount; ++i)
	{
		nsThreadWorker* worker = workers[i];

		const bool bNotMainThread = i > 0;
		bool bSubmit = false;

		if (bNotMainThread)
		{
			worker->CS.Enter();
		}

		if (workCountPerThread > 0)
		{
			worker->Tasks.InsertAt(&tasks[submitCount], workCountPerThread);
			submitCount += workCountPerThread;
			bSubmit = true;
		}

		if (remainingWorkCount > 0)
		{
			worker->Tasks.Add(tasks[submitCount++]);
			remainingWorkCount--;
			bSubmit = true;
		}

		if (bNotMainThread)
		{
			worker->CS.Leave();

			if (bSubmit)
			{
				ReleaseSemaphore(worker->WakeSemaphore, 1, NULL);
			}
		}
	}

	NS_Assert(remainingWorkCount == 0);
	NS_Assert(submitCount == taskCount);

	if (threadAffinityMasks & nsEThreadAffinity::Thread_Main)
	{
		ns_ExecuteMainThreadTasks();
	}
}


nsTArrayInline<nsThreadId, 32> nsThreadPool::GetWorkerThreads() noexcept
{
	NS_Assert(bInitialized);

	nsTArrayInline<nsThreadId, 32> threadWorkers;

	for (int i = 0; i < NumThreads; ++i)
	{
		threadWorkers.Add(ThreadWorkers[i].Id);
	}

	return threadWorkers;
}


bool nsThreadPool::IsMainThread() noexcept
{
	NS_Assert(bInitialized);
	return GetCurrentThreadId() == MainThreadId;
}
