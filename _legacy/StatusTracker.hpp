//Local
#include "ProcessStatus.hpp"
#include "StatusTrackerWebService.hpp"

class StatusTracker
{
public:
	//Inline
	inline void AcquireProcesses()
	{
		this->processesLock.Lock();
	}

	inline ProcessStatus& AddProcessStatusTracker(const String& title, uint32 nFiles, uint64 totalSize)
	{
		AutoLock lock(this->processesLock);

		this->processes.InsertTail(new ProcessStatus(title, nFiles, totalSize));
		return *this->processes.Last();
	}

	inline auto GetProcessesBegin()
	{
		return this->processes.begin();
	}

	inline auto GetProcessesEnd()
	{
		return this->processes.end();
	}

	inline void ReleaseProcesses()
	{
		this->processesLock.Unlock();
	}
};