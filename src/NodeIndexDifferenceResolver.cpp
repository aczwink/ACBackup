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
#include "NodeIndexDifferenceResolver.hpp"
//Local
#include "InjectionContainer.hpp"
#include "status/StatusTracker.hpp"
#include "config/ConfigManager.hpp"

//Public methods
NodeIndexDifferences NodeIndexDifferenceResolver::ComputeDiff(const BackupNodeIndex &leftIndex, const FileSystemNodeIndex &rightIndex)
{
	BinaryTreeSet<uint32> leftToRightDiffs = this->ComputeDifference(leftIndex, rightIndex);
	BinaryTreeSet<uint32> rightToLeftDiffs = this->ComputeDifference(rightIndex, leftIndex);
	return this->ResolveDifferences(leftIndex, rightIndex, leftToRightDiffs, rightToLeftDiffs);
}

//Private methods
BinaryTreeSet<uint32> NodeIndexDifferenceResolver::ComputeDeleted(const FileSystemNodeIndex& leftIndex, const FileSystemNodeIndex& rightIndex, const BinaryTreeSet<uint32>& indexes) const
{
	InjectionContainer &injectionContainer = InjectionContainer::Instance();
	StaticThreadPool& threadPool = injectionContainer.Get<StaticThreadPool>();
	StatusTracker& statusTracker = injectionContainer.Get<StatusTracker>();

	BinaryTreeSet<uint32> nodeDifferences;

	ProcessStatus& process = statusTracker.AddProcessStatusTracker(u8"Resolving deleted nodes", indexes.GetNumberOfElements(), leftIndex.ComputeTotalSize(indexes));
	for(uint32 index : indexes)
	{
		const Path &path = leftIndex.GetNodePath(index);
		if(rightIndex.HasNodeIndex(path))
		{
			process.IncFileCount();
			process.ReduceTotalSize(rightIndex.GetNodeAttributes(index).Size());
			continue;
		}

		nodeDifferences.Insert(index);

		process.IncFileCount();
		process.AddFinishedSize(leftIndex.GetNodeAttributes(index).Size());
	}
	threadPool.WaitForAllTasksToComplete();
	process.Finished();

	return nodeDifferences;
}

BinaryTreeSet<uint32> NodeIndexDifferenceResolver::ComputeDifference(const FileSystemNodeIndex& leftIndex, const FileSystemNodeIndex& rightIndex) const
{
	BinaryTreeSet<uint32> diff;
	Mutex diffLock;

	InjectionContainer &injectionContainer = InjectionContainer::Instance();
	StaticThreadPool& threadPool = injectionContainer.Get<StaticThreadPool>();
	StatusTracker& statusTracker = injectionContainer.Get<StatusTracker>();

	ProcessStatus& process = statusTracker.AddProcessStatusTracker(u8"Computing index differences", leftIndex.GetNumberOfNodes(), leftIndex.ComputeTotalSize());
	for(uint32 i = 0; i < leftIndex.GetNumberOfNodes(); i++)
	{
		threadPool.EnqueueTask([&leftIndex, &rightIndex, &diff, &diffLock, &process, i]()
		{
			const Path& filePath = leftIndex.GetNodePath(i);
			const FileSystemNodeAttributes& fileAttributes = leftIndex.GetNodeAttributes(i);

			bool include = false;
			if(rightIndex.HasNodeIndex(filePath))
			{
				//both indexes have this node
				uint32 otherIndex = rightIndex.GetNodeIndex(filePath);
				const FileSystemNodeAttributes& otherFileAttributes = rightIndex.GetNodeAttributes(otherIndex);
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
		});
	}
	threadPool.WaitForAllTasksToComplete();
	process.Finished();

	return diff;
}

void NodeIndexDifferenceResolver::ComputeNodeDifferences(NodeIndexDifferences& nodeIndexDifferences, const BackupNodeIndex& leftIndex, const FileSystemNodeIndex& rightIndex, const BinaryTreeSet<uint32>& rightToLeftDiffs) const
{
	InjectionContainer &injectionContainer = InjectionContainer::Instance();
	StatusTracker& statusTracker = injectionContainer.Get<StatusTracker>();
	StaticThreadPool& threadPool = injectionContainer.Get<StaticThreadPool>();

	Mutex nodeDifferencesLock;

	//index new
	ProcessStatus& process = statusTracker.AddProcessStatusTracker(u8"Indexing potentially new nodes", rightToLeftDiffs.GetNumberOfElements(), rightIndex.ComputeTotalSize(rightToLeftDiffs));
	for(uint32 index : rightToLeftDiffs)
	{
		threadPool.EnqueueTask([this, index, &leftIndex, &rightIndex, &nodeIndexDifferences, &nodeDifferencesLock, &process]()
		{
			const FileSystemNodeAttributes &attributes = rightIndex.GetNodeAttributes(index);
			switch(attributes.Type())
			{
				case NodeType::Directory:
					nodeDifferencesLock.Lock();
					nodeIndexDifferences.differentData.Insert(index); //new node
					nodeDifferencesLock.Unlock();
					break;
				case NodeType::File:
				case NodeType::Link:
				{
					String hash = this->RetrieveNodeHash(index, rightIndex);
					uint32 leftNodeIndexByHash = leftIndex.FindNodeIndexByHash(hash);

					nodeDifferencesLock.Lock();
					if(leftNodeIndexByHash == Unsigned<uint32>::Max()) //could not find hash value
						nodeIndexDifferences.differentData.Insert(index); //new node
					else
					{
						const Path &filePath = rightIndex.GetNodePath(index);
						if(leftIndex.GetNodePath(leftNodeIndexByHash) == filePath) //same node and same hash, only update metadata
							nodeIndexDifferences.differentMetadata.Insert(index);
						else //same hash but different node -> moved node
							nodeIndexDifferences.moved.Insert(index, leftNodeIndexByHash);
					}
					nodeDifferencesLock.Unlock();
				}
					break;
			}

			process.IncFileCount();
			process.AddFinishedSize(attributes.Size());
		});
	}
	threadPool.WaitForAllTasksToComplete();
	process.Finished();
}

NodeIndexDifferences NodeIndexDifferenceResolver::ResolveDifferences(const BackupNodeIndex &leftIndex, const FileSystemNodeIndex &rightIndex, const BinaryTreeSet<uint32> &leftToRightDiffs, const BinaryTreeSet<uint32> &rightToLeftDiffs) const
{
	InjectionContainer &injectionContainer = InjectionContainer::Instance();
	StatusTracker& statusTracker = injectionContainer.Get<StatusTracker>();
	StaticThreadPool& threadPool = injectionContainer.Get<StaticThreadPool>();

	const Config &config = injectionContainer.Get<ConfigManager>().Config();

	NodeIndexDifferences nodeDifferences;

	nodeDifferences.deleted = this->ComputeDeleted(leftIndex, rightIndex, leftToRightDiffs);
	this->ComputeNodeDifferences(nodeDifferences, leftIndex, rightIndex, rightToLeftDiffs);

	//everything that was moved was not deleted
	for(const auto& kv : nodeDifferences.moved)
		nodeDifferences.deleted.Remove(kv.value);

	return nodeDifferences;
}

String NodeIndexDifferenceResolver::RetrieveNodeHash(uint32 nodeIndex, const FileSystemNodeIndex& index) const
{
	const BackupNodeIndex* backupNodeIndex = dynamic_cast<const BackupNodeIndex *>(&index);
	if(backupNodeIndex)
	{
		InjectionContainer &injectionContainer = InjectionContainer::Instance();
		const Config &config = injectionContainer.Get<ConfigManager>().Config();

		const BackupNodeAttributes& nodeAttributes = backupNodeIndex->GetNodeAttributes(nodeIndex);
		return nodeAttributes.Hash(config.hashAlgorithm);
	}

	const OSFileSystemNodeIndex* osFileSystemNodeIndex = dynamic_cast<const OSFileSystemNodeIndex *>(&index);
	if(osFileSystemNodeIndex)
		return osFileSystemNodeIndex->ComputeNodeHash(nodeIndex);

	NOT_IMPLEMENTED_ERROR; //implement me
	RAISE(ErrorHandling::IllegalCodePathError);
}