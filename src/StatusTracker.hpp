/*
 * Copyright (c) 2019 Amir Czwink (amir130@hotmail.de)
 *
 * This file is part of ACBackup.
 *
 * ACBackup is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ACBackup is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ACBackup.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
//Local
#include "ProcessStatus.hpp"
#include "StatusTrackerWebService.hpp"

class StatusTracker
{
public:
	//Constructor
	StatusTracker(uint16 port = 8080);

	//Destructor
	~StatusTracker();

	//Inline
	inline void AcquireProcesses()
	{
		this->processesLock.Lock();
	}

	inline ProcessStatus& AddProcessStatusTracker(const String& title)
	{
		AutoLock lock(this->processesLock);

		this->processes.InsertTail(new ProcessStatus(title));
		return *this->processes.Last();
	}

	inline ProcessStatus& AddProcessStatusTracker(const String& title, uint32 nFiles, uint64 totalSize)
	{
		AutoLock lock(this->processesLock);

		this->processes.InsertTail(new ProcessStatus(title, nFiles, totalSize));
		return *this->processes.Last();
	}

	inline uint16 GetPort() const
	{
		return this->httpServer.GetBoundPort();
	}

	inline auto GetProcessesBegin()
	{
		return this->processes.begin();
	}

	inline auto GetProcessesEnd()
	{
		return this->processes.end();
	}

	inline HTTPServer& GetServer()
	{
		return this->httpServer;
	}

	inline void ReleaseProcesses()
	{
		this->processesLock.Unlock();
	}

private:
	//Members
	StatusTrackerWebService httpServer;
	Thread thread;
	LinkedList<UniquePointer<ProcessStatus>> processes;
	Mutex processesLock;
};