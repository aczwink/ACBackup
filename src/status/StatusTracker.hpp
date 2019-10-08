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
#include "StatusTrackerWebService.hpp"
#include "ProcessStatus.hpp"

class StatusTracker
{
public:
	//Constructor
	StatusTracker(uint16 port);

	//Destructor
	~StatusTracker();

	//Inline
	inline ProcessStatus& AddProcessStatusTracker(const String& title)
	{
		AutoLock lock(this->processesLock);

		this->processes.InsertTail(new ProcessStatus(title));
		return *this->processes.Last();
	}

	inline uint16 GetPort() const
	{
		return this->httpServer.GetBoundPort();
	}

	inline HTTPServer& GetServer()
	{
		return this->httpServer;
	}

private:
	//Members
	StatusTrackerWebService httpServer;
	Thread thread;
	LinkedList<UniquePointer<ProcessStatus>> processes;
	Mutex processesLock;
};