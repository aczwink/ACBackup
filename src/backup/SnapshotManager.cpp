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
#include "SnapshotManager.hpp"
//Local
#include "../status/ProcessStatus.hpp"
#include "../config/CompressionStatistics.hpp"

//Constructor
SnapshotManager::SnapshotManager()
{
	this->ReadInSnapshots();
}

//Public methods
void SnapshotManager::AddSnapshot(const OSFileSystemNodeIndex& sourceIndex)
{
	InjectionContainer& ic = InjectionContainer::Instance();

	const Map<uint32, NodeDifference> diffNodeIndices = this->ComputeDifference(sourceIndex, true);

	UnprotectFile(ic.Get<ConfigManager>().Config().dataPath);
	UniquePointer<Snapshot> snapshot = new Snapshot();

	const uint64 totalSize = sourceIndex.ComputeTotalSize(diffNodeIndices);
	ProcessStatus& process = ic.Get<StatusTracker>().AddProcessStatusTracker(u8"Creating snapshot: " + snapshot->Name(), sourceIndex.GetNumberOfNodes(), totalSize);

	StaticThreadPool& threadPool = InjectionContainer::Instance().Get<StaticThreadPool>();
	for(const auto& kv : diffNodeIndices)
	{
		//TODO: reenable this
		//threadPool.EnqueueTask([&snapshot, i, &index, &process]()
		//{
		switch(kv.value.type)
		{
			case NodeDifferenceType::BackupFully:
				snapshot->BackupNode(kv.key, sourceIndex, process);
				break;
			case NodeDifferenceType::BackreferenceWithDifferentPath:
				NOT_IMPLEMENTED_ERROR; //TODO: implement me
				break;
			case NodeDifferenceType::Delete:
				NOT_IMPLEMENTED_ERROR; //TODO: implement me
				break;
			case NodeDifferenceType::UpdateMetadataOnly:
			{
				const BackupNodeIndex& lastIndex = *this->LastIndex();
				const BackupNodeAttributes &oldAttributes = lastIndex.GetNodeAttributes(lastIndex.GetNodeIndex(sourceIndex.GetNodePath(kv.key)));
				snapshot->BackupNodeMetadata(kv.key, oldAttributes, sourceIndex, process);
			}
			break;
		}
		process.IncFinishedCount();
		//});
	}
	threadPool.WaitForAllTasksToComplete();
	process.Finished();

	WriteProtectFile(ic.Get<ConfigManager>().Config().dataPath);

	UnprotectFile(ic.Get<ConfigManager>().Config().indexPath);
	snapshot->Serialize();
	snapshot->WriteProtect();
	WriteProtectFile(ic.Get<ConfigManager>().Config().indexPath);

	const Config& config = ic.Get<ConfigManager>().Config();
	ic.Get<CompressionStatistics>().Write(config.backupPath);

	//close snapshot and read it in again
	this->snapshots.Release();
	snapshot = nullptr;

	this->ReadInSnapshots();

	this->EnsureNoDifferenceExists(sourceIndex);

	this->VerifySnapshot(*this->snapshots.Last(), true); //verify full snapshot to make sure older required snapshots not have gone corrupt
}

DynamicArray<uint32> SnapshotManager::VerifySnapshot(const Snapshot &snapshot, bool full) const
{
	InjectionContainer& ic = InjectionContainer::Instance();

	const FileSystemNodeIndex& index = snapshot.Index();

	ProcessStatus& process = ic.Get<StatusTracker>().AddProcessStatusTracker(u8"Verifying snapshot: " + snapshot.Name(),
			index.GetNumberOfNodes(), full ? index.ComputeTotalSize() : snapshot.ComputeSize());
	StaticThreadPool& threadPool = InjectionContainer::Instance().Get<StaticThreadPool>();

	DynamicArray<uint32> failedNodes;
	Mutex failedFilesLock;
	for(uint32 i = 0; i < index.GetNumberOfNodes(); i++)
	{
		//TODO: reenable this
		//threadPool.EnqueueTask([&snapshot, i, &index, &process]()
		//{
		const BackupNodeAttributes &nodeAttributes = snapshot.Index().GetNodeAttributes(i);
		if(nodeAttributes.Type() == FileSystemNodeType::Directory)
			continue;
		const Snapshot* dataSnapshot = snapshot.FindDataSnapshot(i);
		if(full || (dataSnapshot == &snapshot))
		{
			const Path& nodePath = snapshot.Index().GetNodePath(i);
			if (!dataSnapshot->VerifyNode(nodePath))
			{
				failedFilesLock.Lock();
				failedNodes.Push(i);
				failedFilesLock.Unlock();
			}
			process.AddFinishedSize(nodeAttributes.Size());
			process.IncFinishedCount();
		}
		//});
	}

	threadPool.WaitForAllTasksToComplete();
	process.Finished();

	return failedNodes;
}

//Private methods
inline Map<uint32, NodeDifference> SnapshotManager::ComputeDifference(const OSFileSystemNodeIndex& sourceIndex, bool updateDefault) const
{
	if(this->LastIndex())
	{
		const BackupNodeIndex& newestSnapshotIndex = *this->LastIndex();

		BinaryTreeSet<uint32> leftToRightDiffs = newestSnapshotIndex.ComputeDifference(sourceIndex);
		BinaryTreeSet<uint32> rightToLeftDiffs = sourceIndex.ComputeDifference(newestSnapshotIndex);
		return this->ResolveDifferences(newestSnapshotIndex, sourceIndex, leftToRightDiffs, rightToLeftDiffs, updateDefault);
	}

	//all nodes of sourceIndex need to be backed up
	Map<uint32, NodeDifference> result;
	for(uint32 i = 0; i < sourceIndex.GetNumberOfNodes(); i++)
		result.Insert(i, {NodeDifferenceType::BackupFully});
	return result;
}

DynamicArray<Path> SnapshotManager::ListPathsInIndexDirectory()
{
	const Path& indexPath = InjectionContainer::Instance().Get<ConfigManager>().Config().indexPath;
	auto dir = OSFileSystem::GetInstance().GetDirectory(indexPath);
	auto dirWalker = dir->WalkFiles();

	DynamicArray<Path> snapshotFiles;
	for(const Path& relPath : dirWalker)
	{
		snapshotFiles.Push(relPath);
	}
	snapshotFiles.Sort();

	return snapshotFiles;
}

void SnapshotManager::ReadInSnapshots()
{
	const Path& indexPath = InjectionContainer::Instance().Get<ConfigManager>().Config().indexPath;

	DynamicArray<Path> xmlFiles = this->ListPathsInIndexDirectory();
	for(const Path& path : xmlFiles)
	{
		//try to deserialize
		UniquePointer<Snapshot> snapshot = Snapshot::Deserialize(indexPath / path);
		if(!snapshot.IsNull())
		{
			this->snapshots.Push(Move(snapshot));
			if(this->snapshots.GetNumberOfElements() > 1)
				this->snapshots.Last()->Previous(this->snapshots[this->snapshots.GetNumberOfElements()-2].operator->());
		}
	}
}

Map<uint32, NodeDifference> SnapshotManager::ResolveDifferences(const BackupNodeIndex &leftIndex, const OSFileSystemNodeIndex &rightIndex, const BinaryTreeSet<uint32> &leftToRightDiffs, const BinaryTreeSet<uint32> &rightToLeftDiffs, bool updateDefault) const
{
	InjectionContainer &injectionContainer = InjectionContainer::Instance();
	StatusTracker& statusTracker = injectionContainer.Get<StatusTracker>();
	StaticThreadPool& threadPool = injectionContainer.Get<StaticThreadPool>();

	const Config &config = injectionContainer.Get<ConfigManager>().Config();

	Map<uint32, NodeDifference> nodeDifferences;
	Mutex nodeDifferencesLock;

	if(updateDefault)
	{
		//assume all haven't changed and update only metadata for these
		for(uint32 i = 0; i < rightIndex.GetNumberOfNodes(); i++)
			nodeDifferences.Insert(i, {NodeDifferenceType::UpdateMetadataOnly});
	}

	//find deleted
	ProcessStatus& process = statusTracker.AddProcessStatusTracker(u8"Resolving deleted nodes", leftToRightDiffs.GetNumberOfElements(), leftIndex.ComputeTotalSize(leftToRightDiffs));
	for(uint32 index : leftToRightDiffs)
	{
		const Path &path = leftIndex.GetNodePath(index);
		if(rightIndex.HasNodeIndex(path))
		{
			process.IncFileCount();
			process.ReduceTotalSize(rightIndex.GetNodeAttributes(index).Size());
			continue;
		}

		nodeDifferences.Insert(rightIndex.GetNodeIndex(path), {NodeDifferenceType::Delete});

		process.IncFileCount();
		process.AddFinishedSize(leftIndex.GetNodeAttributes(index).Size());
	}
	threadPool.WaitForAllTasksToComplete();
	process.Finished();

	//index new
	ProcessStatus& process2 = statusTracker.AddProcessStatusTracker(u8"Indexing potentially new nodes", rightToLeftDiffs.GetNumberOfElements(), rightIndex.ComputeTotalSize(rightToLeftDiffs));
	for(uint32 index : rightToLeftDiffs)
	{
		//threadPool.EnqueueTask([]() //TODO: implement me
		//{
		const FileSystemNodeAttributes &attributes = rightIndex.GetNodeAttributes(index);
		switch(attributes.Type())
		{
			case FileSystemNodeType::Directory:
				nodeDifferencesLock.Lock();
				nodeDifferences.Insert(index, {NodeDifferenceType::UpdateMetadataOnly});
				nodeDifferencesLock.Unlock();
				break;
			case FileSystemNodeType::File:
			case FileSystemNodeType::Link:
			{
				String hash = rightIndex.ComputeNodeHash(index);
				uint32 leftNodeIndexByHash = leftIndex.FindNodeIndexByHash(hash);

				nodeDifferencesLock.Lock();
				if(leftNodeIndexByHash == Unsigned<uint32>::Max()) //could not find hash value
					nodeDifferences.Insert(index, {NodeDifferenceType::BackupFully});
				else
				{
					const Path &filePath = rightIndex.GetNodePath(index);
					if(leftIndex.GetNodePath(leftNodeIndexByHash) == filePath) //same node and same hash, only update metadata
						nodeDifferences.Insert(index, {NodeDifferenceType::UpdateMetadataOnly});
					else //same hash but different node -> moved node
						nodeDifferences.Insert(index, {NodeDifferenceType::BackreferenceWithDifferentPath, leftNodeIndexByHash});
				}
				nodeDifferencesLock.Unlock();
			}
			break;
		}

		process2.IncFileCount();
		process2.AddFinishedSize(attributes.Size());
		//});
	}
	threadPool.WaitForAllTasksToComplete();
	process2.Finished();

	return nodeDifferences;
}

void SnapshotManager::EnsureNoDifferenceExists(const OSFileSystemNodeIndex &sourceIndex) const
{
	const Map<uint32, NodeDifference> diffNodeIndicesNew = this->ComputeDifference(sourceIndex, false);
	if(!diffNodeIndicesNew.IsEmpty())
		NOT_IMPLEMENTED_ERROR; //TODO: implement me, after the backup there should be no differences to source index
}