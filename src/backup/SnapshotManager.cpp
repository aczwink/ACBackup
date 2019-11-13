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
#include "SnapshotManager.hpp"

//Constructor
SnapshotManager::SnapshotManager(const Path &path, const Config &config, StaticThreadPool& threadPool, StatusTracker &statusTracker) : backupPath(path),
	threadPool(threadPool), statusTracker(statusTracker)
{
	this->ReadInSnapshots();
}

//Public methods
void SnapshotManager::AddSnapshot(const FileSystemNodeIndex &index)
{
	const BinaryTreeSet<uint32> diffNodeIndices = this->ComputeDifference(index);
	UniquePointer<Snapshot> snapshot = new Snapshot(this->backupPath);

	const uint64 totalSize = index.ComputeTotalSize(diffNodeIndices);
	ProcessStatus& process = this->statusTracker.AddProcessStatusTracker(u8"Creating snapshot: " + snapshot->Name(), index.GetNumberOfNodes(), totalSize);

	for(uint32 i = 0; i < index.GetNumberOfNodes(); i++)
	{
		if(diffNodeIndices.Contains(i))
		{
			this->threadPool.EnqueueTask([&snapshot, i, &index, &process]()
			{
				snapshot->AddNode(i, index);
				process.IncFinishedCount();
			});
		}
		else
		{
			NOT_IMPLEMENTED_ERROR; //TODO: this should be a previous file but validate this!
			process.IncFinishedCount();
		}
	}
	this->threadPool.WaitForAllTasksToComplete();
	process.Finished();

	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}

//Private methods
void SnapshotManager::ReadInSnapshots()
{
	//read in snapshots
	DynamicArray<Path> snapshotFiles;

	auto dir = OSFileSystem::GetInstance().GetDirectory(this->backupPath);
	auto dirWalker = dir->WalkFiles();

	for(const Path& relPath : dirWalker)
	{
		snapshotFiles.Push(relPath);
	}
	snapshotFiles.Sort();

	for(const Path& ssfile : snapshotFiles)
	{
		//try to deserialize
		UniquePointer<Snapshot> snapshot = Snapshot::Deserialize(this->backupPath / ssfile);
		if(!snapshot.IsNull())
		{
			this->snapshots.Push(Move(snapshot));
			if(this->snapshots.GetNumberOfElements() > 1)
				this->snapshots.Last()->Previous(this->snapshots[this->snapshots.GetNumberOfElements()-2].operator->());
		}
	}
}
