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
/*AutoPointer<const Node> VirtualSnapshotFilesystem::GetNode(const Path &path) const
{
	if(!this->snapshot.Index().HasNodeIndex(path))
		return nullptr;
	uint32 nodeIndex = this->snapshot.Index().GetNodeIndex(path);

	const BackupNodeAttributes& attributes = this->snapshot.Index().GetNodeAttributes(nodeIndex);
	const Snapshot* dataSnapshot;
	Path realNodePath = path;
	if(attributes.Type() == NodeType::Directory)
		dataSnapshot = &this->snapshot;
	else
		dataSnapshot = this->snapshot.FindDataSnapshot(nodeIndex, realNodePath);

	return dataSnapshot->Filesystem().GetNode(realNodePath);
}*/

SpaceInfo VirtualSnapshotFilesystem::QuerySpace() const
{
	NOT_IMPLEMENTED_ERROR; //implement me
	return SpaceInfo();
}


//TODO: NOT IMPLEMENTED
UniquePointer<DirectoryEnumerator> VirtualSnapshotFilesystem::EnumerateChildren(const Path &path) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return UniquePointer<DirectoryEnumerator>();
}

UniquePointer<InputStream> VirtualSnapshotFilesystem::OpenFileForReading(const Path &path, bool verify) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return UniquePointer<InputStream>();
}

Optional<FileInfo> VirtualSnapshotFilesystem::QueryFileInfo(const Path &path) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return Optional<FileInfo>();
}

Optional<Path> VirtualSnapshotFilesystem::ReadLinkTarget(const Path &path) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return Optional<Path>();
}
//TODO: NOT IMPLEMENTED