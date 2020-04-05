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

class ProcessStatus
{
public:
	//Constructors
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

	//Methods
	Optional<DateTime> ComputeExpectedEndTime() const;

	//Properties
	inline uint64 DoneSize() const
	{
		return this->doneSize;
	}

	inline const Optional<DateTime> EndTime() const
	{
		return this->endTime;
	}

	inline bool IsEndDeterminate() const
	{
		return this->isEndDeterminate;
	}

	inline uint32 NumberOfFiles() const
	{
		return this->nFiles;
	}

	inline uint32 NumberOfFinishedFiles() const
	{
		return this->nFinishedFiles;
	}

	inline const DateTime& StartTime() const
	{
		return this->startTime;
	}

	inline const String& Title() const
	{
		return this->title;
	}

	inline uint64 TotalSize() const
	{
		return this->totalSize;
	}

	//Inline
	inline void AddFinishedSize(uint64 size)
	{
		AutoLock lock(this->mutex);
		this->doneSize += size;
	}

	inline void AddTotalSize(uint64 size)
	{
		this->totalSize += size;
	}

	inline String ComputeExpectedEndTimeAsString() const
	{
		if(this->isEndDeterminate && (this->GetDurationInMicroseconds() > 0))
			return this->ComputeExpectedEndTime()->ToISOString();
		return u8"???";
	}

	inline void Finished()
	{
		AutoLock lock(this->mutex);
		this->endTime = DateTime::Now();
		this->totalTaskDuration = this->clock.GetElapsedMicroseconds();
	}

	inline uint64 GetDurationInMicroseconds() const
	{
		if(this->endTime.HasValue())
			return this->totalTaskDuration;

		return this->clock.GetElapsedMicroseconds();
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