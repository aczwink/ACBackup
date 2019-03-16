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
//Class header
#include "ProcessStatus.hpp"
//Namespaces
using namespace StdXX::CommonFileFormats;

//Private methods
JSONValue ProcessStatus::ToJSON() const
{
	JSONValue obj = JSONValue::Object();

	AutoLock lock(this->mutex);

	uint64 duration_microsecs;
	if(this->endTime.HasValue())
		duration_microsecs = this->totalTaskDuration;
	else
		duration_microsecs = this->clock.GetElapsedMicroseconds();

	obj[u8"title"] = this->title;
	obj[u8"nFinishedFiles"] = this->nFinishedFiles;
	obj[u8"nFiles"] = this->nFiles;
	obj[u8"totalSize"] = this->totalSize;
	obj[u8"doneSize"] = this->doneSize;
	obj[u8"startTime"] = this->startTime.ToISOString();
	obj[u8"duration_millisecs"] = duration_microsecs / 1000;
	if(this->isEndDeterminate)
		obj[u8"progress"] = this->doneSize / float64(this->totalSize);
	else
		obj[u8"progress"] = this->endTime.HasValue();

	if(this->endTime.HasValue())
		obj[u8"endTime"] = this->endTime->ToISOString();
	else
	{
		if(this->isEndDeterminate && (duration_microsecs > 0))
		{
			uint64 leftSize = this->totalSize - this->doneSize;
			float64 speed = this->doneSize / (duration_microsecs / 1000.0);

			uint64 passed = duration_microsecs / 1000;
			uint64 todo = static_cast<uint64>(leftSize / speed);
			obj[u8"expectedEndTime"] = this->startTime.AddMilliSeconds(passed + todo).ToISOString();
		}
		else
			obj[u8"expectedEndTime"] = u8"?";
	}

	return obj;
}