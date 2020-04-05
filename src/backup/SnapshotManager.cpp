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

	const BinaryTreeSet<uint32> diffNodeIndices = this->ComputeDifference(sourceIndex);

	UnprotectFile(ic.Get<ConfigManager>().Config().dataPath);
	UniquePointer<Snapshot> snapshot = new Snapshot();

	const uint64 totalSize = sourceIndex.ComputeTotalSize(diffNodeIndices);
	ProcessStatus& process = ic.Get<StatusTracker>().AddProcessStatusTracker(u8"Creating snapshot: " + snapshot->Name(), sourceIndex.GetNumberOfNodes(), totalSize);

	StaticThreadPool& threadPool = InjectionContainer::Instance().Get<StaticThreadPool>();
	for(uint32 i = 0; i < sourceIndex.GetNumberOfNodes(); i++)
	{
		if(diffNodeIndices.Contains(i))
		{
			//TODO: reenable this
			//threadPool.EnqueueTask([&snapshot, i, &index, &process]()
			//{
				snapshot->AddNode(i, sourceIndex, process);
				process.IncFinishedCount();
			//});
		}
		else
		{
			NOT_IMPLEMENTED_ERROR; //TODO: implement me
		}
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

	const BinaryTreeSet<uint32> diffNodeIndicesNew = this->ComputeDifference(sourceIndex);
	if(!diffNodeIndices.IsEmpty())
		NOT_IMPLEMENTED_ERROR; //TODO: implement me, after the backup there should be no differences to source index

	this->VerifySnapshot(*this->snapshots.Last());
}

//Private methods
inline BinaryTreeSet<uint32> SnapshotManager::ComputeDifference(const FileSystemNodeIndex& index)
{
	if(this->LastIndex())
		NOT_IMPLEMENTED_ERROR; //TODO: implement me

	BinaryTreeSet<uint32> result;
	for(uint32 i = 0; i < index.GetNumberOfNodes(); i++)
		result.Insert(i);
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

void SnapshotManager::VerifySnapshot(const Snapshot &snapshot) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}