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
#include <Std++.hpp>
using namespace StdXX;

class ProcessStatus
{
public:
	//Constructor
	inline ProcessStatus(const String& title)
			: title(title), startTime(DateTime::Now())
	{
		this->isEndDeterminate = false;
		this->nFinishedFiles = 0;
		this->nFiles = 0;
		this->doneSize = 0;
		this->totalSize = 0;

		this->clock.Start();
	}

	inline ProcessStatus(const String& title, uint32 nFiles, uint64 totalSize)
			: title(title), nFiles(nFiles), totalSize(totalSize), startTime(DateTime::Now())
	{
		this->isEndDeterminate = true;
		this->nFinishedFiles = 0;
		this->doneSize = 0;

		this->clock.Start();
	}

	//Inline
	inline void Finished()
	{
		AutoLock lock(this->mutex);
		this->endTime = DateTime::Now();
		this->totalTaskDuration = this->clock.GetElapsedMicroseconds();
	}

	inline void IncFileCount()
	{
		AutoLock lock(this->mutex);
		this->nFiles++;
	}

	inline void IncFinishedCount()
	{
		AutoLock lock(this->mutex);
		this->nFinishedFiles++;
	}

private:
	//Members
	String title;
	DateTime startTime;
	bool isEndDeterminate;
	uint32 nFinishedFiles;
	uint32 nFiles;
	uint64 totalSize;
	uint64 doneSize;
	Optional<DateTime> endTime;
	uint64 totalTaskDuration;
	Clock clock;
	mutable Mutex mutex;
};