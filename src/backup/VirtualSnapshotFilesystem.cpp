/*
 * Copyright (c) 2020-2021 Amir Czwink (amir130@hotmail.de)
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
#include "VirtualSnapshotFilesystem.hpp"

//Public methods
UniquePointer<DirectoryEnumerator> VirtualSnapshotFilesystem::EnumerateChildren(const Path &path) const
{
	if(!this->snapshot.Index().HasNodeIndex(path))
		return nullptr;

	uint32 nodeIndex = this->snapshot.Index().GetNodeIndex(path);
	DynamicArray<uint32> children;

	auto& index = this->snapshot.Index();
	for(uint32 i = 0; i < index.GetNumberOfNodes(); i++)
	{
		if(i == nodeIndex)
			continue;

		const auto& nodePath = index.GetNodePath(i);
		if(nodePath.GetParent() == path)
			children.Push(i);
	}

	class VirtualDirectoryEnumerator : public DirectoryEnumerator
	{
	public:
		//Constructor
		inline VirtualDirectoryEnumerator(DynamicArray<uint32>&& children, const BackupNodeIndex& index) : children(Move(children)), index(index)
		{
			this->currentIndex = -1;
		}

		//Methods
		const DirectoryEntry& GetCurrent() const
		{
			return this->directoryEntry;
		}

		bool MoveForward() override
		{
			if((this->currentIndex + 1) < this->children.GetNumberOfElements())
			{
				this->currentIndex++;
				uint32 nodeIndex = this->children[this->currentIndex];
				this->directoryEntry.type = this->index.GetNodeAttributes(nodeIndex).Type();
				this->directoryEntry.name = this->index.GetNodePath(nodeIndex).GetName();
				return true;
			}
			return false;
		}

	private:
		//Members
		DynamicArray<uint32> children;
		const BackupNodeIndex& index;
		int32 currentIndex;
		DirectoryEntry directoryEntry;
	};

	return new VirtualDirectoryEnumerator(Move(children), index);
}

UniquePointer<InputStream> VirtualSnapshotFilesystem::OpenFileForReading(const Path &path, bool verify) const
{
	if(!this->snapshot.Index().HasNodeIndex(path))
		return nullptr;

	uint32 nodeIndex = this->snapshot.Index().GetNodeIndex(path);
	Path snapshotPath;
	const Snapshot* dataSnapshot = this->snapshot.FindDataSnapshot(nodeIndex, snapshotPath);

	return dataSnapshot->Filesystem().OpenFileForReading(snapshotPath, true);
}

Optional<FileInfo> VirtualSnapshotFilesystem::QueryFileInfo(const Path &path) const
{
	if(!this->snapshot.Index().HasNodeIndex(path))
		return {};

	uint32 nodeIndex = this->snapshot.Index().GetNodeIndex(path);

	const BackupNodeAttributes& attributes = this->snapshot.Index().GetNodeAttributes(nodeIndex);

	FileInfo fileInfo;
	fileInfo.type = attributes.Type();
	fileInfo.size = attributes.Size();
	fileInfo.lastModifiedTime = attributes.LastModifiedTime();
	fileInfo.permissions = attributes.Permissions().Clone();
	fileInfo.storedSize = attributes.ComputeSumOfBlockSizes();

	return fileInfo;
}


//TODO: NOT IMPLEMENTED
SpaceInfo VirtualSnapshotFilesystem::QuerySpace() const
{
	NOT_IMPLEMENTED_ERROR; //implement me
	return SpaceInfo();
}

Optional<Path> VirtualSnapshotFilesystem::ReadLinkTarget(const Path &path) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return Optional<Path>();
}
//TODO: NOT IMPLEMENTED