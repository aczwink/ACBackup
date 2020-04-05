/*
 * Copyright (c) 2020 Amir Czwink (amir130@hotmail.de)
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
#include "TerminalStatusTracker.hpp"

//Constructor
TerminalStatusTracker::TerminalStatusTracker() : reporterThread(Function<int32()>(&TerminalStatusTracker::ThreadMain, this))
{
	this->running = true;
	this->reporterThread.Start();
}

//Destructor
TerminalStatusTracker::~TerminalStatusTracker()
{
	this->running = false;
	this->reporterThread.Join();
}

//Private methods
void TerminalStatusTracker::PrintKnownEndTask(const ProcessStatus &processStatus)
{
	stdOut << u8"Finished files: " << processStatus.NumberOfFinishedFiles() << u8" of " << processStatus.NumberOfFiles() << endl
		<< u8"Finished size: " << String::FormatBinaryPrefixed(processStatus.DoneSize()) << u8" of " << String::FormatBinaryPrefixed(processStatus.TotalSize()) << endl
		<< u8"Speed: " << String::FormatBinaryPrefixed(processStatus.DoneSize() / (processStatus.GetDurationInMicroseconds() / 1000.0 / 1000.0)) << u8"/s" << endl
		<< u8"Progress: " << uint64(processStatus.DoneSize() * 100.0 / processStatus.TotalSize()) << u8"%" << endl;
}

void TerminalStatusTracker::PrintUnknownEndTask(const ProcessStatus &processStatus)
{
	stdOut << u8"Found files: " << processStatus.NumberOfFiles() << endl;
}

int32 TerminalStatusTracker::ThreadMain()
{
	uint16 currentStep = 0;
	while(this->running)
	{
		this->AcquireProcesses();
		const auto& processes = this->Processes();
		if(currentStep < processes.GetNumberOfElements())
		{
			const ProcessStatus& processStatus = *processes[currentStep];

			Optional<DateTime> expectedEndTime = processStatus.ComputeExpectedEndTime();
			stdOut << processStatus.Title() << endl
				<< u8"Process started on: " << processStatus.StartTime().ToISOString() << endl
				<< (processStatus.EndTime().HasValue() ? u8"Process ended on" : u8"Expected end time") << u8": " << (processStatus.EndTime().HasValue() ? processStatus.EndTime()->ToISOString() : processStatus.ComputeExpectedEndTimeAsString()) << endl
				<< u8"Total duration: " << processStatus.GetDurationInMicroseconds() / 1000 / 1000 << u8" s" << endl;
			if(processStatus.IsEndDeterminate())
				this->PrintKnownEndTask(processStatus);
			else
				this->PrintUnknownEndTask(processStatus);

			stdOut << endl << endl;

			if((currentStep+1) < processes.GetNumberOfElements())
				currentStep++;
		}
		this->ReleaseProcesses();

		Sleep(1 * 1000 * 1000 * 1000);
	}
	return EXIT_SUCCESS;
}