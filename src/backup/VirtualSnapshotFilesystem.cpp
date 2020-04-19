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
#include "VirtualSnapshotFilesystem.hpp"

//Public methods
UniquePointer<OutputStream> VirtualSnapshotFilesystem::CreateFile(const Path &filePath)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return UniquePointer<OutputStream>();
}

bool VirtualSnapshotFilesystem::Exists(const Path &path) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return false;
}

void VirtualSnapshotFilesystem::Flush()
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}

AutoPointer<FileSystemNode> VirtualSnapshotFilesystem::GetNode(const Path &path)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return AutoPointer<FileSystemNode>();
}

AutoPointer<const FileSystemNode> VirtualSnapshotFilesystem::GetNode(const Path &path) const
{
	if(!this->snapshot.Index().HasNodeIndex(path))
		return nullptr;
	uint32 nodeIndex = this->snapshot.Index().GetNodeIndex(path);

	const BackupNodeAttributes& attributes = this->snapshot.Index().GetNodeAttributes(nodeIndex);
	const Snapshot* dataSnapshot;
	if(attributes.Type() == FileSystemNodeType::Directory)
		dataSnapshot = &this->snapshot;
	else
		dataSnapshot = this->snapshot.FindDataSnapshot(nodeIndex);

	return dataSnapshot->Filesystem().GetNode(path);
}

AutoPointer<Directory> VirtualSnapshotFilesystem::GetRoot()
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return AutoPointer<Directory>();
}

AutoPointer<const Directory> VirtualSnapshotFilesystem::GetRoot() const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return AutoPointer<const Directory>();
}

uint64 VirtualSnapshotFilesystem::GetSize() const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return 0;
}

void VirtualSnapshotFilesystem::Move(const Path &from, const Path &to)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}