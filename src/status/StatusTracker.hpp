/*
 * Copyright (c) 2019-2020 Amir Czwink (amir130@hotmail.de)
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
#include <Std++.hpp>
using namespace StdXX;
//Local
#include "ProcessStatus.hpp"

enum class StatusTrackerType
{
	Terminal,
	Web
};

class StatusTracker
{
public:
	virtual ~StatusTracker() = default;

	//Inline
	inline ProcessStatus& AddProcessStatusTracker(const String& title)
	{
		AutoLock lock(this->processesLock);

		this->processes.Push(new ProcessStatus(title));
		return *this->processes.Last();
	}

	inline ProcessStatus& AddProcessStatusTracker(const String& title, uint32 nFiles, uint64 totalSize)
	{
		AutoLock lock(this->processesLock);

		this->processes.Push(new ProcessStatus(title, nFiles, totalSize));
		return *this->processes.Last();
	}

	//Functions
	static StatusTracker* CreateInstance(StatusTrackerType type);

protected:
	//Properties
	inline auto& Processes() const
	{
		return this->processes;
	}

	//Inline
	inline void AcquireProcesses()
	{
		this->processesLock.Lock();
	}

	inline auto GetProcessesBegin() const
	{
		return this->processes.begin();
	}

	inline auto GetProcessesEnd() const
	{
		return this->processes.end();
	}

	inline void ReleaseProcesses()
	{
		this->processesLock.Unlock();
	}

private:
	//Members
	DynamicArray<UniquePointer<ProcessStatus>> processes;
	Mutex processesLock;
};