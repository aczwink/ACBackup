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