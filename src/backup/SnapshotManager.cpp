/*
 * Copyright (c) 2019-2021 Amir Czwink (amir130@hotmail.de)
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
#include "SnapshotManager.hpp"
//Local
#include "../status/ProcessStatus.hpp"
#include "../config/CompressionStatistics.hpp"
#include "../NodeIndexDifferenceResolver.hpp"

//Constructor
SnapshotManager::SnapshotManager()
{
	this->ReadInSnapshots();
}

//Public methods
bool SnapshotManager::AddSnapshot(const OSFileSystemNodeIndex& sourceIndex)
{
	InjectionContainer& ic = InjectionContainer::Instance();

	//we simply include all nodes whether they have changed or not and skip diff.deleted
	const NodeIndexDifferences diff = this->ComputeDifference(sourceIndex, true);

	UnprotectFile(ic.Config().dataPath);
	UniquePointer<Snapshot> snapshot = new Snapshot();

	const uint64 totalSize = sourceIndex.ComputeTotalSize(diff.differentData);
	ProcessStatus& process = ic.StatusTracker().AddProcessStatusTracker(u8"Creating snapshot: " + snapshot->Name(), sourceIndex.GetNumberOfNodes(), totalSize);

	StaticThreadPool& threadPool = InjectionContainer::Instance().TaskQueue();
	for(uint32 index : diff.differentData)
	{
		threadPool.EnqueueTask([&snapshot, index, &sourceIndex, &process]()
		{
			snapshot->BackupNode(index, sourceIndex, process);
			process.IncFinishedCount();
		});
	}

	for(uint32 index : diff.differentMetadata)
	{
		const BackupNodeIndex& lastIndex = *this->LastIndex();
		const BackupNodeAttributes &oldAttributes = lastIndex.GetNodeAttributes(lastIndex.GetNodeIndex(sourceIndex.GetNodePath(index)));
		snapshot->BackupNodeMetadata(index, oldAttributes, sourceIndex);
		process.IncFinishedCount();
	}

	for(const auto& kv : diff.moved)
	{
		const BackupNodeIndex& lastIndex = *this->LastIndex();
		const BackupNodeAttributes &oldAttributes = lastIndex.GetNodeAttributes(kv.value);
		snapshot->BackupMove(kv.key, sourceIndex, oldAttributes, lastIndex.GetNodePath(kv.value));
		process.IncFinishedCount();
	}

	threadPool.WaitForAllTasksToComplete();
	process.Finished();

	WriteProtectFile(ic.Config().dataPath);

	UnprotectFile(ic.Config().indexPath);
	snapshot->Serialize();
	snapshot->WriteProtect();
	WriteProtectFile(ic.Config().indexPath);

	const Config& config = ic.Config();
	ic.CompressionStats().Write(config.backupPath);

	//close snapshot and read it in again
	this->snapshots.Release();
	snapshot = nullptr;

	this->ReadInSnapshots();

	this->EnsureNoDifferenceExists(sourceIndex);

	DynamicArray<uint32> results = this->VerifySnapshot(*this->snapshots.Last(), true); //verify full snapshot to make sure older required snapshots not have gone corrupt
	
	return results.IsEmpty();
}

DynamicArray<uint32> SnapshotManager::VerifySnapshot(const Snapshot &snapshot, bool full) const
{
	InjectionContainer& ic = InjectionContainer::Instance();

	const FileSystemNodeIndex& index = snapshot.Index();

	ProcessStatus& process = ic.StatusTracker().AddProcessStatusTracker(u8"Verifying snapshot: " + snapshot.Name(),
			index.GetNumberOfNodes(), full ? index.ComputeTotalSize() : snapshot.ComputeSize());
	StaticThreadPool& threadPool = InjectionContainer::Instance().TaskQueue();

	DynamicArray<uint32> failedNodes;
	Mutex failedFilesLock;
	for(uint32 i = 0; i < index.GetNumberOfNodes(); i++)
	{
		threadPool.EnqueueTask([&snapshot, i, full, &index, &process, &failedNodes, &failedFilesLock]()
		{
			const BackupNodeAttributes &nodeAttributes = snapshot.Index().GetNodeAttributes(i);
			if(nodeAttributes.Type() == FileType::Directory)
				return;
			Path realNodePath;
			const Snapshot* dataSnapshot = snapshot.FindDataSnapshot(i, realNodePath);
			if(full || (dataSnapshot == &snapshot))
			{
				if (!dataSnapshot->VerifyNode(realNodePath))
				{
					failedFilesLock.Lock();
					failedNodes.Push(i);
					failedFilesLock.Unlock();
				}
				process.AddFinishedSize(nodeAttributes.Size());
				process.IncFinishedCount();
			}
		});
	}

	threadPool.WaitForAllTasksToComplete();
	process.Finished();

	return failedNodes;
}

//Private methods
NodeIndexDifferences SnapshotManager::ComputeDifference(const OSFileSystemNodeIndex& sourceIndex, bool updateDefault) const
{
	if(this->LastIndex())
	{
		NodeIndexDifferenceResolver resolver;
		NodeIndexDifferences difference = resolver.ComputeDiff(*this->LastIndex(), sourceIndex);

		if(updateDefault)
		{
			//assume all haven't changed and update only metadata for these
			for(uint32 i = 0; i < sourceIndex.GetNumberOfNodes(); i++)
			{
				if(!( difference.differentData.Contains(i) || difference.differentMetadata.Contains(i) || difference.moved.Contains(i) ))
					difference.differentMetadata.Insert(i);
			}
		}
		return difference;
	}

	//all nodes of sourceIndex need to be backed up
	NodeIndexDifferences difference{};
	for(uint32 i = 0; i < sourceIndex.GetNumberOfNodes(); i++)
		difference.differentData.Insert(i);
	return difference;
}

DynamicArray<String> SnapshotManager::ListSnapshotMetadataFiles()
{
	const Path& indexPath = InjectionContainer::Instance().Config().indexPath;
	File dir(indexPath);

	DynamicArray<String> snapshotFiles;
	for(const auto& entry : dir)
	{
		if(entry.type == FileType::File)
			snapshotFiles.Push(entry.name);
	}
	snapshotFiles.Sort();

	return snapshotFiles;
}

void SnapshotManager::ReadInSnapshots()
{
	const Path& indexPath = InjectionContainer::Instance().Config().indexPath;

	auto files = this->ListSnapshotMetadataFiles();
	for(const auto& entry : files)
	{
		//try to deserialize
		UniquePointer<Snapshot> snapshot = Snapshot::Deserialize(indexPath / entry);
		if(!snapshot.IsNull())
		{
			this->snapshots.Push(Move(snapshot));
			if(this->snapshots.GetNumberOfElements() > 1)
				this->snapshots.Last()->Previous(this->snapshots[this->snapshots.GetNumberOfElements()-2].operator->());
		}
	}
}

void SnapshotManager::EnsureNoDifferenceExists(const OSFileSystemNodeIndex &sourceIndex) const
{
	const NodeIndexDifferences diffNodeIndicesNew = this->ComputeDifference(sourceIndex, false);
	if(diffNodeIndicesNew.Exist())
		throw ErrorHandling::VerificationFailedException();
}
