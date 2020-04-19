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
//Class header
#include "FileSystemNodeIndex.hpp"
//Local
#include "../InjectionContainer.hpp"
#include "../status/StatusTracker.hpp"

//Public methods
BinaryTreeSet<uint32> FileSystemNodeIndex::ComputeDifference(const FileSystemNodeIndex &other) const
{
	BinaryTreeSet<uint32> diff;
	Mutex diffLock;

	InjectionContainer &injectionContainer = InjectionContainer::Instance();
	StaticThreadPool& threadPool = injectionContainer.Get<StaticThreadPool>();
	StatusTracker& statusTracker = injectionContainer.Get<StatusTracker>();
	
	ProcessStatus& process = statusTracker.AddProcessStatusTracker(u8"Computing index differences", this->GetNumberOfNodes(), this->ComputeTotalSize());
	for(uint32 i = 0; i < this->GetNumberOfNodes(); i++)
	{
		//threadPool.EnqueueTask([]() //TODO: reenable this
		//{
		const Path& filePath = this->GetNodePath(i);
		const FileSystemNodeAttributes& fileAttributes = this->GetNodeAttributes(i);

		bool include = false;
		if(other.HasNodeIndex(filePath))
		{
			//both indexes have this node
			uint32 otherIndex = other.GetNodeIndex(filePath);
			const FileSystemNodeAttributes& otherFileAttributes = other.GetNodeAttributes(otherIndex);
			if(fileAttributes != otherFileAttributes)
				include = true;
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
		//});
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

uint64 FileSystemNodeIndex::ComputeTotalSize(const BinaryTreeSet<uint32> &nodeIndices) const
{
	uint64 totalSize = 0;
	for(uint32 nodeIndex : nodeIndices)
		totalSize += this->GetNodeAttributes(nodeIndex).Size();

	return totalSize;
}

uint64 FileSystemNodeIndex::ComputeTotalSize(const Map<uint32, NodeDifference>& nodeIndices) const
{
	uint64 totalSize = 0;
	for(const auto& kv : nodeIndices)
		totalSize += this->GetNodeAttributes(kv.key).Size();

	return totalSize;
}