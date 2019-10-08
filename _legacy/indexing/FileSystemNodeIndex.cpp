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
#include "FileSystemNodeIndex.hpp"
//Local
#include "../ProcessStatus.hpp"

//Public methods
BinaryTreeSet<uint32> FileSystemNodeIndex::ComputeDifference(const FileSystemNodeIndex &other, StaticThreadPool& threadPool, StatusTracker& tracker) const
{
	BinaryTreeSet<uint32> diff;
	Mutex diffLock;
	ProcessStatus& process = tracker.AddProcessStatusTracker(u8"Computing snapshot differences", this->GetNumberOfNodes(), this->ComputeTotalSize());

	for(uint32 i = 0; i < this->GetNumberOfNodes(); i++)
	{
		NOT_IMPLEMENTED_ERROR; //TODO: NOT TESTED
		threadPool.EnqueueTask([this, &other, &process, &diff, &diffLock, i]()
		{
			NOT_IMPLEMENTED_ERROR; //TODO: NOT TESTED
			const Path& filePath = this->GetNodePath(i);
			const FileSystemNodeAttributes& fileAttributes = this->GetNodeAttributes(i);

			uint32 prevIndex = other.FindNodeIndex(filePath);
			bool include = false;
			if(prevIndex != Unsigned<uint32>::Max())
			{
				//both indexes have this node
				const FileSystemNodeAttributes& prevFileAttributes = other.GetNodeAttributes(prevIndex);

				bool didChange = fileAttributes != prevFileAttributes;
				if(didChange)
					include = true;
				//else
					//snapshot->AddPreviousFile(filePath, index); //don't back it up again, but make a hint that a previous snapshot has it
			}
			else
			{
				//only we have this node
				include = true;
			}

			if(include)
			{
				diffLock.Lock();
				diff.Insert(i);
				diffLock.Unlock();
			}

			process.AddFinishedSize(fileAttributes.Size());
			process.IncFinishedCount();
		});
	}
	threadPool.WaitForAllTasksToComplete();
	process.Finished();

	return diff;
}

uint64 FileSystemNodeIndex::ComputeTotalSize() const
{
	uint64 totalSize = 0;
	for(uint32 i = 0; i < this->GetNumberOfNodes(); i++)
		totalSize += this->GetNodeAttributes(i).Size();
	return totalSize;
}
