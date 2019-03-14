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
		this->duration_microsecs = 0;
		this->totalSize = 0;
	}

	inline ProcessStatus(const String& title, uint32 nFiles, uint64 totalSize)
			: title(title), nFiles(nFiles), totalSize(totalSize), startTime(DateTime::Now())
	{
		this->isEndDeterminate = true;
		this->nFinishedFiles = 0;
		this->doneSize = 0;
		this->duration_microsecs = 0;
	}

	//Methods
	CommonFileFormats::JSONValue ToJSON() const;

	//Inline
	inline void AddFinishedSize(uint64 size, uint64 us)
	{
		AutoLock lock(this->mutex);
		this->doneSize += size;
		this->duration_microsecs += us;
	}

	inline void Finished()
	{
		AutoLock lock(this->mutex);
		this->endTime = DateTime::Now();
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

	inline void ReduceTotalSize(uint64 size)
	{
		AutoLock lock(this->mutex);
		this->totalSize -= size;
	}

private:
	//Members
	bool isEndDeterminate;
	mutable Mutex mutex;
	String title;
	uint32 nFinishedFiles;
	uint32 nFiles;
	uint64 totalSize;
	uint64 doneSize;
	DateTime startTime;
	Optional<DateTime> endTime;
	uint64 duration_microsecs;
};